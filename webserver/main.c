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

enum http_method {
HTTP_GET ,
HTTP_UNSUPPORTED ,
};
typedef struct
{
enum http_method method;
int major_version;
int minor_version;
char url[1024];
} http_request;

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

int parse_http_request(FILE *open,http_request *request){
    /* la lecture de la premiere ligne etait deja faite precedemment dans cette fonction */
    int ok = 1; //0=false, 1=true;
    char premiere_ligne[1024];
    char method[10], url[1024];
    
    fgets(premiere_ligne, 1024, open);
    sscanf(premiere_ligne,"%s %s HTTP/%d.%d", method, request->url, &request->major_version,&request->minor_version);

        
    if(strcmp(method, "GET")!= 0){
  
        ok=-1;
          
    }
    else if(strcmp(url, "/")!= 0){
    
        ok=0;
        
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
    }else {
    request->method = HTTP_UNSUPPORTED;
    }
     
    return ok;
}

void skip_headers(FILE *client){
    char message[1024];
    char *ret = fgets_or_exit(message,1024,client);
    int n = strlen(ret);
    while (n > 2){
    ret = fgets_or_exit(message,1024,client);
    }
}
char *rewrite_url(char *url){
    int i = 0;
    while(url[i] != '?'){
        i++;
    }
    char *subbuff= malloc(sizeof(url));
    memcpy(subbuff, &url[0],i);
    subbuff[i] = '\0';   
   return subbuff;
}

void send_status(FILE *client , int code , const char *reason_phrase, http_request request){
      
char status_line[1024]; 
 snprintf(status_line, 1024,"HTTP/%d.%d %d %s \r\n", request.major_version, request.minor_version, code, reason_phrase);
fprintf(client,"%s",status_line);

}
void send_response(FILE *client , int code ,http_request request, const char *reason_phrase ,const char *message_body){
    send_status(client,code,reason_phrase, request);

    /* envoie le corps du message selon les differents cas */
    fprintf(client,"%s",message_body);
}
int main(){
    http_request request; 
	int clientfd, /*retfd,*/socket_serveur ;
	unsigned int size_client;
    char sms_client[1024] ;
	struct sockaddr_in client_addr;
	
	/*char *serverName = "TheAnswer";*/
	
	char *MESSAGE_BIENVENUE = "Bonjour, Welcome, Bienvenido, Bienvenito, ahlan wa sahlan, vous etes connecté au merveilleux serveur The Answer, tout vos desirs sont des ordres je me ferais un réel plaisir de vous servir et de pouvoir repondre à vos magnifiques questions avec la plus belle facon possible, c'est a dire avec un super smile :D, bonnne navigation et au plaisir de vous revoir tres bientot et tres souvent merveilleux utilisateur que vous etes\n";
	
	  const char *chaine_erreur= "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n400 Bad request\r\n";
    const char *chaine_erreur404= "HTTP/1.1 404 Not found\r\nConnection: close\r\nContent-Length: 15\r\n\r\n404 Not found\r\n";
      const char* okSMS = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 13\r\n\r\nTout est ok\r\n";
    
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
		int ok =parse_http_request(open,&request);
		do{
            /*utilisation de fgets au lieu de read pour lire une ligne recu du client*/
		     fgets_or_exit(sms_client, 1024, open);
            printf("%d %s \n",(int) strlen(sms_client), sms_client);
		        /* on lui renvoie le message recu par le client 

		         
			if(fprintf(open, "<%s>  %s",serverName, sms_client)<=0){
				perror("error perroquet fprintf");
				return -1;
			}*/
		        filterString(sms_client);
		         printf("%d %s \n",(int) strlen(sms_client), sms_client);
        } while(strlen(sms_client));
			printf("the answer sort de la boucle de lecture");
			if (ok == -1){
			fprintf(open,"%s",chaine_erreur404);
			}else{
			fprintf(open,"%s",MESSAGE_BIENVENUE);
			}
			if(ok==0)
			fprintf(open,"%s",chaine_erreur);
			if(ok==1)
			fprintf(open,"%s",okSMS);
			
		

			
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

