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

enum http_method {
    HTTP_GET ,
    HTTP_UNSUPPORTED ,
};
typedef struct {
    enum http_method method;
    int major_version;
    int minor_version;
    char url[1024];
} http_request;

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

char *rewrite_url(char *url){
    while (*url != '\0' && *url != '?')
        url++;
    *url = '\0';
    return url;
}

int parse_http_request(FILE *open, http_request *request){
    /* La lecture de la premiere ligne etait deja faite precedemment dans cette fonction */
    int ok = 1; //0=false, 1=true;
    char premiere_ligne[1024];
    char method[10];

    fgets(premiere_ligne, 1024, open);
    printf("Trying to parse the first line\n");
    printf(" - first line: %s\n", premiere_ligne);

    sscanf(premiere_ligne,"%s %s HTTP/%d.%d", method, request->url, &request->major_version,&request->minor_version);
    if(strcmp(method, "GET")!= 0){
        ok=-2;
    }
    else if(request->major_version != 1){
        ok=0;
    }
    else if(request->minor_version !=0 && request->minor_version !=1){
        ok=0;
    }
    if(ok == 1){
        /*si ok==1 alors on a forcement la method GET*/
        request->method = HTTP_GET;        
    }else if(ok == -2){
        request->method = HTTP_UNSUPPORTED;
    }

    rewrite_url(request->url);

    return ok;
}

void skip_headers(FILE *client){
    char message[1024];
    char *ret = fgets_or_exit(message, 1024, client);
    while (strlen(ret)>2){
        ret = fgets_or_exit(message, 1024, client);
    }
}

void send_status(FILE *client , int code , const char *reason_phrase, http_request request){
    char status_line[1024]; 
    snprintf(status_line, 1024,"HTTP/%d.%d %d %s \r\n", request.major_version, request.minor_version, code, reason_phrase);
    fprintf(client, "%s", status_line);
}

void send_response(FILE *client , int code ,http_request request, const char *reason_phrase ,const char *message_body){
    send_status(client,code,reason_phrase, request);
    /* envoie le corps du message selon les differents cas */
    fprintf(client, "%s", message_body);
    fflush(client);
}

int get_file_size(int fd){
    struct stat buf;
    if(fstat(fd, &buf)==-1)
        return -1;
    return buf.st_size;
}

int check_and_open(const char* url, const char* document_root){
    char buffer_url[256];
    struct stat buf;
    snprintf(buffer_url, 256, "%s%s", document_root, url);
    printf("Chemin de la ressource demandée : %s\n", buffer_url);
    if(stat(buffer_url, &buf) == 0){
        if(S_ISREG(buf.st_mode) == -1){
            perror("Is not regular file !");
            return -1;
        } else {
            return open(buffer_url, O_RDONLY);
        }
    }
    return -1;
}


int copy(int in, int out){
    char buff[128];
    int ret;

    while ((ret = read(in, buff, 128)) > 0)
    {
        /* Tant que j'ai lu au moins 1 octet */
        if (write(out, buff, ret) == -1)
        {
            /* J'écris le même nombre d'octets */
            perror("copy_write");
            return -1;
        }
    }

    if (ret == -1)
    {
        perror("copy_read");
        return -1;
    }

    return 0;
}

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

/*
   git commit -m 'tag'
   git tag nomTag
   git push --tags
   git commit -am 'modif effectuée'
   git push
   */
