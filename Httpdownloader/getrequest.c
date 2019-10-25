#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getreq.h"

char * getrequest(char * hostname, char * path)
{
    char * buffer = malloc(1000);

    snprintf(buffer, 1000*sizeof(buffer), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,;q=0.8\r\nUser-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.91 Safari/537.36\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: en-US,en;q=0.8\r\n\r\n",path,hostname);

    return buffer;

}
