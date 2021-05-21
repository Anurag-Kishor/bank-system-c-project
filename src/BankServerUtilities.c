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

#define	MAXLINE	 8192  /* Max text line length */
#define LISTENQ  1024  /* Second argument to listen() */
pthread_mutex_t lock;

int open_listenfd(char *port) 
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;
	char host[MAXLINE],service[MAXLINE];
    int flags;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address AI_PASSIVE - used on server for TCP passive connection, AI_ADDRCONFIG - to use both IPv4 and IPv6 addresses */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number instead of service name*/
    getaddrinfo(NULL, port, &hints, &listp);

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;  /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,    //line:netp:csapp:setsockopt
                   (const void *)&optval , sizeof(int));

		flags = NI_NUMERICHOST | NI_NUMERICSERV; /* Display address string instead of domain name and port number instead of service name */
		getnameinfo(p->ai_addr, p->ai_addrlen, host, MAXLINE, service, MAXLINE, flags);
        printf("host:%s, service:%s\n", host, service);

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        close(listenfd); /* Bind failed, try the next */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
	return -1;
    }
    return listenfd;
}

void logTransaction(char* acc_no, char* new_balance, char c, float amt){
    char s[1000];
    time_t t = time(NULL);
    struct tm * p = localtime(&t);
    strftime(s, 1000, "%I:%M:%S:%p %A, %B %d %Y", p);

    FILE * fPtr;
    char buffer[BUFSIZ];
    char file_name[100];
    memset(file_name, 0, 100);

    strcpy(file_name, "../db/Transactions/");
    strcat(file_name, acc_no);
    strcat(file_name, ".txt");

    fPtr  = fopen(file_name, "a+");
  //  printf("%s", file_name);
    if(c == 'w'){
        fprintf(fPtr, "%s : Rs. %f withdrawn from %s. New balance is %s.\n", s, amt, acc_no, new_balance);
    }else if( c == 'd'){
        fprintf(fPtr, "%s : Rs. %f deposited to %s. New balance is %s.\n", s, amt, acc_no, new_balance);
    }
    else if( c == 'c'){
        fprintf(fPtr, "%s : New account %s created. Current balance %s\n", s, acc_no, new_balance);
    }
    fclose(fPtr);
}

bool createAccount(char *acc_no, char* name, char* age, char* balance){
   
    pthread_mutex_lock(&lock);
    FILE *fp;
    fp = fopen("../db/Customers.txt", "a");
    if(fp == NULL){
        printf("Could not create account!\n");
        return false;
    }

    printf("%d\n", fprintf(fp, "%s %s %s %s\n", acc_no, name, age, balance));
    fclose(fp);
    logTransaction(acc_no, balance, 'c', 0);
    pthread_mutex_unlock(&lock);

    return true;

}

bool validateAdmin(char* id, char* pass){
    FILE *fp;
    char str[100];
    fp = fopen("../db/AdminCredentials.txt", "r");
    if(fp == NULL){
        perror("Error opening file!\n");
        return false;
    }
    
    int temp = 0, j = 0;
    int flag = 0;
    while(fscanf(fp, "%s", str) != EOF){
        temp++;
        
        if(temp == 1){
            if(strcmp(str, id) == 0){
               
                flag = 1;
            }else{
                
                flag = 0; 
                temp = 0;
            }
        }
        else if(temp == 2 && flag == 1){
            if(strcmp(str, pass) == 0){
                return true;
            }
            flag = 0;
            temp = 0;
        }
    }
    fclose(fp);
    return false;
}

void replaceAll(char *str, char *newWord)
{   
    char tempstr[BUFSIZ];
    int temp = 0, j = 0; 
    memset(tempstr, 0, 100);
    for(int i = 0; i < strlen(str); i++){
       if(str[i] == ' '){
           temp++;
       }
       if(temp == 4){
            tempstr[j++] = ' ';
            strcat(tempstr, newWord);
            break;
        }
        tempstr[j] = str[i];
        j++;
    }
    strcpy(str, tempstr);
    memset(tempstr, 0, 100);
}

bool performTransaction(char *str, char* acc_no, char* newbalance, float amt){

    pthread_mutex_lock(&lock);
    FILE * fPtr;
    FILE * fTemp;
    char buffer[BUFSIZ];
    fPtr  = fopen("../db/Customers.txt", "r");
    fTemp = fopen("../db/replace.txt", "w"); 
    /* fopen() return NULL if unable to open file in given mode. */
    if (fPtr == NULL || fTemp == NULL)
    {
        /* Unable to open file hence exit */
        printf("\nUnable to open file.\n");
        printf("Please check whether file exists and you have read/write privilege.\n");
        return false;
    }
    printf("print strng\n%s\n", str);
    while ((fgets(buffer, BUFSIZ, fPtr)) != NULL)
    {
        if(buffer[strlen(buffer) - 1] == '\n'){
        	buffer[strlen(buffer)-1] = 0;
        }
        //replace all occurrence of word from current line
       if(strcmp(str, buffer) == 0){
           
            replaceAll(buffer, newbalance);
        }
        strcat(buffer, "\n");
        fputs(buffer, fTemp);
    }  

    fclose(fPtr);
    fclose(fTemp);
    /* Delete original source file */
    int del = remove("../db/Customers.txt");
    if (!del){

          /* Rename temp file as original file */
        rename("../db/replace.txt", "../db/Customers.txt");
        printf("Executing transaction of Rs. %0.2f for account %s\n", amt, acc_no);
    }
    else{
        remove("../db/replace.txt");
        perror("Unable to delete the file");
    }    

    
	//memset(oldbalance, 0, 100);
	//memset(newbalance, 0, 100); 
    pthread_mutex_unlock(&lock);
	
    return true;

}

void displayUsers(int connfd){
    FILE* fp;

    fp = fopen("../db/Customers.txt", "r");
    if(!fp){
        perror("Could not fetch user data!\n");
        return;
    }
    char details[100];
    int temp = 0;

    printf("\n");
    while (fscanf(fp, "%s", details) != EOF)
    {      
        sleep(0.5);
        write(connfd, details, strlen(details));
        memset(details, 0, strlen(details));
    }
    //printf("here\n");
    write(connfd, "end", 3);
    fclose(fp);
}

void signal_handler(){
    printf("Server shutting down...\n");
    pthread_mutex_destroy(&lock);
    exit(0);
}

bool checkText(char *temp){
    if(strcmp(temp, "end") == 0){

        return true;
    }else {
        return false;
    }
}

void beginCommunication(int connfd, pthread_mutex_t *l)
{
    lock = *l;
    srand(time(NULL));
    size_t n;
    char tempbuf[MAXLINE];
    memset(tempbuf, 0, strlen(tempbuf));
    while((n = read(connfd, tempbuf, 1)) != 0){
       // printf("Im here\n");
        if(strcmp(tempbuf, "a") == 0){
            char pass[50], id[20];
            memset(id, 0, strlen(id));
            n = read(connfd, id, BUFSIZ);
            if(checkText(id)) continue;
            memset(pass, 0, strlen(pass));
            n = read(connfd, pass, BUFSIZ);
            if(checkText(pass)) continue;

            printf("Validating user %s...\n", id);
            if(validateAdmin(id, pass)){
                printf("Success!\n");
                write(connfd, "1", 1);
            }else{
                printf("ERROR!\n");
                write(connfd, "0", 1);
            }
            memset(pass, 0, strlen(pass));
            memset(id, 0, strlen(id));


        }else if(strcmp(tempbuf, "c") == 0){
            srand(0);

            char name[100], tempAge[20], tempbalance[100];
            memset(name, 0, strlen(name));
            memset(tempAge, 0, strlen(tempAge));
            memset(tempbalance, 0, strlen(tempbalance));

            char acc_no[100];
            memset(acc_no, 0, strlen(acc_no));

            read(connfd, acc_no, MAXLINE);
            if(checkText(acc_no)) continue;

            read(connfd, name, MAXLINE);
            if(checkText(name)) continue;

            read(connfd, tempAge, MAXLINE);
            if(checkText(tempAge)) continue;

            read(connfd, tempbalance, MAXLINE);
            if(checkText(tempbalance)) continue;

            printf("Creating new account %s...\n", acc_no);
            if(createAccount(acc_no, name, tempAge, tempbalance)){
                write(connfd, "1", 1);
            }else{
                write(connfd, "0", 1);
            }
           
        } else if((strcmp(tempbuf, "w") == 0) || (strcmp(tempbuf, "d") == 0)){
               // printf("Im here in withdraw/ deposit too\n");
                char acc_no[100], str[100], newbalance[100], amount[100];
                memset(acc_no, 0, strlen(acc_no));
                memset(str, 0, strlen(str));
                memset(newbalance, 0, strlen(newbalance));
                memset(amount, 0, strlen(amount));

                read(connfd, acc_no, MAXLINE);
               // printf("Im here too acc_no %s\n", acc_no);

                if(checkText(acc_no)) {
                    memset(tempbuf, 0, strlen(tempbuf));
                    continue;
                }

                //printf("Im here too acc_no1 %s\n", acc_no);

                read(connfd, str, MAXLINE);
                if(checkText(str)) {
                    memset(tempbuf, 0, strlen(tempbuf));
                    continue;
                }

                read(connfd, newbalance, MAXLINE);
                if(checkText(newbalance)){
                    memset(tempbuf, 0, strlen(tempbuf));
                    continue;
                }

                read(connfd, amount, MAXLINE);
                if(checkText(amount)){
                    memset(tempbuf, 0, strlen(tempbuf));
                    continue;
                }

                float amt;
                amt = atof(amount);
                if(performTransaction(str, acc_no, newbalance, amt)){
                    write(connfd, "1", 1);
                    if(strcmp(tempbuf, "w") == 0){
                        printf("Withdrawing %f from account number %s..\n", amt, acc_no);
                        logTransaction(acc_no, newbalance, 'w', amt);
                    }
                    else if(strcmp(tempbuf, "d") == 0){
                        printf("Depositing %f to account number %s..\n", amt, acc_no);
                        logTransaction(acc_no, newbalance, 'd', amt);
                    }

                }else{
                    write(connfd, "0", 1);
                }
                
        }
        else if(strcmp(tempbuf, "p") == 0){
            printf("Fetching all customer details...\n");
            displayUsers(connfd);
        }
        else if(strcmp(tempbuf, "q") == 0 || strcmp(tempbuf, "end") == 0){
            break;
        }
            
        memset(tempbuf, 0, strlen(tempbuf));
        //printf("Im here too tempbuf %s\n", tempbuf);

    }

    //     write(connfd, buf, n);
	//if (buf[0] == '\n')
	//	break;
    
}
