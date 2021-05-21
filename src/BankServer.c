#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include<pthread.h>
#include<semaphore.h>
#include<signal.h>
#include "../include/bankserverutilities.h"

//include <netinet/in.h>
//include <arpa/inet.h>

/* Misc constants */
#define	MAXLINE	 8192  /* Max text line length */
#define LISTENQ  1024  /* Second argument to listen() */

socklen_t clientlen;
struct sockaddr_storage clientaddr; /* Enough room for any addr */
char client_hostname[MAXLINE], client_port[MAXLINE];
int count = 0;
pthread_t pt[15];
pthread_mutex_t lock;



void* socketThread(void* arg){

    int connfd = *((int*) arg);
    getnameinfo((struct sockaddr *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
    printf("Connected to %s, %s\n", client_hostname, client_port);
	//printf("Start Communication with Client\n");
    beginCommunication(connfd, &lock);
	printf("End Communication with %s %s\n", client_hostname, client_port);
    close(connfd);
    pthread_exit(0);
}

int main(int argc, char **argv)
{

    signal(SIGINT, signal_handler);
    int listenfd, connfd;
    listenfd = open_listenfd("15000");

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    while (1) {
        printf("Waiting for a new Client to connect\n");
        clientlen = sizeof(struct sockaddr_storage); /* Important! */
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

        if( pthread_create(&pt[count], NULL, socketThread, &connfd) != 0 )
            printf("Failed to create thread\n");
        
        if( count >= 50)
        {
          count = 0;
          while(count < 50)
          {
            pthread_join(pt[count++],NULL);
          }
          count = 0;
        }
    }
    pthread_mutex_destroy(&lock);
    exit(0);
}
