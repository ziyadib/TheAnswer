#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "socket.h"



int creer_serveur(int port){

struct sockaddr_in saddr ;
saddr.sin_family = AF_INET ; /* Socket ipv4 */
saddr.sin_port = htons (8000); /* Port d ’ écoute */
saddr.sin_addr.s_addr = INADDR_ANY ; /* écoute sur toutes les interfaces */

int socket_serveur ;
socket_serveur = socket(AF_INET,SOCK_STREAM, 0);
if ( socket_serveur == -1)
{
perror("socket_serveur");
/* traitement de l ’ erreur */
}

if(bind(socket_serveur,(struct sockaddr *)&saddr , sizeof(saddr)) == -1)
{
perror ( " bind socker_serveur " );
/* traitement de l ’ erreur */
}


}