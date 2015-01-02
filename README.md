<h1>Getting Started</h1>

Goal: Implement a file system that manages processes in your system (similar to what you see in /proc) using FUSE APIs.

Filesystem in Userspace lets the user create their own file system by running the code in user space. The FUSE module provides only a “bridge” to the actual kernel interfaces.

<h1>Build and Run</h1>

To run a FUSE program, we'll need two terminals and an empty new directory. Will run filesystem under a debugger in terminal #1; terminal #2 will be used for testing. The scratch directory is needed because 

we must have an empty directory on which to mount your shiny new filesystem. 

Command to compile program:

 gcc -Wall FS.c `pkg-config fuse --cflags --libs` -o FS

Command to mount/run filesystem:

 ./FS /tmp/myproc –d –f

     Optional parameters:

      -d switch means "debug"; in addition to printing helpful debugging output, it keeps the program in the foreground so gdb        won't lose track of it.

      -f enables the printf/fprintf debugging code to work.

When we run your program this way, it automatically goes into the background and starts serving up newly mounted filesystem. After we finish testing, the fusermount command unmounts the mounted filesystem and kills the background program.

<h1>Environment</h1>

1. Ubuntu 14.01 LTS – 64 Bit

2. Programming Language: C

3. FUSE APIs/Library
