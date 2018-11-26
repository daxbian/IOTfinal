#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <time.h>
#include <sqlite3.h>

pid_t pid;
sqlite3 *db;
char *err_msg;
char req[4095];
int rc;

// retourne un entier aléatoire entre 0 et n
int int_rand(int n){
    return (rand()%n);
}

void error(const char *msg){
    perror(msg);
    exit(1);
}

void insertFactures(char* type, int val, int id){
    sqlite3_stmt *stmt;
    int i, s;

    // initialisation de la graine pour les nbs aléatoires
    srand((unsigned int)time(NULL));

    // ouverture de la base de données
    sqlite3_open("sql/mabdd.db", &db);

    // insertion de données dans la base
    // requête SQL insert
    sprintf(req,"INSERT INTO FACTURE(id_logement, type, montant, valeur) VALUES(%d,'%s',%d, %d);", id, type, val, val);

    printf("sql:\t|%s|\n",req);
    // exécution de la requête
    sqlite3_exec(db, req, 0, 0, &err_msg);
    printf("err_msg:|%s|\n\n",err_msg);

    // lecture dans la base
    // requête SQL select
    sprintf(req,"SELECT * FROM FACTURE;");
    printf("%s\n\n",req);

    // préparation de la requête
    rc=sqlite3_prepare_v2(db, req, -1, &stmt, 0);

    // parcours des enregistrements
    while(1){
        // lecture de l'enregistrement suivant
        s=sqlite3_step(stmt);
        // enregistrement existant
        if (s==SQLITE_ROW){
            for(i = 0; i < sqlite3_column_count(stmt); i++){
                const unsigned char *val=sqlite3_column_text(stmt, i);
                // affichage de la valeur
                printf("%s\t\t", val);
            }
            printf("\n");
        }
    // parcours terminé
    else if (s==SQLITE_DONE) break;
  }

  // fermeture de la base de données
  sqlite3_close(db);
}

int main(int argc, char *argv[]){

    /*INITIALISATON DES VARAIABLES ET DES STRUCTS*/
    int sockfd, newsockfd, portnum;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, val, id;
    char type[256];
    
    /*Verification des nombres de parametre*/
    if (argc < 2){
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

	/*Ouverture d'un socket*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

	/*ECRITURE DES PARAMETRES RESEAUX*/
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portnum = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);

	/*BINDING du socket*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) error("ERROR on binding");
    listen(sockfd,5); //On ecoute sur le socket; On accepte jusqua 5 clients conenctés
    clilen = sizeof(cli_addr);

    //Ecoute du socket
    while (1){    
	    /*ACCEPTATION D'UNE REQUETE*/
     	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     	if(newsockfd < 0) error("ERROR on accept");

	    /*LECTURE DE CE QUI EST RECU*/
     	bzero(buffer,256); // Clean le buffer
     	n = read(newsockfd,buffer,255);
     	if (n < 0) 
		error("ERROR reading from socket");

	    /*AFFICHAGE DU MESSAGE*/
        printf("\n---------------------------\n");
		sscanf(buffer,"%d %s %d", &id, type, &val);
        printf("Received packet from: %s:%d\nData: %d %s %d\n",inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), id, type, val);

	    /*Répartition des tâches avec un fork*/
	    switch(pid = fork()){
		    case -1:
			    perror("fork");  /* something went wrong */
			    exit(1);		 /* parent exits */

		    case 0:
			    //Envoi de la facture a la base de donnee
			    insertFactures(type, val, id);
			    exit(3);

            default:
                close(newsockfd);
        }
     }
     close(sockfd);
     return 0; 
}
