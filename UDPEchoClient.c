#include "UDPEchoClient.h"

void sendMsg();
void recvMsg();

int sock;                        /* Socket descriptor */
struct sockaddr_in echoServAddr; /* Echo server address */
struct sockaddr_in fromAddr;     /* Source address of echo */
unsigned short echoServPort;     /* Echo server port */
unsigned int fromSize;           /* In-out of address size for recvfrom() */

ClientMessage myMsg;
ServerMessage servBackMsg;
unsigned int clientID;

char *servIP;                    /* IP address of server */
char *argu_message;                /* String to send to echo server */
int msgLen;               /* Length of string to echo */
int cliMsgLen;              /* Length of ClientMsg to echo */
int servBackMsgLen;               /* Length of received response */

int main(int argc, char *argv[])
{
    clientMessage_init(&myMsg);
    
    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }
    
    servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    argu_message = argv[2];       /* Second arg: string to echo */
 	
    msgLen = strlen(argu_message);
    if(msgLen > 140)
        DieWithError("Message too long(140 limit)!");
    
    for(int i = 0; i < msgLen; i++) myMsg.message[i] = argv[2][i];
    
    if (argc == 4)
        echoServPort = atoi(argv[3]);  /* Use given port, if any */
    else
        echoServPort = 7;  /* 7 is the well-known port for the echo service */
    
    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */
    
    //force user to Login first
    printf("Login...");
    client_askUserID(&myMsg);
    cliMsgLen = sizeof(myMsg);
    sendMsg();
    recvMsg();
    
    pid_t pid;
    pid = fork();
    
    //child process, using for recieve msg
    if(pid == 0)
    {
        do{
            recvMsg();
        }while(true);

        exit(0);
    }
    
    //parent process, using for send msg only
    while(true)
    {
        client_askRequestType(&myMsg);
        
        if(myMsg.request_Type == Follow)
        {
            client_askFollow(&myMsg);
            
            if(myMsg.UserID != myMsg.LeaderID )
            {
                sendMsg();
                sleep(1);
            }
            else
                printf("Can't follow youself!\n");
        }
        else if(myMsg.request_Type == Post)
        {
            client_askPost(&myMsg);
            sendMsg();
            sleep(1);
            //recvMsg();
        }
        else if(myMsg.request_Type == Delete)
        {
            // show all msg posted
            client_askDelete(&myMsg, 0);
            sendMsg();
            sleep(1);
            
            // choose the msg to delete
            printf("Input the message you want to delete: \n");
            client_askDelete(&myMsg, 1);
            sendMsg();
            
        }
        else if(myMsg.request_Type == Unfollow)
        {
            //show all Leader ID
            client_askUnfollow(&myMsg, 0);
            sendMsg();
            sleep(1);
            
            //choose on ID to unfollow
            client_askUnfollow(&myMsg, 1);
            sendMsg();
            sleep(1);
        }
        else if(myMsg.request_Type == Search)
        {
            printf("Input your tag:\n");
            
            memset(myMsg.message, 0, sizeof(char)*140);
            char tag[140];
            scanf("\n");
            fgets (tag, 140, stdin);
            strcpy(myMsg.message, tag);
            
            sendMsg();
            sleep(1);
        }
        else if(myMsg.request_Type == Logout)
        {
            sendMsg();
            close(sock);
            exit(0);
        }
        else;
    }
    
    close(sock);
    exit(0);
}

void sendMsg()
{
    /* Send the string to the server */
    if (sendto(sock, (struct ClientMessage *) &myMsg, sizeof(myMsg), 0, (struct sockaddr *)
               &echoServAddr, sizeof(echoServAddr)) != cliMsgLen)
        DieWithError("sendto() sent a different number of bytes than expected");
}

void recvMsg()
{
    memset(&servBackMsg, 0, sizeof(servBackMsg));
    
    /* Recv a response */
    if ((servBackMsgLen = recvfrom(sock,  (struct ServerMessage *) &servBackMsg, sizeof(servBackMsg), 0,
                                   (struct sockaddr *) &fromAddr, &fromSize)) != sizeof(servBackMsg))
        DieWithError("recvfrom() failed");
    
//    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
//    {
//        fprintf(stderr,"Error: received a packet from unknown source.\n");
//        exit(1);
//    }
//    printf("\nBack Addr %s\n", inet_ntoa(fromAddr.sin_addr));
    
    /* null-terminate the received data */
    printf("%s", servBackMsg.message);    /* Print the echoed arg */
}

void clientMessage_init(ClientMessage* msg)
{
    memset(msg, 0, sizeof(ClientMessage));
}

void client_askRequestType(ClientMessage* cliMsg)
{
    //there're 7 type of request
    
    int request;
    
    do{
        const int reqLen = Logout - Login + 1 - 1;
        
        //{Login, Follow, Post, Search, Delete, Unfollow, Logout }
        printf("\nPlease choose one of the %d requests:\n", reqLen );
        //printf("0 - Login\n");
        printf("1 - Follow\n");
        printf("2 - Post\n");
        printf("3 - Search\n");
        printf("4 - Delete\n");
        printf("5 - Unfollow\n");
        printf("6 - Logout\n");
        
        scanf("%d", &request);
        
        if(request < 1 || request >6 )
            printf("Invalid Request Type!\n");
        else
            break;
    }while(true);
    
    if(request == 0)
        cliMsg->request_Type = Login;
    else if(request == 1)
        cliMsg->request_Type = Follow;
    else if(request == 2)
        cliMsg->request_Type = Post;
    else if(request == 3)
        cliMsg->request_Type = Search;
    else if(request == 4)
        cliMsg->request_Type = Delete;
    else if(request == 5)
        cliMsg->request_Type = Unfollow;
    else
        cliMsg->request_Type = Logout;
}

void client_askUserID(ClientMessage * cliMsg)
{
    int uid;
    printf("Input your User ID(only digits & bigger than 0): \n");
    scanf("%d",&uid);
    
    clientID = uid;
    
    if(uid <= 0)
    {
        printf("Invalid User ID!\n");
        return;
    }
    
    cliMsg->UserID = uid;
}

void client_askFollow(ClientMessage* cliMsg)
{
    printf("Input the Leader to follow(only digits & no 0): \n");
    int lid;
    scanf("%d",&lid);
    
    if(lid <= 0)
    {
        printf("Invalid Leader ID!\n");
        return;
    }
    
    cliMsg->LeaderID = lid;
}

void log_ClientMessage(ClientMessage msg)
{
    printf("UserID = %d \n", msg.UserID);
    printf("Message = %s", msg.message);
    printf("LeaderID = %d \n", msg.LeaderID);
}

void client_askMessage(ClientMessage* cliMsg)
{
    char str[140];
    printf("Input your message(140-chars limits):\n");
//    scanf ("%[^\n]%*c", str);
//    scanf("%[^\n]s",str);
    scanf("\n");
    fgets (str, 140, stdin);
    //printf("Your message: %s", str);
    
    strcpy(cliMsg->message, str);
}

void client_askPost(ClientMessage* cliMsg)
{
    client_askMessage(cliMsg);
}

void client_askDelete(ClientMessage* cliMsg, int flag)
{
    if(flag == 0)   //show all message
        memset(cliMsg->message, 0, sizeof(char)*140);
    else
    {
        memset(cliMsg->message, 0, sizeof(char)*140);
        char str[140];
        scanf("\n");
        fgets (str, 140, stdin);
        strcpy(cliMsg->message, str);
        //printf("Msg you want: %s", cliMsg->message);
    }
}

void client_askUnfollow(ClientMessage* cliMsg, int flag)
{
    if(flag == 0)
    {
        cliMsg->LeaderID = 0;
    }
    else{
        printf("Input the Leader to Unfollow(only digits & no 0): \n");
        int lid;
        scanf("%d",&lid);
        
        if(lid <= 0)
        {
            printf("Invalid Leader ID!\n");
            return;
        }
        cliMsg->LeaderID = lid;
    }
    
}

