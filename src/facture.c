#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg){
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]){
    /*INITIALISATON DES VARAIABLES ET DES STRUCTS*/
    int sockfd, portnum;
    struct sockaddr_in serv_addr; //Structure de socket pour notre serveur
    struct hostent *server; //Structure qui permet de recuperer une adresse par un nom
    char buffer[256]; // On fera un write avec cette variable pour l'envoyer au serveur

    /*On verifie qu'on a bien le bon nombre d'arguments*/
    if (argc < 6) {
       fprintf(stderr,"usage %s ip_adress portnum id_logement type montant\n", argv[0]);
       exit(0);
    }

	/*Ouverture d'un socket*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //on ouvre un socket en recuperant son file descriptor
    if (sockfd < 0) error("ERROR opening socket");

	/*Recuperation du Host*/
    server = gethostbyname(argv[1]); //On definit l'adresse du serveur
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

	/*Entree des infos serveur ( adresse + port )*/
    portnum = atoi(argv[2]); //On stocke le numero du port
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portnum);
	
	/*Connexion au serveur*/
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) error("ERROR connecting");

	/*ENVOI DE DONNE AU SERVEUR*/
	sprintf(buffer,"%d %s %d\n", atoi(argv[3]), argv[4], atoi(argv[5]));
    write(sockfd,buffer,strlen(buffer));

    close(sockfd);
    return 0;
}
