#ifndef GLOBAL
#define GLOBAL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

// /login yi 123 127.0.0.1 5050
// /login me 123 127.0.0.1 5050
// /login hi 123 127.0.0.1 5050
// /login helo 123 127.0.0.1 5050
// /login 1 123 127.0.0.1 5050
// /login 2 123 127.0.0.1 5050
// /login 3 123 127.0.0.1 5050
// /login 4 123 127.0.0.1 5050
// /createsession 1  
// /leavesession
// /list
#define DATA_SIZE 1000
#define MAX_FILEDATA_SIZE 400
#define MAX_PACKET_SIZE 4096
#define MAXBUFLEN 1000
#define MAX_NUM_SESSIONS 100
#define MAX_DATA 200

bool firstClient = true;

/*
./server 3030
./deliver
/login emily haung 192.168.0.33 3030
/list
/createsession haha
/list

*/

#endif