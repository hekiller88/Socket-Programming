#include "UDPEchoServer.h"



int sock;                        /* Socket */
struct sockaddr_in echoServAddr; /* Local address */
struct sockaddr_in echoClntAddr; /* Client address */
unsigned int cliAddrLen;         /* Length of incoming message */
unsigned short echoServPort;     /* Server port */
int recvMsgSize;                 /* Size of received message */

ServerMessage servMsg;
ClientMessage cliMsg;

void sendMsg();
void recvMsg();


int main(int argc, char *argv[])
{
    cntTable = 0;
    for(int i = 0; i < 50; i++)
        table_init(&allTable[i]);
    
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    
    echoServPort = atoi(argv[1]);  /* First arg:  local port */
    
    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */
    
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
    
    for (;;) /* Run forever */
    {
        recvMsg();
        
        ServerTable tmpTable;
        bool flag = table_read(&tmpTable, cliMsg.userID);   //init table and find if there's a existing file
        int tmpID = allTable_get_i_UserID(cliMsg.userID);   // - 1 not in table, others in table
        
        if(tmpID == -1)
        {
            if(flag)        //has file and not in allTable
            {
                //add File table into current table arr
                table_setAddr(&tmpTable, echoClntAddr);
                allTable_add(tmpTable);
                tmpID = cntTable;
            }
            else            //no file and not in allTable
            {
                //creat the file, add into table arr
                printf("Creating File...\n");
                table_setUid(&tmpTable, cliMsg.userID);
                table_setAddr(&tmpTable, echoClntAddr);
                allTable_add(tmpTable);
                tmpID = cntTable;
            }
        }
        else
        {
            if(flag)    //has file and in allTable
            {  //update current table in allTable
                table_setAddr(&tmpTable, echoClntAddr);
                allTable_add(tmpTable);
            }
            else        //no file and in allTable, CANNOT happen
            {   }
        }
        
        //initialize the msg which will go back to client
        memset(&servMsg, 0, sizeof(servMsg));
        
        if(cliMsg.request_Type == Login)
        {
            printf("ID:%d Login\n", cliMsg.userID);
            server_answer_Login(&allTable[tmpID], &servMsg);
            sendMsg();
        }
        else if(cliMsg.request_Type == Follow)
        {
            server_answer_Follow(&allTable[tmpID], &servMsg, cliMsg.LeaderID);
            sendMsg();
        }
        else if(cliMsg.request_Type == Unfollow)
        {
            if(cliMsg.LeaderID == 0)
                for(int i = 0; i < allTable[tmpID].index_lid; i++)
                {
                    servMsg.LeaderID = allTable[tmpID].LeaderID[i];
                    
                    char str[50];
                    sprintf(str, "%d", servMsg.LeaderID);
                    strcpy(servMsg.message, str);
                
                    sendMsg();
                }
            else{
                server_answer_Unfollow(&allTable[tmpID], &servMsg, cliMsg.LeaderID);
                sendMsg();
            }
            
        }
        else if(cliMsg.request_Type == Post)
        {
            server_answer_Post(&allTable[tmpID], &servMsg, cliMsg.message);
            sendMsg();
            
            //begin to send post to follower
            if(allTable[tmpID].index_fid > 0)   //has follower
            {
                for(int i = 0; i < allTable[tmpID].index_fid; i++)
                {
                    ServerMessage tmpServMsg;
                    memset(&tmpServMsg, 0, sizeof(ServerMessage));
                    
                    int online_i = allTable_get_i_UserID(allTable[tmpID].FollowerID[i]);
                    
                    //printf("Send Post: Try Follower %d\n",allTable[tmpID].FollowerID[i]);
                    
                    if(online_i != -1)      //allTable has such follower, means he is online
                    {
                        //printf("...Send Post OK: Follower %d\n",allTable[online_i].UserID);
                        //new address
                        struct sockaddr_in followerAddr;
                        memset(&followerAddr, 0, sizeof(followerAddr));
                        followerAddr = allTable[online_i].UserAddr;
                        
                        for(int j = 0; j < allTable[tmpID].index_msg; j++)
                        {
                            strcpy(tmpServMsg.message, allTable[tmpID].message[j]);
                            
                            //printf("...Send Post 1: Begin to send Msg!\n");
                            if (sendto(sock, (struct ServerMessage *) &tmpServMsg, sizeof(tmpServMsg), 0,
                                       (struct sockaddr *) &followerAddr, sizeof(followerAddr)) != sizeof(tmpServMsg))
                                DieWithError("sendto() sent a different number of bytes than expected");
                            //printf("...Send Post 2: End of send Msg!\n");
                        }
                    }
                    else
                        printf("Send Post Failed! Follower %d is not online!\n", allTable[tmpID].FollowerID[i]);
                }
                
                
            }
        }
        else if(cliMsg.request_Type == Delete)
        {
            printf("Delete Msg: %s\n", cliMsg.message);
            
            if(cliMsg.message[0] == '\0')   //send all message back
                for(int i = 0; i < allTable[tmpID].index_msg; i++)
                {
                    ServerMessage tmpServMsg;
                    char* str = allTable[tmpID].message[i];
                    
                    strcpy(tmpServMsg.message, str);
                    
                    //printf("LID: %d, msg:%s", tmpServMsg.LeaderID,tmpServMsg.message);
                    
                    if (sendto(sock, (struct ServerMessage *) &tmpServMsg, sizeof(tmpServMsg), 0,
                               (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(tmpServMsg))
                        DieWithError("sendto() sent a different number of bytes than expected");
                }
            else                    //delete the specified msg
            {
                bool flag = table_deleteMsg(&allTable[tmpID], cliMsg.message);
            
                char str[140];
                if(flag)
                    sprintf(str, "Server Delete msg: %s", cliMsg.message);
                else
                    sprintf(str, "Delete Failed! No such Message Found!\n");
                strcpy(servMsg.message, str);
                sendMsg();
            }
                
        }
        else if(cliMsg.request_Type == Search)
        {
            
            for(int i = 0; i < cntTable; i++)
            {
                //printf("Search online user %d!\n", allTable[i].UserID);
                if(allTable[i].index_msg == 0)
                    continue;
                
                for(int j = 0; j < allTable[i].index_msg; j++)
                {
                    bool tagFound = table_searchTag(&allTable[i], cliMsg.message, j);
                    
                    if(tagFound)
                    {
                        //printf("Tag Found!\n");
                        memset(&servMsg, 0, sizeof(servMsg));
                        strcpy(servMsg.message, allTable[i].message[j]);
                        sendMsg();
                    }
                }
            }
        }
        else if(cliMsg.request_Type == Logout)
        {
            int delete_i = allTable_get_i_UserID(cliMsg.userID);
            
            if( delete_i != -1)
            {
                printf("User %d Logout!\n", cliMsg.userID);
                allTable_delete(&allTable[delete_i]);
            }
        }
        
        table_write(&allTable[tmpID]);
    }
    /* NOT REACHED */
}

void allTable_add(ServerTable table)
{
    int index = allTable_get_i_UserID(table.UserID);
    
    if(index == -1)
    {
        table_copy(&allTable[cntTable++], table);
    }
    else
    {
        printf("Already exist in online form! Update!\n");
        table_copy(&allTable[index], table);
        
    }
}

int allTable_get_i_UserID(int uid)
{
    bool isFound = false;
    
    int i = 0;
    for(i = 0; i < cntTable; i++)
        if(allTable[i].UserID == uid)
        {
            isFound = true;
            break;
        }
    
    if(isFound)
        return i;
    else
    {
//        printf("No such table by UserID %d!\n", uid);
        return -1;
    }
}

void allTable_delete(ServerTable *table)
{
    bool isFound = false;
    
    int i = 0;
    
    for(i = 0; i < cntTable; i++)
        if(allTable[i].UserID == table->UserID)
        {
            isFound = true;
            break;
        }
    
    if(isFound)
    {
        for(int j = i+1; j < cntTable; j++)
            allTable[j-1] = allTable[j];
        
        table_init(&allTable[cntTable]);
        cntTable--;
    }
    else
        printf("No such table!\n");
}

void serverMessage_init(ServerMessage* msg)
{
    msg->LeaderID = 0;
    
    for(int i = 0; i < 140; i++) msg->message[i] = '\0';
}

void server_answer_Login(ServerTable* table, ServerMessage* servMsg)
{
    
    strcpy(servMsg->message, "Server accept your Login");
}

void server_answer_Follow(ServerTable* table, ServerMessage* servMsg, int lid)
{
    ServerTable tmp;
    table_init(&tmp);
    bool hasFile = table_read(&tmp, lid);
    
    if(hasFile)
    {
        //add Leader
        table_addLid(table, lid);
        
        //add Follower in Leader's table
        table_addFid(&tmp, table->UserID);
        table_write(&tmp);
        
        servMsg->LeaderID = lid;
        char str[140];
        sprintf(str, "Server record LeaderID %d", lid);
        strcpy(servMsg->message, str);
    }
    else    //no such user login before
    {
        char str[140];
        sprintf(str, "Follow Failed!No such UserID %d", lid);
        strcpy(servMsg->message, str);
    }
}

void server_answer_Unfollow(ServerTable* table, ServerMessage* servMsg, int lid)
{
    bool flag = table_deleteLid(table, lid);
    
    ServerTable tmpLeader;
    table_init(&tmpLeader);
    bool hasFile = table_read(&tmpLeader, lid);
    
    if(flag)
    {
        if(hasFile)
        {
            servMsg->LeaderID = lid;
            char str[140];
            sprintf(str, "Server Delete LeaderID %d", lid);
            strcpy(servMsg->message, str);
            
            table_deleteFid(&tmpLeader, table->UserID);
            table_write(&tmpLeader);
        }
        else
            printf("Unfollowed Failed!No file of Leader!\n");
        
    }
    else
    {
        strcpy(servMsg->message, "Didn't find LeaderID");
    }
    
}

void server_answer_Post(ServerTable* table, ServerMessage* servMsg, char* msg)
{
    table_addMsg(table, msg);
    
    char str[140];
    sprintf(str, "Server record msg: %s", msg);
    strcpy(servMsg->message, str);
    
}

void sendMsg()
{
    /* Send received datagram back to the client */
    if (sendto(sock, (struct ServerMessage *) &servMsg, sizeof(servMsg), 0,
               (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(servMsg))
        DieWithError("sendto() sent a different number of bytes than expected");
}

void recvMsg()
{
    /* Set the size of the in-out parameter */
    
    memset(&cliMsg, 0, sizeof(cliMsg));
    cliAddrLen = sizeof(echoClntAddr);
    
    /* Block until receive message from a client */
    if ((recvMsgSize = recvfrom(sock, (struct ClientMessage *) &cliMsg, sizeof(cliMsg), 0,
                                (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
        DieWithError("recvfrom() failed");
    
    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
    printf("Request Type: %d\n", (int)cliMsg.request_Type );
}

