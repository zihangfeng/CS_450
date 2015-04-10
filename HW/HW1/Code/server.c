/*  server.cpp

    This program receives a file from a remote client and sends back an 
    acknowledgement after the entire file has been received.
    
    Two possible options for closing the connection:
        (1) Send a close command along with the acknowledgement and then close.
        (2) Hold the connection open waiting for additional files / commands.
        The "persistent" field in the header indicates the client's preference.
        
    Written by _________________ January 2014 for CS 361
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "CS450Header.h"


int main(int argc, char *argv[])
{
	printf("Zihang Feng.\n");
	printf("CS account jfeng9.\n");
	printf("665726671.\n");
	printf("This is the Server Application.\n");
	int port;
    // User Input
    /* Check for the following from command-line args, or ask the user:
        
        Port number to listen to.  Default = 54321.
    */
        if(argc < 2)
        {
                port = 54321;
        }
        else
        {
                port = atoi(argv[1]);
        }

    //  Call SOCKET to create a listening socket
    //  Call BIND to bind the socket to a particular port number
    //  Call LISTEN to set the socket to listening for new connection requests.
    
	int socketfd;
	if((socketfd = socket(AF_INET,SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Erorr.\n");
		exit(-1);
	}
	
	struct sockaddr_in sockSerAddr, sockClntAddr;

	char hostName[255];
	if(gethostname(hostName, 255) < 0)
	{
		printf("Can't get the host name.\n");
		exit(-1);
	}

	struct hostent *hostIP;
	if((hostIP = gethostbyname(hostName)) == NULL)
	{
        	printf("Can't convert the host IP\n");
                exit(-1);
	}
        memset(&sockSerAddr, 0, sizeof(sockSerAddr));
	memset(&sockClntAddr, 0, sizeof(sockClntAddr));
        sockSerAddr.sin_family = AF_INET;
        sockSerAddr.sin_port = htonl(port);
        sockSerAddr.sin_addr.s_addr= ((struct in_addr *)hostIP->h_addr)->s_addr;

        if(bind(socketfd,(struct sockaddr *)&sockSerAddr, sizeof(sockSerAddr)) < 0)
	{
		printf("Bind Erorr.\n");
		exit(-1);
	}

	if(listen(socketfd, 10) < 0)
	{
		printf("Listen Erorr.\n");
		exit(-1);
	}
    // For HW1 only handle one connection at a time
    
    // Call ACCEPT to accept a new incoming connection request.
    // The result will be a new socket, which will be used for all further
    // communications with this client.
    
    // Call RECV to read in one CS450Header struct
    
    // Then call RECV again to read in the bytes of the incoming file.
    //      If "saveFile" is non-zero, save the file to disk under the name
    //      "filename".  Otherwise just read in the bytes and discard them.
    //      If the total # of bytes exceeds a limit, multiple RECVs are needed.
    
	int clntfd;
	CS450Header headValue, headValueAck;
        while(1)
        {
                if((clntfd = accept(socketfd,(struct sockaddr *)&sockClntAddr, sizeof(sockClntAddr))) < 0)
		{
			printf("Can't accept.\n");
		}

		while(1)
		{
			if(recv(clntfd, &headValue, sizeof(headValue), 0) < 0)
			{
				printf("Recv Erorr.\n");
				break;
			}

			if(headValue.saveFile != 0)//save the file
			{
				char file_name[80];
				strcpy(file_name,headValue.filename);
				int filefd = creat(file_name,0600);
				if(filefd < 0)
				{
					printf("Can't create a new file.\n");
					break;
				}

				void *addr;
        			if((addr = mmap(NULL, headValue.nbytes, PROT_READ, MAP_PRIVATE, filefd, 0)) < (void *)0)
        			{
                			printf("method mmap has problem.\n");
                			close(filefd);
                			break;
        			}

                        	if(recv(clntfd, addr, sizeof(addr), 0) < 0)
                        	{
                                	printf("Recv Erorr.\n");
					close(filefd);
                                	break;
                        	}
				munmap(addr, sizeof(addr));
				close(filefd);
			}
			else// not to save the file in the local disk
			{
				char buf[256];
	                	if(recv(clntfd, buf, sizeof(buf), 0) < 0)
                                {
                                        printf("Recv Erorr.\n");
                                        break;
                                }
			}

    // Send back an acknowledgement to the client, indicating the number of 
    // bytes received.  Other information may also be included.
			memcpy(&headValueAck,&headValue, sizeof(headValueAck));
			headValueAck.from_IP = headValue.to_IP;
			headValueAck.to_IP = headValue.from_IP;
			headValueAck.packetType = htonl(2);
			if(send(clntfd, &headValueAck, sizeof(headValueAck), 0) < 0)
        		{
                		printf("Send error.\n");
                		break;
        		}

			if(headValue.persistent == 0)
			{
				break;// to close the connection
			}
                }// inner while loop

                close(clntfd);
        }

    // If "persistent" is zero, then include a close command in the header
    // for the acknowledgement and close the socket.  Go back to ACCEPT to 
    // handle additional requests.  Otherwise keep the connection open and
    // read in another Header for another incoming file from this client.
    
    
    system("PAUSE");
    return 0;
}

