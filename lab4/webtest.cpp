#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <vector>
#include <sstream>
#include <iostream>

#include "cs360utils.h"

using namespace std;

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define MAX_MSG_SZ      	1024
#define NCONNECTIONS        20


int  main(int argc, char* argv[])
{
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int* contentLength;
    int nHostPort = 0;
    char resource[HOST_NAME_SIZE];
    vector<char *> headerLines;
    bool debugging = false;
    int count = 0;
    int headerNum;

    if(argc < 4)
      {
        printf("\nUsage: webtest [-d] host-name host-port resource count\n");
        return 0;
      }
    else
      {
        int c;
        while ((c = getopt(argc, argv, "d")) != -1) {
        	switch (c) {
        		case 'd':
        			debugging = true;
        			break;
        		default:
        			break;
        	}
        }
        if (argc - optind == 4) {
        	strcpy(strHostName,argv[optind]);
        	for (int i = 0; i < strlen(argv[optind+1]); i++) {
        		if (!isdigit(argv[optind+1][i])) {
        			perror("Port must be a number");
        			return 0;
        		}
        	}
        	nHostPort=atoi(argv[optind+1]);
        	strcpy(resource, argv[optind+2]);
            count=atoi(argv[optind+3]);
        }
        else {
            printf("Invalid command line option\n");
            return 0;
        }
      }
    
      int hSockets[count];                 /* handle to socket */
      double times[count];
      struct timeval oldtime[count];
      struct timeval newtime[count];
      struct epoll_event event[count];
      struct epoll_event events[count];

      int epollfd = epoll_create(count);
      
      for (int i = 0; i < count; i++) {
	      /* make a socket */
	      hSockets[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	      if(hSockets[i] == SOCKET_ERROR)
	      {
	      	perror("\nCould not make a socket\n");
	      	return 0;
	      }
	
	      /* get IP address from name */
	      pHostInfo=gethostbyname(strHostName);
	      if (pHostInfo == 0) {
	      	printf("\nHost is not responding or does not exist\nExiting client\n");
	      	return 0;
	      }
	      /* copy address into long */
	      memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);
	
	      /* fill address struct */
	      Address.sin_addr.s_addr=nHostAddress;
	      Address.sin_port=htons(nHostPort);
	      Address.sin_family=AF_INET;
	
	      /* connect to host */
	  	if(connect(hSockets[i],(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
	  	{
	  		//It takes forever to time out, but does let you know eventually (2-3 minutes?)
	  		perror("\nCould not connect to host\n");
	  		return 0;
	  	}
          event[i].events = EPOLLIN;
          event[i].data.fd = hSockets[i];
          if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, hSockets[i], &event[i])) != 0) {
              perror("epoll_ctl failed\n");
              return 0;
          }
	  	
	  	/* write a request to the server */
	  	#define MAXMSG 1024
	  	char* message = (char*)malloc(MAXMSG);
	  	sprintf(message, "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n", resource, strHostName, nHostPort);
	  	memset(pBuffer, 0, BUFFER_SIZE);
	  	write(hSockets[i], message, strlen(message));
      	//    chomp(message);
      	//    printf("\nWriting \"%s\" to server\n",message);
      	free(message);
          gettimeofday(&oldtime[i], NULL);
      }
      
      int numRead = 0;
      while (numRead < count) {
          //Get epoll_wait return value
          int rval = epoll_wait(epollfd, events, count, -1);

          for (int i = 0; i < rval; i++) {
          	/* read from socket into buffer
          	** number returned by read() and write() is the number of bytes
          	** read or written, with -1 being that an error occured */
          	//    nReadAmount=read(hSockets[i], pBuffer, BUFFER_SIZE);
          	//    printf("%s\n",pBuffer);
          	//    char * startline = GetLine(hSockets[i]);
          	//    printf("Status line %s\n\n", startline);
          	
          	GetHeaderLines(headerLines, events[i].data.fd, false);
          	
          	char* responseBuffer;
          	for (int j = 0; j < headerLines.size(); j++) {
          		if (strstr(headerLines[j], "Content-Length")) {
                    //Need to make contentLength actually point somewhere.
                    contentLength = (int*)malloc(64);
          			sscanf(headerLines[j], "Content-Length: %d", contentLength);
          			responseBuffer = (char*)malloc(*contentLength * sizeof(char));
          		}
          	}


          	int rval2;
          	if ((rval2 = read(events[i].data.fd, responseBuffer, *contentLength)) > 0) {
                int newtimeCounter = event[count-1].data.fd-events[i].data.fd;
                gettimeofday(&newtime[newtimeCounter],NULL);
                double usec = (newtime[newtimeCounter].tv_sec - oldtime[newtimeCounter].tv_sec)*(double)1000000+(newtime[newtimeCounter].tv_usec-oldtime[newtimeCounter].tv_usec);
                times[newtimeCounter] = usec/1000000;
                if (debugging) {
                    std::cout << "Response returned in " << usec/1000000 << " seconds" << std::endl;
                }
          	}
            else {
                perror("Error reading response");
            }
          
          	/* close socket */                       
          	if(close(events[i].data.fd) == SOCKET_ERROR)
          	{
          		perror("\nCould not close socket\n");
          		return 0;
          	}
          	free(responseBuffer);
          	headerNum = headerLines.size();
          	for (int j = 0; j < headerNum; j++) {
          		free(headerLines[j]);
          	}
          	for (int j = 0; j < headerNum; j++) {
          		headerLines.erase(headerLines.begin());
          	}
              numRead++;
          }
      }
    //Calculate average response time
    double sum = 0;
    for (int i = 0; i < count; i++) {
        sum += times[i];
    }
    double avgResponseTime = sum/count;
    //Calculate standard deviation
    double variance = 0;
    for (int i = 0; i < count; i++) {
        double difference = (times[i] - avgResponseTime);
        variance += difference*difference;
    }
    variance /= count;
    double stdDeviation = sqrt(variance);
    std::cout << "Average Response Time: " << avgResponseTime << " seconds" << std::endl;
    std::cout << "Standard Deviation: " << stdDeviation << " seconds" << std::endl;
    return 0;
}
