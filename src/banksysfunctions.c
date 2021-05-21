#include<stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include<unistd.h>  
#include <time.h>
#include<pthread.h>
#include <netdb.h>


/*Contains all the functions that will be used by the client side

alarm_handler() - will execute if client is unable to connect to the server after a pre-decided time.
generateAccountNumber() - generates a random 15 digit account number
findAccountNumber() - used to find the account from database
createAccount() - Used to create the customer's account
withdrawMoney() - Used to withdraw money
depositMoney() - Used to deposit money
displayUsers() - Displays the list of all the Customers
searchCustomer() - Shows the details of customer based on account number.
displayMenu() - Bank Menu function

*/


#define	MAXLINE	 8192  /* Max text line length */
int clientfd;

void alarm_handler(){
    printf("ERROR connecting to the server. Exiting the system..\n");
    //write(clientfd, "end", 3);
    exit(0);
}

int open_clientfd(char *hostname, char *port, char* host, char* service) {
    int clientfd;
    struct addrinfo hints, *listp, *p;
	
    int flags;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections where we get IPv4 or IPv6 addresses */
    getaddrinfo(hostname, port, &hints, &listp);
  
    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

		flags = NI_NUMERICHOST | NI_NUMERICSERV; /* Display address string instead of domain name and port number instead of service name */		
		getnameinfo(p->ai_addr, p->ai_addrlen, host, MAXLINE, service, MAXLINE, flags);
        //printf("host:%s, service:%s\n", host, service);
		
        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
		{
			printf("Connected to server %s at port %s\n", host, service);
            break; /* Success */
		}
        close(clientfd); /* Connect failed, try another */  //line:netp:openclientfd:closefd
    } 

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}

bool connectToServer(){
    char host[MAXLINE], service[MAXLINE];
    clientfd = open_clientfd("localhost", "15000", host, service);
    if(clientfd == -1){
        return false;
    }
    printf("host:%s, service:%s\n", host, service);
    return true;
}

void generateAccountNumber(char *str, size_t size)
{
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}

void quitMenu(){

    printf("\nPress q to return back to menu..\n");
    char ch;
    while(ch != 'q'){

        scanf("%c", &ch);

    }

}

bool findAccountNumber(char *acc_no, float* balance, char* dets){
    FILE* fptr;
    fptr = fopen("../db/Customers.txt", "r");
    if(fptr == NULL){
        printf("Cannot find customer!\n");
        return false;
    }
    char str[100];
    int temp = 0;
    int flag = 0;
    while(fscanf(fptr, "%s", str) != EOF){
            

        temp++;
        if(temp == 1 && strcmp(acc_no, str) == 0){
            flag = 1;

            strcat(dets, str);

        }
        if((temp == 2 || temp == 3 || temp ==4) && flag == 1){
            strcat(dets, " ");
            strcat(dets, str);
        }
        if(temp == 5 && flag == 1){
            *balance = atof(str);
            strcat(dets, " ");
            strcat(dets, str);
            strcpy(str,"");
            fclose(fptr);
            return true;
        }
        else if(temp == 5){
            temp = 0;
            flag = 0;
            strcpy(dets, "");
        }

    }
    fclose(fptr);
    strcpy(str,"");

    return false;
}

bool login(){
    write(clientfd, "a", 1);

    size_t n;
    char tempbuf[BUFSIZ];
    char pass[50], id[20];
 //   printf("ClientID %d\n", clientfd);
    printf("Enter User ID: \n");
    scanf("%s", id);
    write(clientfd, id, strlen(id));

    printf("Enter password: \n");
    scanf("%s", pass);
    write(clientfd, pass, strlen(pass));
    
    if( n = read(clientfd, tempbuf, BUFSIZ) != 0 && strcmp(tempbuf, "1") == 0){
        printf("Login Successful.. \n");
        return true;
    }else{
        printf("%s\n", tempbuf);
        printf("Credentials do not match. \n");
        return false;
    }
}

int signinscreen(){

    char c;
    bool flag = 1;
    printf("--------------------Bank System-----------------\n");
    while(flag){
        printf("Would you like to login? (Y/N)\n");
        scanf(" %c", &c);
        if(c == 'Y' || c == 'y'){
            if(login()){
                return 1;
            }
        }else{
            flag = 0;
        }
    }
   return flag;

}

bool createAccount(){
    printf("\e[1;1H\e[2J");
    write(clientfd, "c", 1);
    char name[100], tempAge[20], tempbalance[100];
    int age;
    float balance;
    size_t len = 100;

    char acc_no[100];

    generateAccountNumber(acc_no, 15);
    write(clientfd, acc_no, strlen(acc_no));
    getchar();
    printf("-----------Account Creation---------\n");
    printf("Enter Account Holder's Name: ");
    fgets(name, 100, stdin);

    if(name[strlen(name)-1] == '\n'){
        name[strlen(name)-1] = 0;
    }
    write(clientfd, name, strlen(name));

    while(1){
        printf("Enter age: ");
        scanf("%d", &age);
        if(age < 18){
            printf("Age has to be greater than 18! Try again..\n");
        }else{
            break;
        }
    }
    sprintf(tempAge, "%d", age);

    write(clientfd, tempAge, strlen(tempAge));

    while(1){
        printf("Enter initial balance amt (minimum 500): ");
        scanf("%f", &balance);
        if(balance < 500){
            printf("You need at least Rs. 500 as initial balance\n");
        }else{
            break;
        }
    }
    sprintf(tempbalance, "%0.6f", balance);
    printf("tempbalance %s", tempbalance);

    write(clientfd, tempbalance, strlen(tempbalance));
    char c[10];
    read(clientfd, c, 1);
    if(strcmp(c, "1") == 0){
        printf("\e[1;1H\e[2J");
        printf("\n-----Account created----\n\n");
        printf("Account Number: %s\n", acc_no);
        printf("Name of account owner: %s\n", name);
        printf("Age: %s\n", tempAge);
        printf("Current Balance: %s\n", tempbalance);
    }else{
        printf("Could not create account!\n");
    }
    quitMenu();
    return true;

}

bool withdrawMoney(){
    write(clientfd, "w", 1);
    printf("\e[1;1H\e[2J");
    char acc_no[100];
    float currBalance;
    float amt = 0;
    char str[BUFSIZ];
    printf("Enter account number: ");
    scanf("%s", acc_no);
    if(findAccountNumber(acc_no, &currBalance, str)){
        printf("Current Account balance: %f\n", currBalance);
    }else{
        printf("Account not found!\n");
        memset(acc_no, 0, 100);
        memset(str, 0, BUFSIZ);
        quitMenu();
        write(clientfd, "end", 3);
        return false;

    }

    printf("Enter amount to withdraw: ");
    scanf("%f", &amt);
    char ch;
    if(amt > currBalance){
        getchar();
        printf("Withdraw amount is more than your Current Balance. Try again? (Y/N)\n");
        scanf("%c", &ch);
        if(ch != 'Y' || ch != 'y'){
            memset(acc_no, 0, strlen(acc_no));
            memset(str, 0, strlen(str));
            //return false;
        }else{
            write(clientfd, "end", 3);
            return false;
        }
    }
    

    char newbalance[100];
    currBalance = currBalance - amt;
    sprintf(newbalance, "%f", currBalance);

    char withdrawAmt[100];
    sprintf(withdrawAmt, "%f", amt);
    sleep(0.5);

    write(clientfd, acc_no, strlen(acc_no)); 
    sleep(0.5);
    write(clientfd, str, strlen(str));
    sleep(0.5);
    write(clientfd, newbalance, strlen(newbalance));
    sleep(0.5);
    write(clientfd, withdrawAmt, strlen(withdrawAmt));

    char c[2];
    read(clientfd, c, 1);
    if(strcmp(c, "1") == 0){
        printf("Amount withdrawn successfully!\n");
        printf("New balance: %s\n", newbalance);
    }else{
        printf("Unable to Deposit amount!\n");
    }

    memset(acc_no, 0, 100);
    memset(str, 0, BUFSIZ);
    memset(newbalance, 0, 100);
    quitMenu();
}

void depositMoney(){
    printf("\e[1;1H\e[2J");
    write(clientfd, "d", 1);
    char acc_no[100];
    float currBalance;
    float amt = 0;
    char str[BUFSIZ];
    printf("Enter account number: ");
    scanf("%s", acc_no);
    if(findAccountNumber(acc_no, &currBalance, str)){
        printf("Current Account balance: %f\n", currBalance);
    }else{
        printf("Account not found!\n");
        memset(acc_no, 0, 100);
        memset(str, 0, BUFSIZ);
        quitMenu();
        write(clientfd, "end", 3);
        return;
    }

    sleep(0.5);
    printf("Enter amount to deposit: ");
    scanf("%f", &amt);
    if(amt <= 0){
        printf("Amount has to be more than Rs. 0\n");
        write(clientfd, "end", 3);
        memset(acc_no, 0, 100);
        memset(str, 0, BUFSIZ);
        quitMenu();
        return;
    }
    char ch;
    
    char newbalance[100];
    currBalance = currBalance + amt;
    sprintf(newbalance, "%f", currBalance);


    char depositAmt[100];
    sprintf(depositAmt, "%f", amt);
    sleep(0.5);

    write(clientfd, acc_no, strlen(acc_no)); 
    sleep(0.5);
    write(clientfd, str, strlen(str));
    sleep(0.5);
    write(clientfd, newbalance, strlen(newbalance));
    sleep(0.5);
    write(clientfd, depositAmt, strlen(depositAmt));

    char c[2];
    read(clientfd, c, 1);
    if(strcmp(c, "1") == 0){
        printf("Amount Deposited successful!\n");
        printf("New balance: %s\n", newbalance);
       // logTransaction(acc_no, newbalance, ', amt);
    }else{
        printf("Unable to deposit amount!\n");
    }

//---------
    
    memset(acc_no, 0, 100);
    memset(str, 0, BUFSIZ);
    memset(newbalance, 0, 100);
    quitMenu();
}

void displayUsers(){
    printf("\e[1;1H\e[2J");
    write(clientfd, "p", 1);
    size_t n;
    char str[100];
    int temp = 0;

    printf("------------Account Details--------------\n");
    while((n = read(clientfd, str, BUFSIZ)) != 0){
        if(strcmp(str, "end") == 0) break;
        temp++;
        if(temp == 1){
            printf("Account Number: %s\n", str);
        }else if(temp == 2){
            printf("Name: %s ", str);
        }else if(temp == 3){
            printf("%s\n", str);
        }else if(temp == 4){
            printf("Age: %s\n", str);
        }else if(temp == 5){
            printf("Current Balance: %s\n\n", str);
            temp = 0;
        } 
        memset(str, 0, strlen(str));
    }
    quitMenu();
}

void searchCustomer(){
    printf("\e[1;1H\e[2J");
    char acc_no[100], details[BUFSIZ];
    printf("Enter account number: \n");
    scanf("%s", acc_no);
    char name[100], age[5], balance[25];
    float amt;
    if(findAccountNumber(acc_no, &amt, details)){
        printf("\nAccount found!\n\n");
        memset(name, 0, strlen(name));
        memset(age, 0, strlen(age));
        memset(balance, 0, strlen(balance));

      //  printf("%s\n", details);
        char *ptr = strtok(details, " \n\0");
        int temp = 0;
        while(ptr != NULL)
        {   
            temp++;
            if(temp == 2){
                strcpy(name, ptr);
            }else if(temp == 3){
                strcat(name, " ");
                strcat(name, ptr);
            }else if(temp == 4){
                strcpy(age, ptr);
            }
            else if(temp == 5){
                strcpy(balance, ptr);
                temp = 0;
            }
            ptr = strtok(NULL, " \n\0");
        }

        printf("Account number: %s\n", acc_no);
        printf("Accountholder Name: %s\n", name);
        printf("Age: %s\n", age);
        printf("Current Balance: %s\n", balance);
        quitMenu();
    }else{

        printf("Account not found!\n");
        quitMenu();
    }

}

int displayMenu(){

   int ch;
   pthread_t threads[4];

   
  // defaultHandler = signal(SIGINT, sign_handler);
   srand(time(NULL));
   while(ch != 6){
        getchar();
        printf("\e[1;1H\e[2J");
        printf("--------------------------Bank Menu-----------------\n");
        printf("1. Create a user's Account\n");
        printf("2. Withdraw Money.\n");
        printf("3. Deposit Money.\n");
        printf("4. Display Users.\n");
        printf("5. Search for an account.\n");
        printf("6. Logout and Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &ch);
        if(ch == 1){
            createAccount();
        }else if(ch == 2){
            withdrawMoney();
        }else if(ch == 3){
            depositMoney();
        }else if (ch == 4){
            displayUsers();
        }else if(ch == 5){
            searchCustomer();
        }
   }

   write(clientfd, "q", 1);
}
