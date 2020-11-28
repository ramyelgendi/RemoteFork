# Remote Fork

Remote Fork (myfork(IP)) is a c++ program that performs the functionality of a normal fork but instead the child created runs on another machine that matches the IP passed.

## Installation

- Both nodes are to be on linux virtual machines.
- CRIU 3.15 to be installed, and both terminal commands “criu dump ….” & “criu restore ….” to be working.

- CRIU 3.15: [Source Code](http://download.openvz.org/criu/criu-3.15.tar.bz2)

- Installation: [Steps](https://criu.org/Installation )

- Zip & Unzip terminal commands to be installed using
```bash
sudo apt-get install unzip
sudo apt-get install zip
```
- There are 2 CPP files to be run: Client & Server and both must run from the terminal not on any IDE (Make sure to run the files with these exact names).

- Client:
Open folder of client.cpp in terminal and run the command: 
```bash
g++ client.cpp -o process
./process
```

- Server:
```bash
Open folder of server.cpp in terminal and run the command: 
g++ server.cpp
./a.out
```


## How Does It Work?

- At the beginning of both server.cpp and client.cpp you will be prompted to enter your administrator password to avoid any interruption while running the code.

- First, you will run the server.cpp on both virtual machines, and keep them running, and on the machine you want to use to use the myfork(ip) command, you will open the client.cpp, change the code to your own in main function and call the myfork command after getting the destination IP. 
- Then, after running the client.cpp file, the program will run and when it's time to remote fork, a local fork is called and the parent will call CRIU on the child. CRIU will step in and dump the child process, then ZIP all of the dumped files, along with the execution file (process) and a text file that has the path to the directory of this execution file. 
- This ZIPed file is then sent over to the destination virtual machine using TCP sockets to handle any data loss and make sure the bytes are sent in order. After the zip file is received, the server will unzip this file and save the execution file (process) in the same path as it was in the source virtual machine and then CRIU restore will run to continue this process on the new virtual machine.
- Also, on the client.cpp after the files are sent, the process will continue running there normally. You will end up with both processes running on two virtual machines simultaneously.

## Contributors
* Ramy ElGendi
