// Libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <dirent.h>
#include <cerrno>
#include <fstream>
#include <array>
#include <memory>
using namespace std;


// Pre-Defined Variables
#define MAX_LINE 4096
#define SERVERPORT 8877
#define BUFFSIZE 4096
#define FILENAME "DUMPED_PROCESS.zip"

// Global Variables
ssize_t total=0;

// Functions
void error(const char * error){
    perror(error);
    exit(1);
}
void sendfile(FILE *fp, int sockfd)
{
    int n;
    char sendline[MAX_LINE] = {0};
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0)
    {
        total+=n;
        if (n != MAX_LINE && ferror(fp)) error("Read File Error");
        if (send(sockfd, sendline, n, 0) == -1) error("Can't send file");
        memset(sendline, 0, MAX_LINE);
    }
}
std::string exec(const char* cmd) {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
int sendProcess(const string& IP)
{
    // Setting Up TCP Connection
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) error("Can't allocate socket_fd");
    struct sockaddr_in serveraddr{};
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVERPORT);
    if (inet_pton(AF_INET, IP.c_str(), &serveraddr.sin_addr) < 0) error("IPaddress Convert Error");
    if (connect(socket_fd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) error("Connect Error");
//    printf("You are connected to IP: %s & PORT: %i \n",SERVERIP,SERVERPORT);

    // Checking Filename & File Existance
    char *filename = basename((char*)FILENAME);
    if (filename == nullptr) error("Can't get filename");
    char buff[BUFFSIZE] = {0};
    strncpy(buff, filename, strlen(filename));
    if (send(socket_fd, buff, BUFFSIZE, 0) == -1) error("Can't send filename");
    FILE *fp = fopen(FILENAME, "rb");
    if (fp == nullptr) error("Can't open file");

    // Sending File
    sendfile(fp, socket_fd);
//    printf("Send Success, NumBytes = %ld\n", total);

    // Closing Connection
    fclose(fp);
    close(socket_fd);
    return 0;
}

int myfork(const string& IP) {
    string command;
    int pid = fork();
    if(pid<0){
        cout<<"Failed at restoring fork"<<endl;
        exit(0);
    } else if (pid>0){ // Parent
        // DUMP
        system("mkdir DUMPED_PROCESS > /dev/null 2>&1");
        command = "sudo -s criu dump -t "+to_string(pid)+" -D DUMPED_PROCESS -j -v4 > /dev/null 2>&1";
        system(command.c_str());


        // ADDING CURRENT PROGRAM RUNNABLE TO DUMPED
        system("cp process DUMPED_PROCESS");

        // ADDING CURRENT PATH TO FOLDER
        ofstream MyFile("DUMPED_PROCESS/ClientProcessPath.txt");
        string temp = exec("readlink -f process");
        temp = temp.erase(temp.length()-8);
//        cout<<temp<<endl;
        MyFile<< temp;
        MyFile.close();

        string x_FileName = FILENAME;
        // ZIP
        command ="zip -r "+x_FileName+" DUMPED_PROCESS > /dev/null 2>&1" ;
        system(command.c_str());

        // SEND FILE
        sendProcess(IP);
        command = "find "+x_FileName+" -delete";
        system(command.c_str());

        // DELETE ZIP & DUMP
        DIR* dir = opendir("DUMPED_PROCESS");
        if (dir) {
            closedir(dir);
            system("find DUMPED_PROCESS -delete");
        } else if (ENOENT == errno) {} else {
            cout<<"Directory Cleaning Failed!"<<endl;
        }
    } else if (pid==0){ // Child
    }
    return 0;
}

int main() {
    cout<<"If you are not running as admin, you will be prompt to enter your password "<<endl;
    system("sudo -s :"); // MUST

    int i=0;
    while(true){
        sleep(1);
        cout<<i++<<"   "<<endl;
        if(i==4)
            myfork("10.211.55.12");
    }
}