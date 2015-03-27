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


char *fgets_or_exit(char *buffer , int size , FILE *stream){
    char *retgets;
    if((retgets = fgets(buffer, size, stream)) == NULL){
        exit(0);
    }
    return retgets;
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
