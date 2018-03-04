//
//  ServerTable.c
//  ArrayList
//
//  Created by Liu He on 11/1/17.
//  Copyright © 2017 Liu He. All rights reserved.
//

#include "ServerTable.h"

void table_init(ServerTable* table)
{
//    memset(table, 0, sizeof(ServerTable));
    table->index_lid = 0;
    table->index_msg = 0;
    table->index_fid = 0;

    table->UserID = 0;

    memset(&table->UserAddr, 0, sizeof(table->UserAddr));

    for(int i = 0; i < SIZE_LID; i++)
        table->LeaderID[i] = 0;

    for(int i = 0; i < SIZE_FID; i++)
        table->FollowerID[i] = 0;

    for(int i = 0; i < SIZE_MSG; i++)
        table->message[i] = malloc(sizeof(char) * 140);
    
}

void table_setAddr(ServerTable* table,struct sockaddr_in addr)
{
    table->UserAddr = addr;
}

void table_setUid(ServerTable* table, int id)
{
    table->UserID = id;
}

void table_addLid(ServerTable* table, int id)
{
    //boundary check
    if(table->index_lid > SIZE_LID)
    {
        printf("No more Following(50 limits)!\n");
        return;
    }
    
    bool isFound = false;
    
    for(int i = 0; i < table->index_lid;i++)
        if(table->LeaderID[i] == id)
        {
            isFound = true;
            break;
        }
    
    if(!isFound)
        table->LeaderID[table->index_lid++] = id;
    else
        printf("Add Leader Failed!Duplicated Leader!\n");
    
}

void table_addFid(ServerTable* table, int id)
{
    if(table->index_fid > SIZE_FID)
    {
        printf("No more Followers(50 limits)!\n");
        return;
    }
    
    bool isFound = false;
    
    for(int i = 0; i < table->index_fid;i++)
        if(table->FollowerID[i] == id)
        {
            isFound = true;
            break;
        }
    
    if(!isFound)
        table->FollowerID[table->index_fid++] = id;
    else
        printf("Add Follower Fail! Duplicated Follower!\n");
}

void table_addMsg(ServerTable* table, char* msg)
{
    //boundary check
    if(table->index_msg > SIZE_MSG)
    {
        printf("No more Message(50 limits)!\n");
        return;
    }
    
    //vector->data = malloc(sizeof(int) * vector->capacity);
    
    table->message[table->index_msg] = malloc(sizeof(msg));
    
    
    strcpy(table->message[table->index_msg++],msg);
}

bool table_deleteLid(ServerTable* table, int id)
{
    bool isFound = false;
    
    int targetID = 0;
    
    for(int i = 0; i < table->index_lid; i++)
    {
        if(table->LeaderID[i] == id)
        {
            isFound = true;
            targetID = i;
            break;
        }
    }
    
    //skip target and copy original ID array
    if(isFound)
    {
        for(int i = targetID + 1; i < table->index_lid; i++)
            table->LeaderID[i-1] =table->LeaderID[i];
        
        table->LeaderID[table->index_lid - 1] = -1;
        table->index_lid--;
    }
    else
        printf("Didn't find the Leader ID!\n");
    
    return isFound;
    
}

void table_deleteFid(ServerTable* table, int id)
{
    bool isFound = false;
    int targetID = -1;
    
    for(int i = 0; i < table->index_fid; i++)
        if(table->FollowerID[i] == id)
        {
            isFound = true;
            targetID = i;
            break;
        }
    
    if(isFound)
    {
        for(int i = targetID + 1; i < table->index_fid; i++)
            table->FollowerID[i-1] = table->FollowerID[i];
        
        table->FollowerID[table->index_fid - 1] = -1;
        table->index_fid--;
    }
    else
        printf("Didn't find the Follower ID!\n" );
}

bool table_deleteMsg(ServerTable* table,char* msg)
{
    bool isFound = false;
    int i = 0;
    for(i = 0; i < table->index_msg; i++)
        if(strcmp(table->message[i], msg) == 0)
        {
            isFound = true;
            break;
        }
    
    if(isFound)
        table_deleteMsg_i(table, i);
    else
        printf("No such message!\n");
    
    return isFound;
}

void table_deleteMsg_i(ServerTable* table, int index)
{
    //clear i message
//    table.message[index][0] = '\0';
    
    if(index > table->index_msg || table->index_msg == 0)
    {
        printf("No such index of msg!\n");
        return;
    }
    
    for(int i = index + 1; i < table->index_msg; i++)
    {
        memset(table->message[i - 1], 0, strlen(table->message[i - 1]));
        table->message[i - 1] = realloc(table->message[i - 1], sizeof(table->message[i]));
        strcpy(table->message[i - 1], table->message[i]);
    }
    
    memset(table->message[table->index_msg - 1], 0, strlen(table->message[table->index_msg - 1]));
    table->index_msg--;
}

char* table_getMsg_i(ServerTable* table, int index)
{
    if(index > table->index_msg)
    {
        printf("No Message at index %d", index);
    }
    
    return table->message[index];
}

void table_showAllMsg(ServerTable* table)
{
    if(table->index_msg == 0)
    {
        //printf("No messages for this User\n");
        return;
    }
    
    for(int i = 0; i < table->index_msg; i++)
        printf("No. %d message: %s\n", i, table->message[i]);
}

void table_log(ServerTable* table)
{
    printf("\nUserID: %d\n", table->UserID);
    
    printf("Number of LeaderID is %d:\n",table->index_lid);
    for(int i = 0 ; i < table->index_lid; i++)
        if( i == table->index_lid - 1)
            printf("%d\n", table->LeaderID[i]);
        else
            printf("%d ", table->LeaderID[i]);
    
    printf("Number of Message is %d:\n", table->index_msg);
    table_showAllMsg(table);
    
}

void table_free(ServerTable* table)
{
    free(table);
}

void table_write(ServerTable* table)
{
    FILE *wfp;
    
    char directory[50];
    sprintf(directory, "./table/%d.txt", table->UserID);
    
    wfp = fopen(directory, "w+");
    
    //    File looks like:
    //    UserID
    //    unsigned short sin_family;    /* Internet protocol (AF_INET) */
    //    unsigned short sin_port;       /* Port (16-bits) */
    //    struct in_addr sin_addr;       /* Internet address (32-bits) */
    //    Size of Leader ID
    //    LeaderID1, LeaderID2,...
    //    Size of Follower ID
    //    FollowerID1, FollowerID2,...
    //    Size of Message
    //    Message1, Message2,...
    
    if (wfp == NULL) {
        fprintf(stderr, "Can't open the file!\n");
        exit(1);
    }
    
    fprintf(wfp,"%u\n",table->UserID);

    fprintf(wfp,"%u\n", table->UserAddr.sin_family);
    fprintf(wfp,"%u\n", table->UserAddr.sin_port);
    fprintf(wfp,"%lu\n",table->UserAddr.sin_addr.s_addr);
    
    fprintf(wfp,"%d\n",table->index_lid);
    for(int i = 0; i < table->index_lid; i++)
        fprintf(wfp, "%u ", table->LeaderID[i]);
    fputs("\n",wfp);
    
    fprintf(wfp,"%d\n",table->index_fid);
    for(int i = 0; i < table->index_fid; i++)
        fprintf(wfp,"%u ", table->FollowerID[i]);
    fputs("\n", wfp);
    
    fprintf(wfp,"%d\n",table->index_msg);
    for(int i = 0; i < table->index_msg; i++)
        fprintf(wfp,"%s",table->message[i]);

    fclose(wfp);
}

bool table_read(ServerTable* table, int userID)
{
    table_init(table);
    
    //    File looks like:
    //    UserID
    //    unsigned short sin_family;    /* Internet protocol (AF_INET) */
    //    unsigned short sin_port;       /* Port (16-bits) */
    //    struct in_addr sin_addr;       /* Internet address (32-bits) */
    //    Size of Leader ID
    //    LeaderID1, LeaderID2,...
    //    Size of Follower ID
    //    FollowerID1, FollowerID2,...
    //    Size of Message
    //    Message1, Message2,...
    
    FILE *rfp;
    
    char directory[50];
    sprintf(directory, "./table/%d.txt", userID);
    
    rfp = fopen(directory, "r");
    
    if (rfp == NULL) {
        fprintf(stderr, "No such file!\n");
        return false;
    }
    
    //UserID
    int uid;
    fscanf(rfp, "%u",&uid);
    table->UserID = uid;
    
    //addr
    unsigned short family;
    unsigned short port;
    unsigned long addr;
    
    fscanf(rfp, "%hu", &family);
    fscanf(rfp, "%hu", &port);
    fscanf(rfp, "%lu", &addr);
    
    memset(&table->UserAddr, 0, sizeof(table->UserAddr));    /* Zero out structure */
    table->UserAddr.sin_family = family;                 /* Internet addr family */
    table->UserAddr.sin_port   = port;     /* Server port */
    table->UserAddr.sin_addr.s_addr = addr;  /* Server IP address */

    //read Leader ID
    int size_lid;
    fscanf(rfp,"%d", &size_lid);
    table->index_lid = size_lid;
    for(int i = 0; i <size_lid; i++)
    {
        int lid;
        fscanf(rfp,"%d", &lid);
        table->LeaderID[i] = lid;
    }
    
    //Follower ID
    int size_fid;
    fscanf(rfp,"%d", &size_fid);
    table->index_fid = size_fid;
    for(int i = 0; i < size_fid; i++)
    {
        int fid;
        fscanf(rfp,"%d", &fid);
        table->FollowerID[i] = fid;
    }
    
    //read message
    int size_msg;
    fscanf(rfp, "%d", &size_msg);
    table->index_msg = size_msg;
    for(int i = 0; i <size_msg; i++)
    {
        char msg[140];
        //fscanf(rfp,"%s", msg);
        fscanf(rfp,"\n");
        fgets(msg, sizeof(msg), rfp);
        //memset(table->message[i], 0, sizeof(table->message[i]));
        table->message[i] = malloc(sizeof(msg));
        strcpy(table->message[i], msg);
    }
    
    return true;
    
}

void table_copy(ServerTable* target, ServerTable src)
{
//    typedef struct{
//
//        int index_lid;
//        int index_fid;
//        int index_msg;
//
//        unsigned int UserID;
//        struct sockaddr_in UserAddr;
//        unsigned int LeaderID[SIZE_LID];
//        unsigned int FollowerID[SIZE_FID];
//        char *message[SIZE_MSG];
//
//    }ServerTable;
    
    table_init(target);
    
    target->index_fid = src.index_fid;
    target->index_lid = src.index_lid;
    target->index_msg = src.index_msg;
    target->UserID = src.UserID;
    target->UserAddr = src.UserAddr;
    
    for(int i = 0; i < target->index_lid; i++)
        target->LeaderID[i] = src.LeaderID[i];
    
    for(int i = 0; i < target->index_fid; i++)
        target->FollowerID[i] = src.FollowerID[i];
    
    for(int i = 0; i < target->index_msg; i++)
    {
        target->message[i] = malloc(sizeof(src.message[i]));
        strcpy(target->message[i], src.message[i]);
    }
    
}

bool table_searchTag(ServerTable* table, char* tag, int i_msg)
{
    bool hashFound = false;
    
    for(int i = 0; i < 140; i++)
        if( table->message[i_msg][i] == '#')
        {
            hashFound = true;
            break;
        }
    
    if(!hashFound)
        return false;
    
    int tagLen = strlen(tag);
    
    bool tagFound = false;
    for(int i = 0; i < 140; i++)
    {
        if(table->message[i_msg][i] == tag[0])
        {
            if( i + tagLen > 140)
                break;
            
            bool flag = true;
            
            for(int j = 0; j < tagLen - 1; j++)
                if(tag[j] != table->message[i_msg][i+j])
                {
                    flag = false;
                    //printf("Not Match!%c,%c!\n", tag[j], table->message[i_msg][i+j]);
                }
            
            
            if(flag)
                return true;
        }
    }
    
    return tagFound;
        
        
    
    
}
