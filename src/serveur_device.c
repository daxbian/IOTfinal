#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sqlite3.h>

//Ouvre la base de données et insère une mesure sur le bon capteur (unique car contrainte IP + Port)
void insertMesure(char* ip, int port, int valeur){
	int i, s;
  sqlite3 *db;
  char *err_msg;
  sqlite3_stmt *stmt;
  char req[300];
  int id_sensor;

  //Ouvre la base de données
  sqlite3_open("sql/mabdd.db", &db);
  
  //Recherche l'id du capteur
  sprintf(req, "SELECT capteur.ID_CAPTEUR FROM capteur WHERE capteur.port == %d;\n", port); //err ?
  sqlite3_prepare_v2(db, req, -1, &stmt, 0);
  while(sqlite3_step(stmt)!=SQLITE_DONE){
    id_sensor = atoi(sqlite3_column_text(stmt,0));
  }

  sqlite3_reset(stmt);

  //Insère la mesure
  sprintf(req, "INSERT INTO mesure(id_capteur,valeur) VALUES(%d,%d);\n",id_sensor, valeur);
  printf("%s\n",req);
  sqlite3_exec(db, req, 0, 0, &err_msg);
	// lecture dans la base
	// requête SQL select
	sprintf(req,"SELECT * FROM MESURE;");
	printf("%s\n\n",req);

	// préparation de la requête
	sqlite3_prepare_v2(db, req, -1, &stmt, 0);

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

  //Ferme la base de données
  sqlite3_close(db);

  return;
}

//Renvoie un file descriptor disponible
int network_accept_any(int fds[], unsigned int count,struct sockaddr *addr, socklen_t *addrlen){
    fd_set readfds;
    int maxfd, fd;
    unsigned int i;
    int status;

    FD_ZERO(&readfds);

    //On cherche le file descriptor maximum
    maxfd = -1;
    for (i = 0; i < count; i++) {
        FD_SET(fds[i], &readfds);
        if (fds[i] > maxfd)
            maxfd = fds[i];
    }

    //On regarde si une connection a été accepté
    status = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (status < 0)
        return -1;
    fd = -1;

    //On met sélectionne le bon file descriptor
    for (i = 0; i < count; i++)
        if (FD_ISSET(fds[i], &readfds)) {
            fd = fds[i];
            break;
        }
    //On renvoie le accept qu'il faut    
    if (fd == -1)
        return -1;
    else
        return accept(fd, addr, addrlen);
}

//La fonction main
int main(int argc, char **argv) {
    //On lance le serveur qui va vérifier tous les ports stockés dans la table
    printf("Server creating...");
    if(argc!=1){
        fprintf(stderr, "Erreur en entrée. Format: ./device_server\n");
        return 1;
    }

    int socket_desc, client_sock, c;
    char req[300], client_message[1000];
    struct sockaddr_in server, client;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    sqlite3_open("sql/mabdd.db", &db);

    //On récupère tous les ports    
    sprintf(req, "SELECT DISTINCT port FROM capteur;");
    sqlite3_prepare_v2(db, req, -1, &stmt, 0);
    
    int fd[200];
    int j=0;
    //On créé 1 socket par capteurs
    while(sqlite3_step(stmt)!=SQLITE_DONE){

        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_desc == -1) {
            printf("Socket not created\n");
        }

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;

        int serverPort;
        serverPort = atoi(sqlite3_column_text(stmt, 0));
        server.sin_port = htons(serverPort);

        if(bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
            fprintf(stderr, "Bind failed\n");
            return 1;
        }
        
        listen(socket_desc, 3);

        fd[j] = socket_desc;
        j+=1;
    }

    sqlite3_reset(stmt);
    sqlite3_close(db);

    while(1){
        c = sizeof(struct sockaddr_in);

        //On accepte les connections de tous les ports
        client_sock = network_accept_any(fd, 2,(struct sockaddr*)&client,(socklen_t*)&c);
        if(client_sock < 0) {
            fprintf(stderr,"accept failed\n");
            return 1;
        }

        printf("New connection\n");
        //On lit la requête du client
        if(read(client_sock, client_message, 1000)<0){
            fprintf(stderr,"read failed\n");
            return 1;
        }
        printf("Received: %s\n", client_message);
        
		char method[10];
        sscanf(client_message, "%s", method);
        if(strncmp("PUT", method, 4)==0) {
            if(write(client_sock,"PUT\n", 3)<0){
                fprintf(stderr, "write failed\n");
                return 1;
            }
            char ip[40];
            int port;
            int valeur;

            //On analyse la chaine et on insère la valeur
            sscanf(client_message, "%s %s %d %d",method, ip,&port,&valeur);
            insertMesure(ip, port, valeur);
		}
		else {
            //On a reçu un mauvais message de la part du client, on ferme le serveur
            if(write(client_sock,"ERROR\n", 3)<0){
                fprintf(stderr, "write failed\n");
                return 1;
            }
            printf("Error\n");
            return 1;
        }
    }

    return 0;
}
