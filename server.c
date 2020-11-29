#include "clientSession.h"
#include "packet.h" 

#define BACKLOG 20
bool message=false;

struct message processPacket(struct message incomingPacket, User *current){
    printf("in process packet!\n");
    struct message packetToSend;
    
    //process the packet 
    //1. look at the packet type
    switch (incomingPacket.type){
        //follow the types according to the enumeration
        case 0: //login: store the clientID in the client list
            printf("Its a login!\n");
            //struct clientStruct * currentClient;
            //verify if client already logged in or already existing
            bool addSuccess=false;
            //strcpy(current->clientID, (unsigned char *) incomingPacket.source);
            addSuccess = addToClientList(current);
            printf("^^^^^Add success : %d\n",addSuccess);
            if (addSuccess){
                printf("Login sucess!\n");
                current->loggedIn = true;
                //do LO_ACK
                packetToSend = makeLoAckPacket(current);
            }else{
                printf("Login fail!\n");
                current->loggedIn = false;
                //do LO_NAK
                char reasonFailed[] = "This user is already logged in";
                packetToSend = makeLoNakPacket(current, reasonFailed);
            }
            break;

        case 1: //Logout: remove the clientID in the client list
            printf("It's a logout!\n");
            removeFromClientList((unsigned char *)incomingPacket.source, current->clientFD);
            break;
        case 4: //Joinsession
            printf("Joining session!\n");
            //joining
            bool joinSuccess=false;
            char * reasonForFailure = malloc(1000 * sizeof(char));
            joinSuccess = joinSession((char *)incomingPacket.data, current, reasonForFailure); 
            printf("\n&&&&   IN SERVER: %s\n", reasonForFailure);
            if (joinSuccess) {
                printf("Sucessfuly joined session!\n");
                //create Jn_ack packet
                current -> inSess = true;
                memcpy(current -> sessionID, incomingPacket.data,MAXBUFLEN);
                packetToSend = makeJnAckPacket(current, incomingPacket.data);
            }else{
                printf("Can't join session !\n");
                 //make jn_nak packet
                current -> inSess = false;
                packetToSend = makeJnNakPacket(current, incomingPacket.data,reasonForFailure);
            }
            free(reasonForFailure);
            break;
        case 7: //leavesession
            printf("Leaving session!\n");
            leaveSession(current);
            packetToSend = makeLeaveAckPacket(current);
            break;
        case 8: //create session
            printf("creating session!\n");
            bool createSuccess=false;
            createSuccess = createSession((unsigned char *)incomingPacket.data, current); 
             if (createSuccess) {
                current->inSess=true;
                printf("Sucessfuly Created session!\n");
                //create Jn_ack packet
                packetToSend = makeNsAckPacket(current);
            }
            else{
                current->inSess=false;
                printf("Unsucessful!\n");
                 //make jn_nak packet
                packetToSend = makeNsNakPacket(current);
            }
            break;
        case 11: //list
            printf("List!\n");
            packetToSend = makeQuAckPacket(current->clientID,current->sessionID);
            break;
        default:  
            break;
    }
    return packetToSend;
}


int main(int argc, char *argv[]){
    char *portNum;
    struct addrinfo servAddr;
    struct addrinfo* servAddrPtr;
    struct sockaddr_storage cliAddr;  
    int sockfd;

    //expecting server <port num>
    if(argc != 2){
        printf("Error: the number of inputs is incorrect!\n");
        exit(EXIT_FAILURE);
    }

//===============Setup===============
    //make the socket
    portNum = argv[1];
    printf("%s\n", portNum);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.ai_family = AF_INET;       //IPv4
    servAddr.ai_socktype = SOCK_STREAM;
    servAddr.ai_flags = AI_PASSIVE;

    //allocate linked list(fills the structs needed later) using getaddrinfo
    int structureSetupSuccess = getaddrinfo(NULL, portNum, &servAddr, &servAddrPtr);
 
    if(structureSetupSuccess < 0 ) { 
        printf("Structure setup failed!\n"); 
        exit(EXIT_FAILURE);
    }

    //create unbound socket in domain
    //servAddrPtr->ai_family: which is IPv4
    //servAddrPtr->ai_socktype is datagram for UDP
    //protocol is 0: default protocol
    //sockfd: used to access the socket later
    sockfd = socket(servAddrPtr -> ai_family, servAddrPtr->ai_socktype, servAddrPtr->ai_protocol);
    printf("MY SOCKETFD: %d\n", sockfd);
    if(sockfd < 0){
        printf("Socket failed!\n");
        exit(EXIT_FAILURE);
    } 

    
    //bind socket made
    //assign address to unbound socket just made, unbound socket CAN'T receive anything
    //assigns the address in the 2nd parameter(servAddrPtr -> ai_addr) to the socket in the 1st parameter 
    //which is the socket's file descriptor
    //3rd parameter specifies the size, in bytes, of the address structure pointed to by addr
    int bindRet = bind(sockfd, (struct sockaddr * )servAddrPtr -> ai_addr,servAddrPtr -> ai_addrlen);
    printf("bind: %d\n", bindRet);
    if(bindRet < 0){
        printf("Bind failed!\n");
        exit(EXIT_FAILURE); 
    } 

    //listen for connections
    if(listen(sockfd, BACKLOG) == -1){
        perror("Listen\n");
        exit(1);
    } 

//=============Clients coming=============
    firstClient = true;
    bool logout = false;
    int acceptFD;
    socklen_t addrLen = sizeof(cliAddr);  
    User *newUser = calloc(sizeof(User), 1);

    newUser->clientFD = accept(sockfd, (struct sockaddr *)&(cliAddr), &addrLen);
    if(newUser->clientFD < 0){
        perror("newUser->clientFD\n");  
        exit(1); 
    }
    newUser ->inSess = false;
    newUser -> next = NULL;
      

    struct message * incomingPacket;
    struct message packetToSend;
    struct message * ptrToPacketToSend;
    int recvBytes;
    while(1){
        printf("\n\n****************************\n");
        incomingPacket = (struct message * ) malloc (sizeof (struct message));
        printf("Before read:\n");
        printf("reading from: %d\n",newUser->clientFD);
        printPacket(incomingPacket);
        recvBytes = read(newUser->clientFD, incomingPacket, sizeof(struct message));
        printf("After read:\n");

        printf("bytes received from client: %d\n",recvBytes);

        if(recvBytes < 0){
            perror("Error receiving!\n");
            exit(EXIT_FAILURE);
        }

        printf("\nReceiving packet: \n");
        printPacket(incomingPacket);

        //before processing packet, fill in user info as much as possible
        memcpy(newUser -> clientID,incomingPacket -> source,MAXBUFLEN);
        
        packetToSend = processPacket(*incomingPacket,newUser);
        printf("\n###Server's gonna send:\n");
        printPacket(&packetToSend);

        ptrToPacketToSend = &packetToSend;
        int sendBytes = write(newUser->clientFD, ptrToPacketToSend, sizeof(struct message));

        //clean up for next round
        free(incomingPacket);
        incomingPacket = NULL;
    
        firstClient = false;
    }//end of while loop
    close(sockfd);
    return (EXIT_SUCCESS);
}