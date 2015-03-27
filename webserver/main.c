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
#include <sys/stat.h>
#include <fcntl.h>
#include "signaux.h"
#include "http.h"
#include "outils.h"

int main(int argc, char *argv[]){
    http_request request; 
    int clientfd, socket_serveur ;
    unsigned int size_client;
    char* chemin;
    struct stat buf;
    struct sockaddr_in client_addr;
    /*char *MESSAGE_BIENVENUE = "Bonjour, Welcome, Bienvenido, Bienvenito, ahlan wa sahlan, vous etes connecté au merveilleux serveur The Answer, tout vos desirs sont des ordres je me ferais un réel plaisir de vous servir et de pouvoir repondre à vos magnifiques questions avec la plus belle facon possible, c'est a dire avec un super smile :D, bonnne navigation et au plaisir de vous revoir tres bientot et tres souvent merveilleux utilisateur que vous etes\n";*/
    int fd_target;

    initialiser_signaux();
    if((socket_serveur=creer_serveur(8080))==-1){
        perror("pb creer_serveur");
    }
    /* Connexion au serveur */
    while(1){
        size_client = sizeof(client_addr);
        clientfd = accept(socket_serveur,(struct sockaddr *) &client_addr,&size_client);
        if(clientfd == -1){
            perror("error accept");
            return -1;
        }
        chemin = strcat(argv[1],request.url);
        printf("document_root : %s\n", chemin);
        if(stat(chemin, &buf)==0){/* Ignore if directory */
            if(S_ISDIR(buf.st_mode)== -1){
                printf("%s %d\n", chemin, argc);
                return -1;
            }
        } else {
            perror("stat");
            return -1;
        }
        printf("Nouveau client connecté\n");
        if(fork()==0){
            FILE *open;
            if((open = fdopen(clientfd, "w+"))== NULL){
                perror("fdopen error");
                return -1;
            }
            int bad_request = parse_http_request(open, &request);
            skip_headers(open);

            printf("bad request= %d\n", bad_request);

            /* Envoi de la reponse au client */
            if (bad_request == -2)
                send_response(open , 400, request, "Bad Request", "Bad request\r\n");
            else if (request.method == HTTP_UNSUPPORTED)
                send_response(open , 405, request, "Method Not Allowed", "Method Not Allowed\r\n");
            else /*if (strcmp(request.url, "/") == 0)*/{

                /*
                 * Si l'url est /, il faut modifier request.url pour lui donner la valeur index.html
                 */
                if (strcmp(request.url, "/") == 0){
                    strcpy(request.url, "/index.html");
                }


                fd_target = check_and_open(request.url, argv[1]);
                printf("Valeur de retour de check_and_open: %d\n", fd_target);


                if(fd_target == -1){
                    send_response(open , 404, request, "Not Found", "Not Found\r\n");
                } else {
                    /*send_response(open , 200,request, "OK", MESSAGE_BIENVENUE);*/
                    send_status(open, 200, "OK", request);
                    fprintf(open, "Content-Length: %d", get_file_size(fd_target));

                    /* Il faut une ligne vide, donc une ligne qui ne contient qu'une seule chose: un retour à la ligne */
                    fprintf(open, "\r\n\r\n");

                    /* Un fflush pour s'assurer que les fprintf se passe avant le copy */
                    fflush(open);

                    copy(fd_target, clientfd);
                }
            }			
            exit(0);
        } else {
            traitement_signal(SIGCHLD);
            close(clientfd);
            /* Fermeture du client dans le pere i.e: il est executé dans le fils*/
        }
    }
    return 0;
}

