#ifndef CLIENTLIST
#define CLIENTLIST

#include "global.h"

typedef struct clientStruct{
    int clientFD;
    unsigned char clientID[MAXBUFLEN];
    bool inSess;
    bool loggedIn;
    unsigned char sessionID[MAXBUFLEN];
    int sessionElementsOccupied;
    struct clientStruct * next;
}User;

typedef struct session{
    char sessionID[MAXBUFLEN];
    User * clientsInSession;
    struct session *next;
}Session;

//global client and session lists
User * clientList = NULL; //head
User * lastClient = NULL;

Session *sessionList=NULL;
Session *lastSession=NULL;

void printClientStruct(User *currentClient){
    printf("clientFD: %d\n", currentClient -> clientFD);
    printf("clientID: %s\n", currentClient -> clientID);
    printf("logged in: %d\n", currentClient -> loggedIn);
    printf("in sess: %d\n", currentClient -> inSess);
    return;
}   

void printClientList(User * n){
    if(n == NULL){
        printf("No user(s) to list. Nothing is connected.\n");
        return;
    }
    while(n != NULL){
        printClientStruct(n);
        if(n -> next != NULL)n = n -> next;
        else break;
    }
    return;
}

void printSessionStruct(Session *currentSession){
    printf("sessionID: %s\n", currentSession -> sessionID);
    printf("users in this section: \n");
    printClientList(currentSession -> clientsInSession);
    return;
}   

void printSessionList(Session * n){
    if (n==NULL){
        printf("No active session!\n");
        return;
    }
    while(n!=NULL){
        printSessionStruct(n);
        if(n -> next != NULL)n = n -> next;
        else break;
    }
}


void listCommand(){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    //print everything in the client linked list
    printf("These are the clients connected: \n");
    printClientList(clientList);
    
    printf("\nThese are the sessions available: \n");
    printSessionList(sessionList);
    return;
}


//check if client list has someone with the same userID
bool checkClientID(unsigned char * clientID){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);

    User * temp = clientList;
    while(temp != NULL){
        printf("In while in checkClientID\n");
        if(memcmp(temp -> clientID,clientID, MAXBUFLEN)==0)return true;
        if(temp -> next != NULL)temp = temp -> next;
    }
    return false;
}


bool addToClientList(User *currentClient){
    //if user has already logged in: return false

    if(currentClient -> loggedIn == true)return false;
    if(checkClientID(currentClient -> clientID)== true)return false;

    //else add to list and return true
    printf("current User: %s\n", currentClient->clientID);
    User * temp = clientList;
    
    if(clientList==NULL){
        printf("First client in the list!\n");
        clientList = currentClient;
        lastClient = currentClient;
        currentClient -> next = NULL;
    }else{
        lastClient -> next = currentClient;
        lastClient = currentClient;
        currentClient -> next = NULL;
    }
    return true;
    
}

void removeFromClientList(unsigned char clientID[], int acceptFD){
    printf("Removing from client list!\n");
    struct clientStruct * current = clientList;
    struct clientStruct * prev = NULL;

    if (current != NULL && memcpy(current -> clientID,clientID,MAXBUFLEN)==0){
        clientList = current -> next;
        free(current);
        return;
    }

    while(current != NULL && memcpy(current -> clientID,clientID, MAXBUFLEN)!=0){
        prev = current;
        current = current -> next;
    }

    if(current == NULL) return;//never found
    prev -> next = current -> next;
    free(current);
    return;
}






bool joinSession(char sessID[], User *currentClient, char * reasonForFailure){  
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    printClientStruct(currentClient);
    //1.check if session exists
    Session * currSess = sessionList;
    bool validSessID=false;
    while(currSess!= NULL){
        //if not found, continue to check
        if(strcmp(currSess->sessionID,sessID)!=0)currSess = currSess -> next;
        else{
            //check if client already in this session. If so, return true with the appropriate message
            validSessID = true;
            break;
        }
    }
    
    printf("Done checking if session exists!\n");

    //session doesn't exist || not logged in || already in another session -> can't join
    if (validSessID==false || currSess == NULL){
        char whyFailed[] = "Can't join session: session doesn't exist!\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }
    printf("Done checking condition 1\n");
    if(currentClient->loggedIn==false){
        char whyFailed[] = "Can't join session: you are not logged in!\n";
        strcpy(reasonForFailure, whyFailed);
        return false;
    }
    printf("Done checking condition 2\n");
    if(currentClient->inSess==true){
           printf("InSess == true!\n");
        char whyFailed[] = "Can't join session: you are already in a session!\n";
        strcpy(reasonForFailure, whyFailed);
         printf("reason failed: %s\n", reasonForFailure);
        return false; 
    }

    printf("About to look for client list\n");

    //currentSession now points to the session the user wants to join
    //add to session's clients
    User *temp = currSess->clientsInSession; //nested client Linkedlist in Sessionlist
    if(temp == NULL){
        printf("no clients in sess\n");
        //directly add this node
        temp = currentClient;
        return true;
    }else{
        while(temp->next!=NULL){
            printf("looking for last client in sess\n");
            temp = temp->next;
        }  
        temp->next = currentClient;
        return true;
    }
}



bool createSession(unsigned char sessID[], User *currentClient){
    printf("This is %s() from %s, line %d\n",__FUNCTION__, __FILE__, __LINE__);
    if(currentClient->loggedIn==false || currentClient->inSess==true ) return false; //User not logged in or already in another sess

    Session *curr = sessionList;
    bool validSessID =false;
    Session *temp= calloc(sizeof(Session), 1);
    memcpy(temp -> sessionID,sessID, MAXBUFLEN);
    temp -> clientsInSession= currentClient;
    temp -> next = NULL;

    if (sessionList==NULL){
        printf("First session in the list\n");
        sessionList = temp;
        lastSession = temp;
    }else{
        lastSession -> next = temp;
        lastSession = temp;      
    }
     
    //printSession(temp);
    return true;
}



void leaveSession(User *currentClient){
    printf("In leave session function!\n");

    if (currentClient->inSess == false) {
        printf("Client never in session!\n");
        return;
    }

    Session *curr= sessionList;
    while(curr!=NULL){

        if(memcmp(curr->sessionID, currentClient->sessionID, MAXBUFLEN)==0){
            User *temp = curr->clientsInSession; 
            User *prev;

             while(temp->next!=NULL){
              prev= temp;
                if(memcmp(currentClient->clientID, temp->clientID, MAXBUFLEN)==0){
                    free(temp);
                    break;
                }
                temp= temp->next;
             }//end of inner while loop  

             temp->next= currentClient;
             break;
        }

        curr= curr->next;
    }//end of outer while loop

}


#endif
