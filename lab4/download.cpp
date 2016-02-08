#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/epoll.h>
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
    int hSockets[NCONNECTIONS];                 /* handle to socket */
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
    bool counting = false;
    int count;
    int countCopy;
    int headerNum;
    struct epoll_event event;
    struct epoll_event events[NCONNECTIONS];

    if(argc < 4)
      {
        printf("\nUsage: download [-d, -c count] host-name host-port resource\n");
        return 0;
      }
    else
      {
        int c;
        while ((c = getopt(argc, argv, "c:d")) != -1) {
        	switch (c) {
        		case 'c':
        			counting = true;
        			for (int i = 0; i < strlen(optarg); i++) {
        				if (!isdigit(optarg[i])) {
        					perror("-c must be followed by a number");
        					return 0;
        				}
        			}
        			if (sscanf(optarg, "%d", &count) == EOF) {
        				perror("-c must be followed by a number");
        				return 0;
        			}
        			countCopy = count;
        			break;
        		case 'd':
        			debugging = true;
        			break;
        		default:
        			break;
        	}
        }
        if (argc - optind == 3) {
        	strcpy(strHostName,argv[optind]);
        	for (int i = 0; i < strlen(argv[optind+1]); i++) {
        		if (!isdigit(argv[optind+1][i])) {
        			perror("Port must be a number");
        			return 0;
        		}
        	}
        	nHostPort=atoi(argv[optind+1]);
        	strcpy(resource, argv[optind+2]);
        }
        else {
            printf("Invalid command line option\n");
            return 0;
        }
      }
    
    do	{
        int epollfd = epoll_create(20);
        
        for (int i = 0; i < NCONNECTIONS; i++) {
	        /* make a socket */
		    hSockets[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		    if (debugging) {
		    	printf("Made a socket\n");
		    }
		    if(hSockets[i] == SOCKET_ERROR)
		    {
		    	perror("\nCould not make a socket\n");
		    	return 0;
		    }
	
		    /* get IP address from name */
		    pHostInfo=gethostbyname(strHostName);
		    if (debugging) {
		    	printf("Got host by name\n");
		    }
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
	
		    if (debugging) {
		    	printf("About to connect to host\n");
		    }	/* connect to host */
	    	if(connect(hSockets[i],(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
	    	{
	    		//It takes forever to time out, but does let you know eventually (2-3 minutes?)
	    		perror("\nCould not connect to host\n");
	    		return 0;
	    	}
            event.events = EPOLLIN;
            event.data.fd = hSockets[i];
            if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, hSockets[i], &event)) != 0) {
                perror("epoll_ctl failed\n");
                return 0;
            }
            if (debugging) {
	    		printf("Connected to host\n");
	    	}
	    	
	    	/* write a request to the server */
	    	#define MAXMSG 1024
	    	char* message = (char*)malloc(MAXMSG);
	    	sprintf(message, "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n", resource, strHostName, nHostPort);
	    	if (debugging) {
	    		printf("\nHTTP Request:\nMessage: %s",message);
	    	}
	    	memset(pBuffer, 0, BUFFER_SIZE);
	    	write(hSockets[i], message, strlen(message));
        	//    chomp(message);
        	//    printf("\nWriting \"%s\" to server\n",message);
    		free(message);
    	

        }
        
            //Get epoll_wait return value
            int rval = epoll_wait(epollfd, events, NCONNECTIONS, 30000);
            if (debugging) {
                printf("epoll_wait return value: %d\n", rval);
            }

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
            	if (debugging) {
            		printf("Headers:\n");
            	}
            	for (int j = 0; j < headerLines.size(); j++) {
            		if (debugging) {
            			printf("[%d] %s\n", j, headerLines[j]);
            		}
            		if (strstr(headerLines[j], "Content-Length")) {
                        //Need to make contentLength actually point somewhere.
                        contentLength = (int*)malloc(64);
            			sscanf(headerLines[j], "Content-Length: %d", contentLength);
            			responseBuffer = (char*)malloc(*contentLength * sizeof(char));
            		}
            	}

                if (debugging) {
            	    printf("\n==============================\n");
            	    printf("Headers are finished, now read the file\n");
            	    printf("Content Length is %d\n", *contentLength);
            	    printf("==============================\n");
            	}

            	int rval2;
            	if (debugging && !counting) {
            		printf("\nResponse Body:");
            	}
            	if ((rval2 = read(events[i].data.fd, responseBuffer, *contentLength)) > 0) {
            		if (!counting) {
            			printf("\n%s\n", responseBuffer);
            		}
            	}
                else {
                    perror("Error reading response");
                }
            
                if (debugging) {
            	    printf("\nClosing socket\n");
                }
            	/* close socket */                       
            	if(close(hSockets[i]) == SOCKET_ERROR)
            	{
            		perror("\nCould not close socket\n");
            		return 0;
            	}
                if (debugging) {
                    printf("Freeing stuff\n");
                }
            	free(responseBuffer);
            	headerNum = headerLines.size();
            	for (int j = 0; j < headerNum; j++) {
            		free(headerLines[j]);
            	}
            	for (int j = 0; j < headerNum; j++) {
            		headerLines.erase(headerLines.begin());
            	}
            }

	//    free(startline);
		if (counting) {
//			printf("Count is %d\n", count);
			count--;
		}
    } while(counting && count > 0);
    if (counting) {
    	printf("Successfully downloaded the page %d times\n", countCopy);
    }
    return 0;
}
