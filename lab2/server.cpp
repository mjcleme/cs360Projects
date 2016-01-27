#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include "cs360utils.h"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define QUEUE_SIZE          5
#define MAX_MSG_SZ          1024
#define HOST_NAME_SIZE      255

using namespace std;

//Reads from the socket, and writes a response
void readFromWriteToSocket(int hSocket, string basedir) {
    vector<char *> headerLines;
    char* resource;

    //Get the headers
    GetHeaderLines(headerLines, hSocket, false);

    if (headerLines.size() == 0) {
        //If we don't receive headers, refuse the connection so we don't crash.
        return;
    }

    resource = strtok(headerLines[0], " ");
    resource = strtok(NULL, " ");

    string resourceStr = resource;

    string path = basedir + resourceStr;
    //Get file, if it can be gotten
    struct stat filestat;
    
    char* buffer;
    //Set content type to html by default
    string contentType = "text/html";
    int imageSize = 0;

    bool image = false;
    bool error = false;
    //If not found, basically
    if(stat(path.c_str(), &filestat)) {
        error = true;
        //Generate our error message and stick it into the buffer
        string errorMessage = "<!DOCTYPE html>\n<head></head>\n<body><h1>404</h1><h3>File not found.</h3></body>\n</html>";
        buffer = (char*)malloc(errorMessage.length()+1);
        (buffer, errorMessage.c_str(), errorMessage.length());
        buffer[errorMessage.length()] = '\0';
    }
    //If a file
    else if(S_ISREG(filestat.st_mode)) {
        //If a text file
        if (path.substr(path.size()-3, path.size()-1) == "txt") {
            FILE *fp = fopen(path.c_str(), "r");
            buffer = (char *)malloc(filestat.st_size+1);
            memset(buffer, 0, filestat.st_size);
            fread(buffer, filestat.st_size, 1, fp);
            buffer[filestat.st_size] = '\0';
            contentType = "text/plain";
            fclose(fp);
        }
        //If an html file
        else if (path.substr(path.size()-4, path.size()-1) == "html") {
            FILE *fp = fopen(path.c_str(), "r");
            buffer = (char *)malloc(filestat.st_size);
            memset(buffer, 0, filestat.st_size);
            fread(buffer, filestat.st_size, 1, fp);
            buffer[filestat.st_size] = '\0';
            fclose(fp);
        }
        //If a jpg image
        else if (path.substr(path.size()-3, path.size()-1) == "jpg") {
            FILE *fp = fopen(path.c_str(), "r");
            buffer = (char *)malloc(filestat.st_size+1);
            memset(buffer, 0, filestat.st_size);
            fread(buffer, filestat.st_size, 1, fp);
            fclose(fp);
            image = true;
            imageSize = filestat.st_size;
            contentType = "image/jpg";
        }
        else if (path.substr(path.size()-3, path.size()-1) == "gif") {
            FILE *fp = fopen(path.c_str(), "r");
            buffer = (char *)malloc(filestat.st_size+1);
            memset(buffer, 0, filestat.st_size);
            fread(buffer, filestat.st_size, 1, fp);
            fclose(fp);
            image = true;
            imageSize = filestat.st_size;
            contentType = "image/gif";
        }
        else {
            //404 if not one of these types
            error = true;
            //Generate our error message and stick it into the buffer
            string errorMessage = "<!DOCTYPE html>\n<head></head>\n<body><h1>404</h1><h3>File not found.</h3></body>\n</html>";
            buffer = (char*)malloc(errorMessage.length()+1);
            memcpy(buffer, errorMessage.c_str(), errorMessage.length());
            buffer[errorMessage.length()] = '\0';
        }
    }
    else if(S_ISDIR(filestat.st_mode)) {
        int len = 0;
        DIR *dirp;
        struct dirent *dp;
        vector<string> dirList;

        dirp = opendir(path.c_str());
        while ((dp = readdir(dirp)) != NULL) {
            char* listing = (char*)malloc(MAX_MSG_SZ);
            sprintf(listing, "%s", dp->d_name);
            string temp = listing;
            dirList.push_back(temp);
            free(listing);
        }
        (void)closedir(dirp);
        string directoryTempString = "<html>\n<head></head>\n<body>\n";
        if (resourceStr == "/") resourceStr = "";
        for (int i = 0; i < dirList.size(); i++) {
            if (dirList[i] != "." && dirList[i] != "..") {
                directoryTempString += "<a href=\"" + resourceStr + "/" + dirList[i] + "\">";
                directoryTempString += "<h4>" + dirList[i] + "</h4>";
                directoryTempString += "</a>\n";
            }
        }
        directoryTempString += "</body>\n</html>";
        buffer = (char*)malloc(directoryTempString.length()+1);
        copy(directoryTempString.begin(), directoryTempString.end(), buffer);
        buffer[directoryTempString.size()] = '\0';
        contentType = "text/html";
    }

    //Writing stuff
    char* headers;
    if (!image && !error) {
        //Create response headers
        headers = (char*)malloc(MAX_MSG_SZ);
        sprintf(headers, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %d\r\n\r\n", contentType.c_str(), strlen(buffer));
        
        if (write(hSocket, headers, strlen(headers)) != SOCKET_ERROR) {
        }
        else {
            perror("Error writing response headers");
        }

        if (write(hSocket, buffer, strlen(buffer)) != SOCKET_ERROR) {
        }
        else {
            perror("Error writing response body");
        }
    }
    else if (error) {
        //Create response headers
        headers = (char*)malloc(MAX_MSG_SZ);
        sprintf(headers, "HTTP/1.1 404 Not Found\nContent-Type: %s\nContent-Length: %d\r\n\r\n", contentType.c_str(), strlen(buffer));

        if (write(hSocket, headers, strlen(headers)) != SOCKET_ERROR) {
        }
        else {
            perror("Error writing response headers");
        }

        if (write(hSocket, buffer, strlen(buffer)) != SOCKET_ERROR) {
        }
        else {
            perror("Error writing response body");
        }
    }
    else {
        //Create response headers
        headers = (char*)malloc(MAX_MSG_SZ);
        sprintf(headers, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %d\r\n\r\n", contentType.c_str(), imageSize);

        if (write(hSocket, headers, strlen(headers)) != SOCKET_ERROR) {
        }
        else {
            perror("Error writing response headers");
        }

        if (write(hSocket, buffer, imageSize) != SOCKET_ERROR) {
        }
        else {
            perror("Error writing response body");
        }
        free(buffer);
    }
    

    //Free the lines from the headerLines vector
    for (int i = 0; i < headerLines.size(); i++) {
        free(headerLines[i]);
    }
    free(headers);
}

void handler (int status) {
    printf("Received signal %d\n", status);
}

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;
    char baseDir[MAX_MSG_SZ];

    if(argc < 3)
      {
        printf("\nUsage: server host-port base-dir\n");
        return 0;
      }
    else
      {
        nHostPort=atoi(argv[1]);
        strcpy(baseDir, argv[2]);
      }


    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        perror("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;


    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        perror("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);



    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        perror("\nCould not listen\n");
        return 0;
    }
    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    //Ensures the server is not killed by PIPE or HUP signals
    struct sigaction sigold, signew;
    signew.sa_handler=handler;
    sigemptyset(&signew.sa_mask);
    sigaddset(&signew.sa_mask,SIGINT);
    signew.sa_flags = SA_RESTART;
    sigaction(SIGHUP,&signew,&sigold);
    sigaction(SIGPIPE,&signew,&sigold);

    for(;;)
    {
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);


        readFromWriteToSocket(hSocket, argv[2]);

        /* close socket */
        #ifdef notdef
        linger lin;
        unsigned int y = sizeof(lin);
        lin.1_onoff=1;
        lin.1_linger=10;
        setsockopt(hSocket, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin));
        shutdown(hSocket, SHUT_RDWR);
        #endif
        if(close(hSocket) == SOCKET_ERROR)
        {
         perror("\nCould not close socket\n");
         return 0;
        }
    }
}
