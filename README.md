# honk
This is a simple botnet in C that uses TCP and supports only a single C&C.  I wrote it a couple of months ago to learn about network programming.  The scanner portion does not actually contain any scanning code and is unfinished.  By default, the bot binary is compiled with gcc and diet libc and the server using the system's default (both of these can be changed and this project does not depend on any compiler/libc specific code).  Both support being compiled with nearly all of GCC/Clangs optimizations and security features (I use this as one way to make sure I'm not relying on UB).  I also used some code and ideas from Mirai and Bashlite.

# Features
* Written in C99
* Adding random data to packets (beginning and end): see protocol.c:protocol_format_send_msg()
* Mostly secure random number generation using an rand with a seed of clock()*getppid()
* Only a couple of minor compiler warnings (with nearly all of them turned on)
* Server uses select() and nonblocking sockets
* Messages have a code system, 'p' and 'P' for ping and pong, 's' to execute command, easy to implement your own
* Control C&C over Telnet server (by default on port 5555)
* Client keeps trying to reconnect (with increasingly large delay)
* Client closes kernel's watchdog and STDOUT/STDIN/STDERR if not in debug mode