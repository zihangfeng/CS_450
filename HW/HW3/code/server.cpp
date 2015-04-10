/*  server.cpp

    This program receives a file from a remote client and sends back an 
    acknowledgement after the entire file has been received.
    
    Two possible options for closing the connection:
        (1) Send a close command along with the acknowledgement and then close.
        (2) Hold the connection open waiting for additional files / commands.
        The "persistent" field in the header indicates the client's preference.
        
    Written by _________________ January 2014 for CS 450
*/


#include <iostream>
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
#include "CS450Header7.h"
#include "450UtilsUDP.h"

#define WINSIZE 10

using namespace std;


void ackSend(int UDPsockfd, CS450Header headerAck, struct sockaddr_in clntAddr, int clntLen)
{
	int count = 0;
        while(sendto(UDPsockfd, &headerAck, sizeof(headerAck), 0, (const struct sockaddr *)&clntAddr, clntLen) < 0 && count < 10)
        {
        	printf("Can't send and the remaining chance is %d", 9-count);
                if(count == 9)
                {
                	printf("There is no way to send, the server program is terminated.\n");
                        exit(-1);
                }
                count++;
        }
}

void revcFile(int UDPsockfd, Packet pack, struct sockaddr_in clntAddr, socklen_t clntLen)
{
int rn, filefd, number_garble = 0;
uint16_t checksumVal;
CS450Header headerAck;
int8_t toSaveFile = pack.header.saveFile;
uint32_t bytesReceived, nTotalBytes = pack.header.nTotalBytes;


char file_name[80];
strcpy(file_name,pack.header.filename);

filefd = creat(file_name,0600);

if(filefd < 0)
{
	printf("Can't create a new file.\n");
        exit(-1);
}

	bytesReceived = 0;
	headerAck.ackNumber = 0;
	while(bytesReceived < nTotalBytes)
        {
        	rn = recvfrom(UDPsockfd, &pack, PacketSize,0, (struct sockaddr *)&clntAddr, &clntLen);
                if(rn < 0)
              	{
			if(bytesReceived > 0)
			{
                		bytesReceived -= pack.header.nbytes;
			}
			printf("recv not right.");
                       	headerAck.ackNumber = pack.header.sequenceNumber;
                       	ackSend(UDPsockfd, headerAck, clntAddr, clntLen);
                }
                else
                {
			// to check if it is the expected packet
			if(headerAck.ackNumber == pack.header.sequenceNumber)
			{
				checksumVal = pack.header.checksum;
				pack.header.checksum = 0;
               			if(calcCheckSum(&pack, PacketSize) == checksumVal)
                        	{
                        		if(toSaveFile)// the file from the client needs to be saved
                                	{
                                		write(filefd, pack.data, pack.header.nbytes);
                                	}
					bytesReceived += pack.header.nbytes;
                                	headerAck.ackNumber = pack.header.sequenceNumber + 1;// one packet received
                                	ackSend(UDPsockfd, headerAck, clntAddr, clntLen);
                        	}
                       		else
                       		{
					number_garble++;
					if(bytesReceived > 0)
					{
						bytesReceived -= pack.header.nbytes;
					}
                        		headerAck.ackNumber = pack.header.sequenceNumber;// get the wrong package from the client
                                	ackSend(UDPsockfd, headerAck, clntAddr, clntLen);
					printf("Garbled.");
                        	}
			}
			else
			{
				printf("ack and sequnce number does not match.\n");
			}
                }

	}
	close(filefd);
	printf("The nubmer of garbled file is %d.\n",number_garble);
	printf("one file has been received accurately.\n");

}
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
	port = 54323;
}
else
{
	port = atoi(argv[1]);
}

    //  Call SOCKET to create a listening socket
    //  Call BIND to bind the socket to a particular port number
    //  Call LISTEN to set the socket to listening for new connection requests.

int UDPsockfd;
if((UDPsockfd = socket(AF_INET,SOCK_DGRAM, 0)) < 0)
{
	printf("Socket Erorr.\n");
	exit(-1);
}

struct sockaddr_in localAddr, clntAddr;

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
memset(&localAddr, 0, sizeof(localAddr));
memset(&clntAddr, 0, sizeof(clntAddr));
localAddr.sin_family = AF_INET;
localAddr.sin_port = htons(port);
localAddr.sin_addr.s_addr = ((struct in_addr *)hostIP->h_addr)->s_addr;;

if(bind(UDPsockfd,(struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
{
	printf("Bind Erorr.\n");
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

Packet pack;
CS450Header headerAck;
socklen_t clntLen;

while(1)
{

	clntLen = sizeof(clntAddr);
	int rn = recvfrom(UDPsockfd, &pack, PacketSize,0, (struct sockaddr *)&clntAddr, &clntLen);
	if(rn < 0)
	{
		printf("The recv value is less than 0\n");
		headerAck.ackNumber = 0;
		ackSend(UDPsockfd, headerAck, clntAddr, clntLen);
	}
	else// got the header file accurately
	{
		headerAck.ackNumber = 1;
		headerAck.windowSize = WINSIZE;
		ackSend(UDPsockfd, headerAck, clntAddr, clntLen);
		revcFile(UDPsockfd, pack, clntAddr, clntLen);
	}// finish getting the whole file

    // Send back an acknowledgement to the client, indicating the number of 
    // bytes received.  Other information may also be included.

	memcpy(&headerAck,&pack.header, sizeof(pack.header));
	headerAck.from_IP = pack.header.to_IP;
	headerAck.to_IP = pack.header.from_IP;
	headerAck.packetType = 1;
	headerAck.from_Port = pack.header.to_Port;
	headerAck.to_Port = pack.header.from_Port;
	ackSend(UDPsockfd, headerAck, clntAddr, clntLen);

	if(headerAck.relayCommand)
	{
		printf("Close the connection with the client.\n");
		close(UDPsockfd);
	}

}// outer while loop

    // If "persistent" is zero, then include a close command in the header
    // for the acknowledgement and close the socket.  Go back to ACCEPT to 
    // handle additional requests.  Otherwise keep the connection open and
    // read in another Header for another incoming file from this client.
    

    return 0;
}

