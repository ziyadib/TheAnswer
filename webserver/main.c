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

void initialiser_signaux(void){
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
		perror("signal");
	}
}

int main(){
	int clientfd, /*retfd,*/socket_serveur ;
	unsigned int size_client;
	char sms_client[1024];
	struct sockaddr_in client_addr;
	char *serverName = "TheAnswer";
	char *MESSAGE_BIENVENUE = "Bonjour, Welcome, Bienvenido, Bienvenito, ahlan wa sahlan, vous etes connecté au merveilleux serveur The Answer, tout vos desirs sont des ordres je me ferais un réel plaisir de vous servir et de pouvoir repondre à vos magnifiques questions avec la plus belle facon possible, c'est a dire avec un super smile :D, bonnne navigation et au plaisir de vous revoir tres bientot et tres souvent merveilleux utilisateur que vous etes\n";
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
		if(write(clientfd,MESSAGE_BIENVENUE, strlen(MESSAGE_BIENVENUE))==-1){
			perror("error welcome message");
			return -1;
		}
		printf("jecris un sms de bienvenue\n");
		while(1){
		/*utilisation de fgets au lieu de read pour lire une ligne recu du client*/
		    if(fgets(sms_client, 1024, open) == NULL){
		        perror("fgets error");
		        return -1;
		    }
			
			printf("%s \n", sms_client);
			if(fprintf(open, "<%s>  %s",serverName, sms_client)<=0){
				perror("error perroquet fprintf");
				return -1;
			}
		}
		printf("client deconnecté\n");
		exit(0);
		
		}else{
		close(clientfd);
    /* fermeture du client dans le pere i.e: il ext executé dans le fils*/
		}
	}
	return 0;
}
