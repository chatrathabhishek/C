#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "getip.h"

char* getip(char * hostname)
{
    struct hostent *he;

    char * hostip;
    //int i;

    if((he = gethostbyname(hostname)) == NULL)
    {
        printf("Cannot resolve hostname");
        exit(1);
    }
    hostip = inet_ntoa(*((struct in_addr *)he->h_addr_list[0]));

    return hostip;
}
