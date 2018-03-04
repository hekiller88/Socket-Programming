//
//  ServerTable.h
//  ArrayList
//
//  Created by Liu He on 11/1/17.
//  Copyright © 2017 Liu He. All rights reserved.
//

#ifndef ServerTable_h
#define ServerTable_h

#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */

#define SIZE_LID 50
#define SIZE_MSG 50
#define SIZE_FID 50

typedef int bool;
#define true 1
#define false 0

//struct using for record
typedef struct{
    
    int index_lid;
    int index_fid;
    int index_msg;
    
    unsigned int UserID;
    struct sockaddr_in UserAddr;
    unsigned int LeaderID[SIZE_LID];
    unsigned int FollowerID[SIZE_FID];
    char *message[SIZE_MSG];

}ServerTable;

void table_init(ServerTable* table);        //initilize table

void table_setAddr(ServerTable* table,struct sockaddr_in addr);

void table_setUid(ServerTable* table, int id);

void table_addLid(ServerTable* table, int id);

void table_addFid(ServerTable* table, int id);

void table_addMsg(ServerTable* table, char* msg);

bool table_deleteLid(ServerTable* table, int id);

void table_deleteFid(ServerTable* table, int id);

void table_deleteMsg_i(ServerTable* table,int index);

bool table_deleteMsg(ServerTable* table,char* msg);

char* table_getMsg_i(ServerTable* table, int index);        //get message by index

void table_free(ServerTable* table);                        //free the mem of table

void table_log(ServerTable* table);                         //log printout, using for debug

void table_write(ServerTable* table);                       //write table into a file

bool table_read(ServerTable* table, int userID);            //read table from a file

void table_copy(ServerTable* target, ServerTable src);      //copy table

bool table_searchTag(ServerTable* table, char* tag,int i_msg);  //searching tag

#endif
