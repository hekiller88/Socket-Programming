//
//  UDPEchoServer.h
//  UDPEchoServer
//
//  Created by Liu He on 11/3/17.
//  Copyright © 2017 Liu He. All rights reserved.
//

#ifndef UDPEchoServer_h
#define UDPEchoServer_h

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "ServerTable.h"

typedef int bool;
#define true 1
#define false 0

typedef struct{
    enum {Login, Follow, Post, Search, Delete, Unfollow, Logout }
        request_Type;    // same size as an unsigned int(32 bits)
    unsigned int userID;                // unique client identifier
    char message[140];                    // text message
    unsigned int LeaderID;
} ClientMessage;

typedef struct{
    unsigned int LeaderID;                // unique client identifier
    char message[140];                    // text message
} ServerMessage;

void DieWithError(char *errorMessage);  /* External error handling function */

void serverMessage_init(ServerMessage* msg);
void server_answer_Login(ServerTable* table, ServerMessage* servMsg);
void server_answer_Follow(ServerTable* table, ServerMessage* servMsg, int lid);
void server_answer_Unfollow(ServerTable* table, ServerMessage* servMsg, int lid);
void server_answer_Post(ServerTable* table, ServerMessage* servMsg, char* msg);

ServerTable allTable[50];       //using to store online user's table
int cntTable;                   //current online user

int allTable_get_i_UserID(int uid);       //get online table by UserID, if not online return -1
void allTable_add(ServerTable table);     //add user to online table, if already exist, overlap the old record
void allTable_delete(ServerTable* table); //delete the user from online table

#endif /* UDPEchoServer_h */
