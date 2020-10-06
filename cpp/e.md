- there's a lot of inconsistency which will quickly make the whole project very difficult to work on.
  i strongly suggest fixing all this now, and writing some kind of style guide for yourself to adhere to.
  the main points are indentation (e.g. `Process.hpp` uses tabs yet `client.hpp` uses spaces), capitalization
  in naming of files and instances, functions, variables etc (e.g. `Process.hpp` vs `client.hpp`, `SOCKET ListenSocket`
  vs `WSADATA wsaData` vs `std::vector<SOCKET> all_connections`, `int send_commands()` vs `BOOL ReadFromSocket()`).

  some variables also are named adhering to a principle called Hungarian Notation in which the type is encoded in the var
  name (e.g. `bRunning` - "boolean Running"), whereas others aren't. microsoft uses this in its examples
  but it's generally a bad practise to follow in my opinion, especially in the way that microsoft makes use of it


- there's some duplication of code between client & server in `client.hpp` and `server.hpp`. since you're
  using VS projects i'm not really sure how to address this but try to put the `Commands` class declaration in its
  own file, stored at the root of the project, and reference the one file from both projects using a relative include
  path e.g. `#include "../../commands.hpp"`. this isn't ideal but without the pain of subprojects i don't know how to accomplish that


- you're doing what i used to do, which is writing c++ like it's c. there **are** some compelling reasons to
  use things such as `printf` even in c++ but you should never really be doing anything like storing string
  constants as character arrays. prefer std's string classes wherever possible, along with having your
  functions take std strings rather than char pointers. you can convert a cstring to a c++ string with `std::string(cstring)`
  and vica versa with `stdstring.c_str()`


- `Commands::print_help()` should take advantage of implicit string concatenation and not have commas after
  each literal, so you only need to call `printf` once:
  ```
    printf("\n=================================================================\n"
    "help           Shows this help menu\n"
                    "screenshot     Take a screenshot of the targets screen\n"
                    "download       Download chosen file from the target\n"
                    "upload         Uploads Local file to the target\n"
                    "webcam         Transfers a picture from the victims primary webcam\n"
                    "livecam        tTransfers a video from the victims primary webcam\n"
                    "getpass        Attempts to grab the targets saved passwords\n"
                    "quit           Quit to handler\n"
    "=================================================================\n");
  ```


- windows is fucked in regards to how it does utf-8 / unicode stuff. you should use wide character
  versions of api functions (those that end in W instead of A or nothing at all), declare
  string / character literals with an L prepended (e.g. `std::wstring s = L"hello world";`),
  and use wide char versions of standard library functions (e.g. `wstring`, `wcout`, etc).
  also replace your entry point with `auto wmain(int argc, wchar_t *argv[], wchar_t *envp[]) -> int`


- i'm not sure why in the main files you create the instance of a class then immediately make a pointer
  to it and use the arrow operator from thereon, but if there's a good reason that's definitely the kind
  of thing that should be commented.
  also for future reference you can just use `.` with pointers. you only need `->` when it's a reference


- l39 of `client.cpp` is some mad C shit and should be murdered at all costs


- you don't need parentheses around lone bitwise operations e.g. `(STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW)`


- `void`-returning functions don't need the `return;` keyword at the end

