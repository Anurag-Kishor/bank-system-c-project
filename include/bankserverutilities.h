int open_listenfd(char *port);
void logTransaction(char* acc_no, char* new_balance, char c, float amt);
bool createAccount(char *acc_no, char* name, char* age, char* balance);
bool validateAdmin(char* id, char* pass);
void replaceAll(char *str, char *newWord);
bool performTransaction(char *str, char* acc_no, char* newbalance, float amt);
void displayUsers(int connfd);
void signal_handler();
void beginCommunication(int connfd, pthread_mutex_t *l);