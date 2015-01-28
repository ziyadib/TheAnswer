#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>


int creer_serveur(int port){
char *MESSAGE_BIENVENUE = "salut"; /*"Bonjour, Welcome, Bienvenido, Bienvenito, ahlan wa sahlan, vous etes connecté au merveilleux serveur The Answer, tout vos desirs sont des ordres je me ferais un réel plaisir de vous servir et de pouvoir repondre à vos magnifiques questions avec la plus belle facon possible, c'est a dire avec un super smile :D, bonnne navigation et au plaisir de vous revoir tres bientot et tres souvent merveilleux utilisateur que vous etes";
  */
struct sockaddr_in saddr, client_addr;

int socket_serveur, clientfd, retfd ;
unsigned int size_client;
char sms_client[256];
socket_serveur = socket(AF_INET,SOCK_STREAM, 0);

    if ( socket_serveur == -1)
    {
        perror("socket_serveur");
        /* traitement de l ’ erreur */
        return -1;
}
saddr.sin_family = AF_INET ; /* Socket ipv4 */
saddr.sin_port = htons (port); /* Port d ’ écoute */
saddr.sin_addr.s_addr = INADDR_ANY ; /* écoute sur toutes les interfaces */

    if(bind(socket_serveur,(struct sockaddr *)&saddr , sizeof(saddr)) == -1)
    {
            perror ( " bind socker_serveur " );
             /* traitement de l ’ erreur */
            return -1;
      }
      
     if(listen(socket_serveur,10) == -1){
perror ( " listen socket_serveur " );
/* traitement d ’ erreur */
    return -1;
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
    /*envoie du sms de bienvenue */
    if(write(clientfd,MESSAGE_BIENVENUE, sizeof(MESSAGE_BIENVENUE))==-1){
    perror("error welcome message");
    return -1;
    }
    printf("jecris un sms de bienvenue\n");
    while(1){
        if((retfd=read(clientfd, sms_client, 256))==-1){
        return -1;
        }
        
        sms_client[retfd] = '\0';
        printf("%s \n", sms_client);
       if(write(socket_serveur, sms_client, retfd)==-1){
       perror("error perroquet");
       return -1;
    }
    }
 }
    

return 0;
}
