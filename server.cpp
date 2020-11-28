#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <filesystem>
#include <iostream>
#include <cerrno>
#include <dirent.h>
#include <fstream>

using namespace std;

#define MAX_LINE 4096
#define LINSTENPORT 7788
#define SERVERPORT 8877
#define BUFFSIZE 4096

void writefile(int sockfd, FILE *fp);
ssize_t total=0;
int main(int argc, char *argv[])
{
    cout<<"If you are not running as admin, you will be prompt to enter your password "<<endl;
    system("sudo -s :"); // MUST

    while (true) {
        ifstream ifexfile("DUMPED_PROCESS.zip");
        if (ifexfile)
            system("find DUMPED_PROCESS.zip -delete");

        ifexfile.close();

        DIR* dir = opendir("DUMPED_PROCESS");
        if (dir) {
            closedir(dir);
            system("find DUMPED_PROCESS -delete");
        } else if (ENOENT == errno) {} else {
            cout<<"Directory Cleaning Failed!"<<endl;
        }


        total = 0;
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("Can't allocate sockfd");
            exit(1);
        }

        struct sockaddr_in clientaddr, serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons(SERVERPORT);

        if (bind(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1) {
            perror("Bind Error");
            exit(1);
        }

        if (listen(sockfd, LINSTENPORT) == -1) {
            perror("Listen Error");
            exit(1);
        }

        socklen_t addrlen = sizeof(clientaddr);
        int connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen);
        if (connfd == -1) {
            perror("Connect Error");
            exit(1);
        }
        close(sockfd);

        char filename[BUFFSIZE] = {0};
        if (recv(connfd, filename, BUFFSIZE, 0) == -1) {
            perror("Can't receive filename");
            exit(1);
        } else {
            printf("%s", filename);
        }

        FILE *fp = fopen(filename, "wb");
        if (fp == NULL) {
            perror("Can't open file");
            exit(1);
        }

        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN);
        writefile(connfd, fp);
//        printf("Receive Success, NumBytes = %ld\n", total);


        fclose(fp);
        close(connfd);

        // UNZIP
        string processFileZIP = filename;
        string command ="unzip "+processFileZIP+" > /dev/null 2>&1";
        system(command.c_str());

        // GET DIRECTORY
        ifstream MyFile("DUMPED_PROCESS/ClientProcessPath.txt");
        if(!MyFile){
            cout<<"[ERROR] ClientProcessPath.txt DOES NOT EXIST!"<<endl;
            exit(0);
        }
        std::string dirr((std::istreambuf_iterator<char>(MyFile)),
                        std::istreambuf_iterator<char>());
        MyFile.close();

        // RESTORE
        command="mkdir -p "+dirr+" && cp -r DUMPED_PROCESS/process "+dirr+"process > /dev/null 2>&1";
        system(command.c_str());
//        system("cp -r DUMPED_PROCESS/process /home/client/CLionProjects/myfork/process");
        command = "sudo -s criu restore -D DUMPED_PROCESS/ -j -v4 > /dev/null 2>&1";
        system(command.c_str());
    }
    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0)
    {
        total+=n;
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }

        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE);
    }
}