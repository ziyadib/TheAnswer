#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>


int creer_serveur(int port){

struct sockaddr_in saddr;

int socket_serveur ;

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
 

return 0;
}
