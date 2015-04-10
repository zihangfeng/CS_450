/*  450UtilsUDP.h

    This file contains a number of useful functions for use in CS 450 HW
        
    Written by John T. Bell January 2014 for CS 450
*/

#ifndef CS450UTILSUDP_H
#define CS450UTILSUDP_H

#include <cstdlib>
#include <cstdio>

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>


#include "CS450Header7.h"

using namespace std;

void printIP( ostream & out, uint32_t IP );

ostream & operator << ( ostream & out, const CS450Header & h );
ostream & operator << ( ostream & out, const struct sockaddr_in & addr );

uint16_t calcCheckSum( void *data, int nbytes );
/* Algorithm:
    1. Typecast data into a uint16_t * data16, and also into a uint8_t * data8
    2. Set uint32_t sum = 0, max16 = 0xffff;
    3. Loop for i = 0 to nbytes / 2
        A. add data16[ i ] to sum
        B. if sum > max16, subtract max16 from sum
    4. if nbytes is odd
        A. add data8[ nbytes - 1 ] to sum
        B. if sum > max16, subtract max16 from sum
    5. Return the one's complement of sum.
    
    To use:
        1. Set checksum field of header to 0
        2. Calculate checksum of Packet 
        3. Store result in checksum field of header.
        4. Now if you calculate the checksum of the packet,
            the result should be zero.
*/


void packHeader( CS450Header & h, int32_t version, int32_t UIN, 
    int32_t transactionNumber, int32_t sequenceNumber, 
    int32_t ackNumber, uint32_t from_IP, uint32_t to_IP, 
    uint32_t trueFromIP, uint32_t trueToIP, uint32_t nbytes,
    uint32_t nTotalBytes, const char * filename, uint16_t from_Port,
    uint16_t to_Port, uint16_t checksum, int8_t HW_number, int8_t packetType, 
    int8_t relayCommand, int8_t saveFile, const char * ACCC, int8_t dropChance, 
    int8_t dupeChance, int8_t garbleChance, int8_t protocol );
    
void unpackHeader( const CS450Header & h, int32_t & version, int32_t & UIN, 
    int32_t & transactionNumber, int32_t & sequenceNumber, 
    int32_t & ackNumber, uint32_t & from_IP, uint32_t & to_IP, 
    uint32_t & trueFromIP, uint32_t & trueToIP, uint32_t & nbytes,
    uint32_t & nTotalBytes, char * filename, uint16_t & from_Port,
    uint16_t & to_Port, uint16_t & checksum, int8_t & HW_number, 
    int8_t & packetType, int8_t & relayCommand, int8_t & saveFile, char * ACCC,
    int8_t & dropChance, int8_t & dupeChance, int8_t & garbleChance,
    int8_t & protocol );

#endif












