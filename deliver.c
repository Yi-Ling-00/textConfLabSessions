#include "clientSession.h"
#include "packet.h"
  
int main(int argc, char *argv[]){
    
    int sockfd;
    struct addrinfo servAddr;
    struct addrinfo *servAddrPtr;
    bool promptForLogin = true;
    struct message packetToSend;
    struct message * ptrToPacketToSend;
    
    //newly added vars
    char inputStr[1000]={'\0'}; 

 
    //expecting ./deliver <server addr> to run the program
    if (argc != 1){
        printf("Error: the number of inputs is incorrect!\n Expecting ./client\n");
        exit(EXIT_FAILURE);
    }

    while(true){
        printf("\n\n****************************\n");
        printf("Enter your prompts below: \n");
        char input[1000],singleWordCommands[1000];
        char fwOfCommand[15], excess[1000];
        enum clientCommands fwEnum;
        fgets (input, 1000, stdin);
        printf("input: %s\n", input);

        //send the message directly to server.
        //take out the first word of input to decide what to do with it
         
        sscanf(input,"/%s %s", fwOfCommand, excess);
        printf("Command: %s\n", fwOfCommand);

        printf("Excess: %s\n", excess);
        if(strcmp(excess, "")==0){
            //no second work, just take input with / removed
            ///memmove(fwOfCommand, fwOfCommand+1, strlen(fwOfCommand));
            fwEnum = convertToEnum(fwOfCommand);
        }else{
            fwEnum = convertToEnum(fwOfCommand); 
        }
         
        printf("enum: %d\n",fwEnum);
 
        
        printf("!\n");
        
        switch(fwEnum){
            //login
            case 0:
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("login!\n");  

                char loginStr[7],clientIDStr[1000],pwStr[1000],serverIP[1000], serverPortStr[1000];
                sscanf(input, "%s %s %s %s %s", loginStr, clientIDStr, pwStr, serverIP, serverPortStr);
                //printf("You enterred: %s %s %s %s %s\n", loginStr, clientIDStr, pwStr, serverIP, serverPortStr);
            
                //check if it's in the right format
                if(strcmp(loginStr, "/login")==0 && atoi(serverPortStr) < 1024){
                    printf("Your login info is in the wrong format. Please re-login!\n");
                    promptForLogin = false;
                    return 0;
                }

                packetToSend = makeLoginPacket(clientIDStr, pwStr);
                ptrToPacketToSend = &packetToSend;

                if(firstClient){
                    memset(&servAddr, 0, sizeof(servAddr));
                    servAddr.ai_family = AF_INET;
                    servAddr.ai_flags = AI_PASSIVE;
                    servAddr.ai_socktype = SOCK_STREAM;

                    int structureSetupSuccess = getaddrinfo(serverIP, serverPortStr, &servAddr, &servAddrPtr);
                    if (structureSetupSuccess != 0){
                        printf("Structure setup failed!\n");
                        printf("structure setup success: %d\n",structureSetupSuccess);
                        exit(EXIT_FAILURE);
                    }

                    printf("creating socket\n");
                    sockfd = socket(servAddrPtr->ai_family, servAddrPtr->ai_socktype, servAddrPtr->ai_protocol);

                    printf("sockfd: %d\n", sockfd); 
                    if (sockfd < 1){
                        printf("Bad socket. Exit");
                        exit(EXIT_FAILURE);
                    }

                
                    if (connect(sockfd, servAddrPtr->ai_addr, servAddrPtr->ai_addrlen) == -1) {
                        close(sockfd);
                        perror("client: connect");
                        exit(EXIT_FAILURE);
                    }
                    firstClient = false;//don't connect to server again
                }


               

                break;
            


            //logout
            case 1: 
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("logout!\n");

                packetToSend = makeLogoutPacket(clientIDStr);
                ptrToPacketToSend = &packetToSend;
                //close socket(not actually opened at this point since no outer while loop)
                break;

            //join session
            case 2: 
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("join session!\n");
                char joinSessionStr[13], sessionID[1000];
                sscanf(input, "%s %s", joinSessionStr, sessionID);
                if(strcmp(joinSessionStr, "/joinsession")==0){
                    printf("Not join session. Please re-enter your prompt\n");
                    promptForLogin = false;
                }

                packetToSend = makeJoinSessPacket(clientIDStr, sessionID);
                ptrToPacketToSend = &packetToSend;

                break;


            //leave session
            case 3:
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("leave session!\n");


                packetToSend = makeLeaveSessPacket(clientIDStr);
                ptrToPacketToSend = &packetToSend;


                break;


            //create session
            case 4:
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("create session!\n");
                char createSessionStr[13],newSessID[1000];
                sscanf(input, "%s %s", createSessionStr, newSessID);
                printf("new session id: %s\n", newSessID);

                packetToSend = makeCreateSessPacket(clientIDStr,newSessID);
                ptrToPacketToSend = &packetToSend;
                break; 

            //list
            case 5:
                printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
                printf("list!\n");
                packetToSend = makeQueryPacket(clientIDStr);
                ptrToPacketToSend = &packetToSend;
                
                
                break; 

            //quit
            case 6:
                close(sockfd);
                break; 
            default:
                printf("Invalid command\n");
                break;
        }
        
        printf("\nWill send packet: \n");
        printPacket(ptrToPacketToSend);

        int numBytes = write(sockfd, ( void *)ptrToPacketToSend, sizeof(packetToSend));
        //int numBytes = sendto(sockfd, ( void *)ptrToPacketToSend, sizeof(packetToSend), 0, (struct sockaddr *)(servAddrPtr->ai_addr), servAddrPtr->ai_addrlen);
        if (numBytes < 0){
            perror("Client failed sending stringToSend!\n");
            exit(EXIT_FAILURE);
        }else{
            printf("Client sent successfully!\n");
        }


        //clear for next round
        memset(input, 0, sizeof input);
        //clear for next round
        memset(fwOfCommand, 0, sizeof fwOfCommand);
        memset(excess, 0, sizeof excess);

        printf("\nReceiving from server: \n");
        struct message * serverPacket = (struct message * ) malloc (sizeof (struct message));
        read(sockfd, ( void *)serverPacket, sizeof(struct message));
        printPacket(serverPacket);
        printf("\n");
    }
   


    close(sockfd);
    return 0;      
}