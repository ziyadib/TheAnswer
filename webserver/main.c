#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <sys/wait.h>


void traitement_signal(int sig){
    waitpid(-1, NULL, WNOHANG);
    printf("Signal%dreçu\n client deconnecté",sig);
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

void filterString(char string[]){
    unsigned int i;
    char r= '\r';
    char n ='\n';
    for(i =0; i<strlen(string); i++){
        if(string[i] == r || string[i] == n ){
            string[i]= '\0';
        }
    }
}

char *fgets_or_exit(char *buffer , int size , FILE *stream){
    char *retgets;
    if((retgets = fgets(buffer, size, stream)) == NULL){
        exit(0);
    }
    return retgets;
}

void first_line(int size, FILE *open){
    int ok = 1; //0=false, 1=true;
    const char *chaine_erreur= "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n400 Bad request\r\n";
    const char *chaine_erreur404= "HTTP/1.1 404 Not found\r\nConnection: close\r\nContent-Length: 15\r\n404 Not found\r\n";
    const char* okSMS = "HTTP/1.1 200 OK\r\n";
    char premiere_ligne[1024];
    char method[10], url[1024];
    int major, minor;
    fgets(premiere_ligne, size, open);
    sscanf("%s %s HTTP/%d.%d", method, url, &major, &minor);
    if(strcmp(method, "GET")!= 0){
        fprintf(open,"%s", chaine_erreur);
        ok=0;
    }
    if(strcmp(url, "/")!= 0){
        fprintf(open,"%s", chaine_erreur404);
        ok=0;
    }
    if(major != 1){
        fprintf(open,"%s", chaine_erreur);
        ok=0;
    }
    if(minor !=0 && minor !=1){
        fprintf(open,"%s", chaine_erreur);
        ok=0;
    }
    if(ok==0)
        fprintf(open, "%s", okSMS);
}

int main(){
	int clientfd, /*retfd,*/socket_serveur ;
	unsigned int size_client;
    char sms_client[1024] ;
	struct sockaddr_in client_addr;
	char *serverName = "TheAnswer";
	char *retgets;
	/*char *MESSAGE_BIENVENUE = "Bonjour, Welcome, Bienvenido, Bienvenito, ahlan wa sahlan, vous etes connecté au merveilleux serveur The Answer, tout vos desirs sont des ordres je me ferais un réel plaisir de vous servir et de pouvoir repondre à vos magnifiques questions avec la plus belle facon possible, c'est a dire avec un super smile :D, bonnne navigation et au plaisir de vous revoir tres bientot et tres souvent merveilleux utilisateur que vous etes\n";
	*/
	initialiser_signaux();
	if((socket_serveur=creer_serveur(8080))==-1){
		perror("pb creer_serveur");
	}
	/* connexion au serveur */
	while(1){
		size_client = sizeof(client_addr);
		clientfd = accept(socket_serveur,(struct sockaddr *) &client_addr,&size_client);
   
		if(clientfd == -1){
			perror("error accept");
			return -1;
		}
		printf("hey un nouveau client est connecté\n");

        if(fork()==0){
		FILE *open;
		if((open= fdopen(clientfd , "w+"))== NULL){
           perror("fdopen error");
           return -1;
        }
         
		/*envoie du sms de bienvenue */
		/*if(write(clientfd,MESSAGE_BIENVENUE, strlen(MESSAGE_BIENVENUE))==-1){
			perror("error welcome message");
			return -1;
		}
		
		printf("jecris un sms de bienvenue\n");*/
		first_line(1024, open);
		do{
            /*utilisation de fgets au lieu de read pour lire une ligne recu du client*/
		    retgets = fgets_or_exit(sms_client, 1024, open);
            printf("%s", sms_client);
		        /* on lui renvoie le message recu par le client */
		         
			if(fprintf(open, "<%s>  %s",serverName, sms_client)<=0){
				perror("error perroquet fprintf");
				return -1;
			}
		        filterString(sms_client);
        } while(retgets != NULL || strlen(sms_client));
			
			
		
        /*	printf("%s \n je suis avant le perroquet", sms_client);
			if(fprintf(open, "<%s>  %s",serverName, sms_client)<=0){
				perror("error perroquet fprintf");
				return -1;
			}*/
			
		/*printf("client deconnecté\n");*/
            exit(0);
		
		} else {
            traitement_signal(SIGCHLD);
            close(clientfd);
    /* fermeture du client dans le pere i.e: il ext executé dans le fils*/
		}
	}
	return 0;
}

