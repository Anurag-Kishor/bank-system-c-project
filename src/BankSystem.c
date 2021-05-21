#include<stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include<unistd.h>  
#include <time.h>
#include "../include/banksystem.h"
#include<signal.h>
#include <netdb.h>

//Bank System - Admin Side
/*
This is a simulation of the bank system where customer can deposit, withdraw money into/from their account.
Customer will need to create an account with the bank first.
*/

int main(){
    printf("\e[1;1H\e[2J");
    signal(SIGALRM, alarm_handler);
    alarm(5);
    while(1){
        
        if(connectToServer()){
            signal(SIGALRM, SIG_IGN);
            break;
        }
    }
    //printf("ClientID %d\n", clientfd);
    bool login_check = signinscreen();
   // printf("%d", login);
    if(login_check){
        displayMenu();
    }else{
        printf("Exiting the system...\n");
    }
}