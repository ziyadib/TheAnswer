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
#include "http.h"
#include "outils.h"

char *rewrite_url(char *url){
    while (*url != '\0' && *url != '?')
        url++;
    *url = '\0';
    return url;
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
