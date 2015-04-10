/* client.cpp

    This program transfers a file to a remote server, possibly through an
    intermediary relay.  The server will respond with an acknowledgement.
    
    This program then calculates the round-trip time from the start of
    transmission to receipt of the acknowledgement, and reports that along
    with the average overall transmission rate.
    
    Written by _______________ for CS 450 HW1 January 2014
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CS450Header.h"


int main(int argc, char *argv[])
{
	printf("Zihang Feng.\n");
        printf("CS account jfeng9.\n");
        printf("665726671.\n");
        printf("This is the Client Application.\n");

    // User Input
    
    /* Check for the following from command-line args, or ask the user:
        
        Destination ( server ) name and port number
        Relay name and port number.  If none, communicate directly with server
        File to transfer.  Use OPEN(2) to verify that the file can be opened
        for reading, and ask again if necessary.
    */
	struct hostent *hostIP, *clntIP;
	int serPort, clntPort;
	
	if(argc < 2)
	{
		hostIP = gethostbyname("127.0.0.1");
                if(hostIP == NULL)
                {
                        printf("Can't convert the host IP\n");
                        exit(-1);
                }

		serPort = 54321;
	}
	else if(argc < 3)
	{
		hostIP = gethostbyname(argv[1]);
                if(hostIP == NULL)
                {
                        printf("Can't convert the host IP\n");
                        exit(-1);
                }
		serPort = 54321;
	}
	else
	{
		hostIP = gethostbyname(argv[1]);
		if(hostIP == NULL)
		{
			printf("Can't convert the host IP\n");
			exit(-1);
		}
		serPort = atoi(argv[2]);
	}

    // Use FSTAT and MMAP to map the file to a memory buffer.  That will let the
    // virtual memory system page it in as needed instead of reading it byte
    // by byte.

	char file_name[80];
	printf("Give a file name: ");
	scanf("%s",file_name);
	printf("\n");

	int filefd;
        if((filefd = open(file_name, O_RDONLY)) == -1)
        {
        	printf( "method open has problem.\n");
               	exit(-1);
        }

	struct stat filestatus;
	if(fstat(filefd, &filestatus) < 0)
	{
		printf("Can't find the size of the file.\n");
		exit(-1);
	}
	int size_of_file = filestatus.st_size;

	// missing a file size as the second argument of the mmap
	void *addr;
	if((addr = mmap(NULL, size_of_file, PROT_READ, MAP_PRIVATE, filefd, 0)) == (void *)-1)
        {
        	printf("method mmap has problem.\n");
		close(filefd);
               	exit(-1);
        }


	char clntHostName[255];
	if(gethostname(clntHostName,255) < 0)
	{
		printf("Can't get the client host name.\n");
		exit(-1);
	}

	if((clntIP = gethostbyname(clntHostName)) == NULL)
	{
		printf("Can't convert it the client host name.\n");
		exit(-1);
	}

        struct sockaddr_in sockClntAddr;

	clntPort = 54329;
        memset(&sockClntAddr, 0, sizeof(sockClntAddr));
        sockClntAddr.sin_family = AF_INET;
        sockClntAddr.sin_port = htonl(clntPort);
        sockClntAddr.sin_addr.s_addr= ((struct in_addr *)clntIP->h_addr)->s_addr;

    // Open a Connection to the server ( or relay )  TCP for the first HW
    // call SOCKET and CONNECT and verify that the connection opened.

	int socketfd = socket(AF_INET,SOCK_STREAM, 0);
	if(socketfd < 0)
	{
		printf("Socket Erorr.\n");
		close(filefd);
		exit(-1);
	}

	struct sockaddr_in sockSerAddr;

        memset(&sockSerAddr, 0, sizeof(sockSerAddr));
        sockSerAddr.sin_family = AF_INET;
        sockSerAddr.sin_port = htonl(serPort);
        sockSerAddr.sin_addr.s_addr= ((struct in_addr *)hostIP->h_addr)->s_addr;

	if(connect(socketfd,(struct sockaddr *)&sockSerAddr, sizeof(sockSerAddr)) < 0)
	{
		printf("Connect error.\n");
		close(filefd);
		close(socketfd);
		exit(-1);
	}

    // Note the time before beginning transmission
	clock_t start = clock();
    // Create a CS450Header object, fill in the fields, and use SEND to write
    // it to the socket.

	CS450Header headValue;
	headValue.version = 4;
	headValue.UIN = 665726671;
	headValue.HW_number = 1;
	strcpy(headValue.ACCC,"jfeng9");
	strcpy(headValue.filename, file_name);
	headValue.from_IP = htonl(sockClntAddr.sin_addr.s_addr);
 	headValue.to_IP = htonl(sockSerAddr.sin_addr.s_addr);
	headValue.nbytes = size_of_file;
	headValue.persistent = htonl(0);
	headValue.saveFile = htonl(1);
	headValue.from_Port = htons(clntPort);
	headValue.to_Port = htons(serPort);



    // Use SEND to send the data file.  If it is too big to send in one gulp
    // Then multiple SEND commands may be necessary.

        if(send(socketfd, &headValue, sizeof(headValue), 0) < 0)
        {
                printf("Send error.\n");
		close(socketfd);
		close(filefd);
                exit(-1);
        }

        if(send(socketfd, addr, sizeof(size_of_file), 0) < 0)
        {
                printf("Send error.\n");
                close(socketfd);
                close(filefd);
                exit(-1);
        }

    // Use RECV to read in a CS450Header from the server, which should contain
    // some acknowledgement information.

	ssize_t length_recieved = recv(socketfd,&headValue, sizeof(headValue), 0);
	if(length_recieved < 0)
	{
		printf("Recv Error.\n");
		close(socketfd);
		close(filefd);
		exit(-1);
	}

	if(headValue.packetType != 2)
	{
		printf("Can't receive the thing from the server.\n");
		exit(-1);
	}
	else
	{
		printf("The whole received.\n");
	}
    // Calculate the round-trip time and
    // bandwidth, and report them to the screen along with the size of the file
    // and output echo of all user input data.

	clock_t end = clock();
        float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	
	printf("Time it takes to the destination and back is %f\n", seconds);
        close(filefd);
        close(socketfd);
    // When the job is done, either the client or the server must transmit a
    // CS450Header containing the command code to close the connection, and 
    // having transmitted it, close their end of the socket.  If a relay is 
    // involved, it will transmit the close command to the other side before
    // closing its connections.  Whichever process receives the close command
    // should close their end and also all open data files at that time.
    
    // If it is desired to transmit another file, possibly using a different
    // destination, protocol, relay, and/or other parameters, get the
    // necessary input from the user and try again.
    
    // When done, report overall statistics and exit.

    system("PAUSE");
    return 0;
}

