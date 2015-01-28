#include <stdio.h>
#include <string.h>
#include "socket.h"

int main(){

if(creer_serveur(8080)==-1){
perror("pb creer_serveur");
}

return 0;
}
