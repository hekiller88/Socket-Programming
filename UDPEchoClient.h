//
//  UDPEchoClient.h
//  UDPEchoClient
//
//  Created by Liu He on 11/3/17.
//  Copyright © 2017 Liu He. All rights reserved.
//

#ifndef UDPEchoClient_h
#define UDPEchoClient_h

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define ECHOMAX 255     /* Longest string to echo */

typedef int bool;
#define true 1
#define false 0

typedef struct{
    enum {Login, Follow, Post, Search, Delete, Unfollow, Logout }
        request_Type;                   // same size as an unsigned int(32 bits)
    unsigned int UserID;                // unique client identifier
    char message[140];                  // text message
    unsigned int LeaderID;          // follows at most 50 leader
} ClientMessage;

typedef struct{
    unsigned int LeaderID;                // unique client identifier
    char message[140];                    // text message
} ServerMessage;

void DieWithError(char *errorMessage);  /* External error handling function */

void log_ClientMessage(ClientMessage msg);              //using for debug

void clientMessage_init(ClientMessage* msg);            //initialize ClientMessage
void client_askRequestType(ClientMessage * cliMsg);     
void client_askUserID(ClientMessage * cliMsg);
void client_askUserAddr(ClientMessage * cliMsg);
void client_askMessage(ClientMessage* cliMsg);
void client_askFollow(ClientMessage* cliMsg);
void client_askPost(ClientMessage* cliMsg);
void client_askDelete(ClientMessage* cliMsg, int flag);
void client_askUnfollow(ClientMessage* cliMsg, int flag);


#endif /* UDPEchoClient_h */
