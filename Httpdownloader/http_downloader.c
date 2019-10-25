#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "getip.h"
#include "getreq.h"
#include <curl/curl.h>

pthread_mutex_t lock;

struct dwnloader{

    size_t len;
    int cont_len;
    char* url;
    char* range;
    int range_lower;
    int range_div;
    int t;
    char* cmd;


};

struct dwnloader dl;

int createSock(){
    return socket(AF_INET, SOCK_STREAM,0);
}



int filedownload(struct dwnloader dl)
{

    CURL *curl;
    FILE *fp;
    CURLcode res;

    char * outfile = malloc(FILENAME_MAX);
    strcpy(outfile,"Part_");
    char filenum[20];
    snprintf(filenum, sizeof(filenum),"%d",dl.t);
    outfile = strcat(outfile,filenum);


    curl = curl_easy_init();
    if(curl)
    {
        fp = fopen(outfile,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, dl.url);
        curl_easy_setopt(curl, CURLOPT_RANGE, dl.range);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,fp);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }


    if(res != CURLE_OK)
    {
        printf("Download failed for %s", outfile);
        exit(0);
    }



    //return 0;


}


void *mythreadfunc(void *param)
{
    pthread_mutex_lock(&lock);
    snprintf(dl.range, 100000000*sizeof(dl.range),"%d-%d", dl.range_lower, dl.range_div-1);
    filedownload(dl);
    dl.range_lower = dl.range_lower+dl.range_div;
    dl.range_div = dl.range_div+dl.range_div;
    dl.t++;
    free(dl.range);
    dl.range = malloc(100000000);
    snprintf(dl.range, 100000000*sizeof(dl.range),"%d-%d", dl.range_lower, dl.range_div-1);
    pthread_mutex_unlock(&lock);

    return 0;
}


int main(int argc , char *argv[])
{
    int socket_des;
    socket_des = createSock();

    if(socket_des == -1){
        printf("Could not create a socket");
    }


    dl.len = strlen(argv[1]);
    dl.url = malloc(dl.len+1);
    strcpy(dl.url,argv[1]);

    //Split Url into Hostname and Path


    char http[] = "http";
    char *check = NULL;
    char *hostname;
    char *path = malloc(dl.len+1);

    char *token = strtok(dl.url, "/");

    check = strstr(dl.url,http);
    if (check == dl.url)
    {
        token = strtok(NULL,"/");
        hostname = token;
        token = strtok(NULL,"/");
        while(token != NULL)
        {
            strcat(path,"/");
            strcat(path, token);
            token = strtok(NULL, "/");
        }
    }
    else
    {
        hostname =token;
        token = strtok(NULL,"/");
        while(token != NULL)
        {
            strcat(path,"/");
            strcat(path, token);
            token = strtok(NULL, "/");
        }
    }

    char ip[100];
    strcpy(ip,getip(hostname));
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip);
    printf("hostname is %s\n", hostname);
    printf("path is %s\n", path);
    printf("ip is %s\n", ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    //Connect to Server
    if (connect(socket_des , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		exit(1);
	}
	else
	{
        puts("Connected");
    }

    char * req = getrequest(hostname, path);

    printf("%s",req);

    if(send(socket_des,req,strlen(req),0)<0)
    {
        printf("error");
        exit(1);
    }
    else printf("Request sent, successfully\n");

    char buffer[2000];
    int cont_len;
    if(recv(socket_des, buffer, sizeof(buffer), 0) == -1)
    {
        perror("recv");
        printf("error recieving\n");
        exit(1);
    }
    else
    {
        //Extract content length
        char * head = strstr(buffer,"Content-Length");
        char * token2 = strtok(head," ");
        token2 = strtok(NULL," ");
        char * content_len = token2;
        dl.cont_len = atoi(content_len);
        printf("Content Length : %d\n",dl.cont_len);

    }


    //rebuild URL
    strcpy(dl.url,hostname);
    dl.url = strcat(dl.url,path);
    int nthreads = atoi(argv[2]);
    pthread_t thread[nthreads];
    int thread_args[nthreads];

    if(dl.cont_len%nthreads == 0)
    {
        dl.range_div = dl.cont_len/nthreads;
    }
    else dl.range_div = dl.cont_len/nthreads +1;
    dl.range_lower = 0;
    dl.t = 0;
    dl.range = malloc(10000000);


    int rc, i;


    //Create multiple threads for download


    pthread_mutex_init(&lock, NULL);


    for (i=0; i<nthreads;i++)
    {
        rc = pthread_create(&thread[i], NULL, mythreadfunc, (void *) &thread_args[i]);
    }


    // Wait for downloads to finish
    for (i=0;i<nthreads;i++)
        rc = pthread_join(thread[i], NULL);

    pthread_mutex_destroy(&lock);


    //Extract filename
    char finalfile[100];
    char * token3 = strtok(path,"/");
    while(token3 !=NULL)
    {
        strcpy(finalfile,token3);
        token3 = strtok(NULL,"/");
    }

    printf("%s\n", finalfile);

    char *filenames = malloc(1000000);
    dl.cmd = malloc(1000000);
    char* fn = malloc(20);
    int x;

    for(x=0; x<nthreads;x++)
    {
        sprintf(fn,"Part_%d",x);
        strcat(filenames, fn);
        strcat(filenames, " ");
    }

    //Combining the files
    sprintf(dl.cmd, "cat %s > %s", filenames, finalfile);
    system(dl.cmd);


    printf("I am finished\n\n");
    free(dl.url);
    free(path);
    free(filenames);
    free(dl.cmd);
    free(fn);

    return 0;
}
