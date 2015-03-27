#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


void traitement_signal(int sig){
    waitpid(-1, NULL, WNOHANG);
    printf("Signal %d reçu \n client deconnecté \n",sig);
}

void initialiser_signaux(void){
    struct sigaction sa ;
    sa.sa_handler = traitement_signal ;
    sigemptyset (&sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        perror("signal");
    }
    if (sigaction(SIGCHLD,&sa,NULL)== -1){
        perror("sigaction(SIGCHLD)" );
    }
}
