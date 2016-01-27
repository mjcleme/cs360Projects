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

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define QUEUE_SIZE          5
#define MAX_MSG_SZ          1024
#define HOST_NAME_SIZE      255

using namespace std;

// Determine if the character is whitespace
bool isWhitespace(char c)
{ switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;

    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
        if (amtread >= 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}

// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;

    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');

        if (s[i] == '-')
            s[i] = '_';
    }

}

// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, const char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 1;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat)
{
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;

    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        if (strstr(tline, "Content-Length") ||
            strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, "");
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, "HTTP_");
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);
            }
        }
        //fprintf(stderr, "Header --> [%s]\n", line);

        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}

//Returns a file, directory listing, or error if path is not a file or directory
void returnFileOrDir(char* path) {

    struct stat filestat;

    if(stat(path, &filestat)) {
        printf("Should send 404");
    }
    if(S_ISREG(filestat.st_mode)) {
        FILE *fp = fopen(path, "r");
        char *buffer = (char *)malloc(filestat.st_size);
        fread(buffer, filestat.st_size, 1, fp);
        printf("File: \n%s\n", buffer);
        free(buffer);
        fclose(fp);
    }
    if(S_ISDIR(filestat.st_mode)) {
        int len;
        DIR *dirp;
        struct dirent *dp;

        dirp = opendir(path);
        while ((dp = readdir(dirp)) != NULL)
            printf("name %s\n", dp->d_name);
        (void)closedir(dirp);
    }

}

//Reads from the socket, and writes a response
void readFromWriteToSocket(int hSocket, string basedir) {
    vector<char *> headerLines;
    char* resource;

    //Get the headers
    GetHeaderLines(headerLines, hSocket, false);

    printf("Got this from the browser: \n");
    for (int i = 0; i < headerLines.size(); i++) {
        printf("%s\n", headerLines[i]);
    }

    resource = strtok(headerLines[0], " ");
    resource = strtok(NULL, " ");
    printf("\nResource: %s\n\n", resource);

    string path = basedir + resource;
    //Get file, if it can be gotten
    struct stat filestat;
    
    char* buffer;
    //Start out as a 404 error page, will be replaced if needed
    char* bodyText = "<!DOCTYPE html>\n<head></head>\n<body><h1>404</h1><h3>File not found.</h3></body>\n</html>";
    //Set content type to html by default, for 404
    string contentType = "text/html";
    int imageSize = 0;

    bool image = false;
    //If not found, basically
    if(stat(path.c_str(), &filestat)) {
        //Malloc buffer so we can free it later without problems.
        buffer = (char*)malloc(MAX_MSG_SZ);
        //404, do nothing
    }
    //If a file
    if(S_ISREG(filestat.st_mode)) {
        //If a text file
        if (path.substr(path.size()-3, path.size()-1) == "txt") {
            FILE *fp = fopen(path.c_str(), "r");
            buffer = (char *)malloc(filestat.st_size);
            fread(buffer, filestat.st_size, 1, fp);
            printf("File: \n%s\n", buffer);
            bodyText = buffer;
            contentType = "text/plain";
            fclose(fp);
        }
        //If an html file
        else if (path.substr(path.size()-4, path.size()-1) == "html") {
            FILE *fp = fopen(path.c_str(), "r");
            buffer = (char *)malloc(filestat.st_size);
            fread(buffer, filestat.st_size, 1, fp);
            printf("File: \n%s\n", buffer);
            bodyText = buffer;
            fclose(fp);
        }
        //If a jpg image
        else if (path.substr(path.size()-3, path.size()-1) == "jpg") {
            FILE *fp = fopen(path.c_str(), "r");
            printf("fileSize: %d\n", filestat.st_size);
            buffer = (char *)malloc(filestat.st_size+1);
            fread(buffer, filestat.st_size, 1, fp);
            fclose(fp);
            image = true;
            imageSize = filestat.st_size;
            contentType = "image/jpg";
        }
        else if (path.substr(path.size()-3, path.size()-1) == "gif") {
            FILE *fp = fopen(path.c_str(), "r");
            printf("fileSize: %d\n", filestat.st_size);
            buffer = (char *)malloc(filestat.st_size+1);
            fread(buffer, filestat.st_size, 1, fp);
            fclose(fp);
            image = true;
            imageSize = filestat.st_size;
            contentType = "image/gif";
        }
        else {
            //404 if not one of these types
            //Malloc buffer so it can be freed later without problems.
            buffer = (char*)malloc(MAX_MSG_SZ);
        }
    }
    if(S_ISDIR(filestat.st_mode)) {
        int len = 0;
        DIR *dirp;
        struct dirent *dp;
        vector<string> dirList;

        dirp = opendir(path.c_str());
        std::cout << "Starting while loop" << std::endl;
        while ((dp = readdir(dirp)) != NULL) {
            char* listing = (char*)malloc(MAX_MSG_SZ);
            sprintf(listing, "%s", dp->d_name);
            string temp = listing;
            dirList.push_back(temp);
            free(listing);
        }
        (void)closedir(dirp);
        std::cout << "dirp closed" << std::endl;
        std::cout << "malloc'd buffer" << std::endl;
        string directoryTempString = "";
        for (int i = 0; i < dirList.size(); i++) {
            directoryTempString += dirList[i] + "\n";
        }
        buffer = (char*)malloc(directoryTempString.length()+1);
        copy(directoryTempString.begin(), directoryTempString.end(), buffer);
        buffer[directoryTempString.size()] = '\0';
        printf("%s", buffer);
        bodyText = buffer;
    }

    char* headers;
    if (!image) {
        //Create response headers
        headers = (char*)malloc(MAX_MSG_SZ);
        sprintf(headers, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %d\r\n\r\n", contentType.c_str(), strlen(bodyText));
        
        if (write(hSocket, headers, strlen(headers)) != SOCKET_ERROR) {
            printf("Writing response headers to server:\n%s", headers);
        }
        else {
            perror("Error writing response headers");
        }

        if (write(hSocket, bodyText, strlen(bodyText)) != SOCKET_ERROR) {
            printf("Writing response body to server:\n%s", bodyText);
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
            printf("Writing response headers to client:\n%s", headers);
        }
        else {
            perror("Error writing response headers");
        }

        if (write(hSocket, buffer, imageSize) != SOCKET_ERROR) {
            printf("Writing an image to the client\n");
        }
        else {
            perror("Error writing response body");
        }
    }
    

    //Free the lines from the headerLines vector
    for (int i = 0; i < headerLines.size(); i++) {
        free(headerLines[i]);
    }
    free(headers);
    free(buffer);
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

    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d\n",nHostPort);

    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
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
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));

        readFromWriteToSocket(hSocket, argv[2]);

        printf("\nClosing the socket");
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
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
