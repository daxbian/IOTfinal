#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sqlite3.h>


char webpage1[512] = "HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Test Title</title>\r\n"
"<body>Consommation: En construction</body>"
"</html>";

char webpage2[512] = "HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Test Title</title>\r\n"
"<body>Etat capteurs: En construction</body>"
"</html>";

char webpage3[512] = "HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Test Title</title>\r\n"
"<body>Configuration: En construction</body>"
"</html>";

int eau = 0;
int gaz = 0;
int electricite = 0;

int eauArray[256];
int gazArray[256];
int electriciteArray[256];

int iterEau = 0;
int iterGaz = 0;
int iterElec = 0;

void insertCapteur(char* buf){
	int piece, port;
	char ref[20], type[20];
	sqlite3 *db;
	char *err_msg;
	sqlite3_stmt *stmt;
	char req[200];

	//Ouvre la base de données
	sqlite3_open("sql/mabdd.db", &db);
 	//create_id_piece=2&create_type=elec&capt_ref=ESP82&create_port=3030
	char *temp = strstr(buf,"create_id_piece=");
	sscanf(temp, "create_id_piece=%d&create_type=%[^&]&capt_ref=%[^&]&create_port=%d", &piece, type, ref, &port);
	//Insère le capteur
	sprintf(req, "INSERT INTO capteur(id_piece,type,reference,port) VALUES(%d,\"%s\",\"%s\",%d);\n", piece, type, ref, port);
	printf("%s", req);
	sqlite3_exec(db, req, 0, 0, &err_msg);
    printf("err_msg:|%s|\n\n",err_msg);

	//Ferme la base de données
	sqlite3_close(db);
}

void deleteCapteur(char* buf){
	int id;
	sqlite3 *db;
	char *err_msg;
	sqlite3_stmt *stmt;
	char req[200];

	//Ouvre la base de données
	sqlite3_open("sql/mabdd.db", &db);
 	//capt=5%2C+2%2C+%27gaz%27%2C+%27ESP8266%27%2C+12345
	char *temp = strstr(buf, "capt=");
	sscanf(temp, "capt=%d", &id);
	//Insère le capteur
	sprintf(req, "DELETE FROM capteur WHERE id_capteur = '%d';", id);
	printf("Requete:\n%s", req);
	sqlite3_exec(db, req, 0, 0, &err_msg);
    printf("err_msg:|%s|\n\n",err_msg);

	//Ferme la base de données
	sqlite3_close(db);
}

/*
void calcul_facture(){
    //Factures
    eau = gaz = electricite = 0;

    //BDD
    sqlite3 *db;
    char *err_msg;
    sqlite3_stmt *stmt;
    char req[300];
    char type[20];
    int valeur; 
    //Ouvre la base de données
    sqlite3_open("sql/mabdd.db", &db);
    
    //On recherche l'ensemble des factures dans la BDD
    sprintf(req, "SELECT facture.type, facture.valeur FROM facture;");
    printf("%s\n",req);
    sqlite3_prepare_v2(db, req, -1, &stmt, 0);
    while(sqlite3_step(stmt)!=SQLITE_DONE){
        //Récupération des types et montants
        strcpy(type, sqlite3_column_text(stmt,0));
        valeur = atoi(sqlite3_column_text(stmt,1));

        //Mise à jour des valeurs
        if(strncmp(type, "gaz", 3) == 0) gaz += valeur;
        else if(strncmp(type,"eau", 3) == 0) eau += valeur;		
        else if(strncmp(type,"electricite", 4) == 0) electricite += valeur;		
    }
    printf("Gaz: %d, Eau: %d, Electricite: %d\n", gaz, eau, electricite);

    //Ferme la base de données
    sqlite3_reset(stmt);
    sqlite3_close(db);
}
*/
void calcul_facture(){
    //Factures
    eau = gaz = electricite = 0;
	iterEau = 0;
	iterGaz = 0;
	iterElec = 0;

	//BDD
    sqlite3 *db;
    char *err_msg;
    sqlite3_stmt *stmt;
    char req[300];
    char type[20];
    int valeur; 
    //Ouvre la base de données
    sqlite3_open("sql/mabdd.db", &db);
    
    //On recherche l'ensemble des factures dans la BDD
    sprintf(req, "SELECT facture.type, facture.valeur FROM facture;");
    printf("%s\n",req);
    sqlite3_prepare_v2(db, req, -1, &stmt, 0);
    while(sqlite3_step(stmt)!=SQLITE_DONE){
        //Récupération des types et montants
        strcpy(type, sqlite3_column_text(stmt,0));
        valeur = atoi(sqlite3_column_text(stmt,1));

        //Mise à jour des valeurs
        if(strncmp(type, "gaz", 3) == 0){
			gaz += valeur;
			gazArray[iterGaz] = valeur;
			iterGaz += 1;
		}
        else if(strncmp(type,"eau", 3) == 0){
			eau += valeur;
			eauArray[iterEau] = valeur;
			iterEau += 1;		
		}	
        else if(strncmp(type,"electricite", 4) == 0){
			electricite += valeur;
			electriciteArray[iterElec] = valeur;
			iterElec += 1;	
		}	
    }
    printf("Gaz: %d, Eau: %d, Electricite: %d\n", gaz, eau, electricite);

    //Ferme la base de données
    sqlite3_reset(stmt);
    sqlite3_close(db);

}
int main(int argc, char *argv[]){
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);
    int fd_server, fd_client;
    /* Storing the contents sent by the browser (a request) */
    char buf[2048];
    int on = 1;

    fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_server < 0){
        perror("socket");
        exit(1);
    }

    setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        close(fd_server);
        exit(1);
    }

    if(listen(fd_server, 10) == -1){
        perror("listen");
        close(fd_server);
        exit(1);
    }

    while(1){
        fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

        if(fd_client == -1){
            perror("Connection failed...\n");
            continue;
        }

        printf("Got client connection...\n");
        if(!fork()){
            /* Child process */
            /* Close this as the client no longer needs it */
            close(fd_server);
            memset(buf, 0, 2048);
            read(fd_client, buf, 2047); /* 2047 because of null char? */

            /* Print the request on the console */
            printf("%s\n", buf);

            //Demande de chargement piechart
            if(!strncmp(buf, "GET /pieChart", 13)){
                //Re-calcule les factures
                printf("Calcul des factures...\n");
                calcul_facture();

                //Permet l'interpretation du code dans le navigateur
                sprintf(buf, "HTTP/1.1 200 OK\nConnection:close\nContent-Type: text/html\n\n");
                write(fd_client, buf, strlen(buf));
                
                //Ouverture et lecture du fichier HTML
                FILE *f = fopen("site/pieChart.html", "r");
                char file_buf[4096];
                fread(file_buf, sizeof(char), sizeof(file_buf), f);
                fclose(f);
                
                //Remplacer par les données des mesures du capteur dans la balise {{data}} du code HTML
                char *startData = strstr(file_buf,"{{data}}");
                char htmlToSend[4096];
                char montantDisplay[100];
       			memset(htmlToSend, 0, 4096);
           		memset(montantDisplay, 0, 100);
                strcpy(htmlToSend,"\0");
                strncat(htmlToSend, file_buf, startData - file_buf);
                strcpy(montantDisplay,"\0");
                sprintf(montantDisplay, "[\"Electricite\",%d], [\"Eau\",%d], [\"Gaz\",%d]", electricite, eau, gaz);
                strcat(htmlToSend, montantDisplay);
                strcat(htmlToSend, startData + strlen("{{data}}"));
                //Envoyer le pieChart au client
                write(fd_client, htmlToSend, sizeof(htmlToSend) - 1);
            }

			//**********************************************************************************************************//
			//**********************************************************************************************************//
			//**********************************************************************************************************//

            else if(!strncmp(buf, "GET /conso", 10)){

                //Re-calcule les factures
                printf("Calcul des factures...\n");
                calcul_facture();
                //Permet l'interpretation du code dans le navigateur
                sprintf(buf, "HTTP/1.1 200 OK\nConnection:close\nContent-Type: text/html\n\n");
                write(fd_client, buf, strlen(buf));
                
                //Ouverture et lecture du fichier HTML
                FILE *f = fopen("site/areaChart.html", "r");
                char file_buf[4096];
            	memset(file_buf, 0, 4096);	
                fread(file_buf, sizeof(char), sizeof(file_buf), f);
                fclose(f);

                //Remplacer par les données des mesures du capteur dans la balise {{data}} du code HTML
				//Partie Elec
                char *startData = strstr(file_buf,"{{data}}");
                char htmlToSend[4096];
                char montantDisplay[4096];
				char montantTemp[100];
       			memset(htmlToSend, 0, 4096);
        	   	memset(montantDisplay, 0, 4096);
        	   	memset(montantTemp, 0, 100);
                strcpy(htmlToSend,"\0");
                strncat(htmlToSend, file_buf, startData - file_buf);
                strcpy(montantDisplay,"\0");
				for(int i=0; i<iterElec;i++){
					if(i==(iterElec-1)){
               			sprintf(montantTemp, "[%d,%d]", i,electriciteArray[i]);
					}
					else{
               			sprintf(montantTemp, "[%d,%d],", i,electriciteArray[i]);					
					}
					strcat(montantDisplay, montantTemp);
				}
                strcat(htmlToSend, montantDisplay);
                strcat(htmlToSend, startData + strlen("{{data}}"));
				
				//Partie Gaz========================================================================
				strcpy(file_buf,htmlToSend);
				startData = strstr(file_buf,"{{data2}}");
       			memset(htmlToSend, 0, 4096);
        	   	memset(montantDisplay, 0, 4096);
        	   	memset(montantTemp, 0, 100);
				strcpy(htmlToSend,"\0");
                strncat(htmlToSend, file_buf, startData - file_buf);
                strcpy(montantDisplay,"\0");
				for(int i=0; i<iterGaz;i++){
					if(i==(iterGaz-1)){
               			sprintf(montantTemp, "[%d,%d]", i,gazArray[i]);
					}
					else{
               			sprintf(montantTemp, "[%d,%d],", i,gazArray[i]);					
					}
					strcat(montantDisplay, montantTemp);
				}
                strcat(htmlToSend, montantDisplay);
                strcat(htmlToSend, startData + strlen("{{data2}}"));
				
				//Partie Eau======================================================================
				strcpy(file_buf,htmlToSend);
				startData = strstr(file_buf,"{{data3}}");
       			memset(htmlToSend, 0, 4096);
        	   	memset(montantDisplay, 0, 4096);
        	   	memset(montantTemp, 0, 100);
				strcpy(htmlToSend,"\0");
                strncat(htmlToSend, file_buf, startData - file_buf);
                strcpy(montantDisplay,"\0");
				for(int i=0; i<iterEau;i++){
					if(i==(iterEau-1)){
               			sprintf(montantTemp, "[%d,%d]", i,eauArray[i]);
					}
					else{
               			sprintf(montantTemp, "[%d,%d],", i,eauArray[i]);					
					}
					strcat(montantDisplay, montantTemp);
				}
                strcat(htmlToSend, montantDisplay);
                strcat(htmlToSend, startData + strlen("{{data3}}"));
				
                //Envoyer le pieChart au client
                write(fd_client, htmlToSend, sizeof(htmlToSend) - 1);
            }

			//**********************************************************************************************************//
			//**********************************************************************************************************//
			//**********************************************************************************************************//

            else if(!strncmp(buf, "GET /etatCapteurs", 17)){
                //write(fd_client, webpage2, sizeof(webpage2) - 1);
                //Permet l'interpretation du code dans le navigateur
                sprintf(buf, "HTTP/1.1 200 OK\nConnection:close\nContent-Type: text/html\n\n");
                write(fd_client, buf, strlen(buf));

                //Ouverture et lecture du fichier HTML
                FILE *f = fopen("site/etatCapteurs.html", "r");
                char file_buf[4096];
                fread(file_buf, sizeof(char), sizeof(file_buf), f);
                fclose(f);
                
                //Remplacer par les données des mesures du capteur dans la balise {{data}} du code HTML
                char *startData = strstr(file_buf,"{{data}}");
                char htmlToSend[4096];
                char montantDisplay[4096], temp[4096];
                strcpy(htmlToSend,"\0");
                strncat(htmlToSend, file_buf, startData - file_buf);
                strcpy(montantDisplay,"\0");

                //Complétion des données avec la BDD                
                printf("Récupération des données capteurs...\n");
                sqlite3 *db;
                char *err_msg;
                sqlite3_stmt *stmt;
                char req[300];
                char type[50], ref[50];
                int idCapteur, idPiece, port; 
                int r = 0; //Permet de pas mettre de virgule à la premiere iteration
                
                //On recherche l'ensemble des capteurs dans la BDD
                sqlite3_open("sql/mabdd.db", &db);
                sprintf(req, "SELECT capteur.ID_CAPTEUR, capteur.id_piece, capteur.type, capteur.reference, capteur.port FROM capteur;");
                printf("%s\n",req);
                sqlite3_prepare_v2(db, req, -1, &stmt, 0);
                while(sqlite3_step(stmt)!=SQLITE_DONE){
                    //Récupération des types et montants
                    idCapteur = atoi(sqlite3_column_text(stmt,0));
                    idPiece = atoi(sqlite3_column_text(stmt,1));
                    strcpy(type, sqlite3_column_text(stmt,2));
                    strcpy(ref, sqlite3_column_text(stmt,3));
                    port = atoi(sqlite3_column_text(stmt,4));
                    if(r == 0){
                        sprintf(montantDisplay, "[%d, %d, '%s', '%s', %d]", idCapteur, idPiece, type, ref, port);
                        r = 1;
                    }
                    else{
                        sprintf(temp, ",\n\t[%d, %d, '%s', '%s', %d]", idCapteur, idPiece, type, ref, port);
                        strcat(montantDisplay, temp);
                    }
                }
                //printf("data extracted:%s\n", montantDisplay);
                //Ferme la base de données
                sqlite3_reset(stmt);
                sqlite3_close(db);

                //Envoyer le tableau au client
                strcat(htmlToSend, montantDisplay);
                strcat(htmlToSend, startData + strlen("{{data}}"));
                write(fd_client, htmlToSend, sizeof(htmlToSend) - 1);
            }

			//**********************************************************************************************************//
			//**********************************************************************************************************//
			//**********************************************************************************************************//

            else if(!strncmp(buf, "GET /config", 11)){
                sprintf(buf, "HTTP/1.1 200 OK\nConnection:close\nContent-Type: text/html\n\n");
                write(fd_client, buf, strlen(buf));

                //Ouverture et lecture du fichier HTML
                FILE *f = fopen("site/config.html", "r");
                char file_buf[4096];
                fread(file_buf, sizeof(char), sizeof(file_buf), f);
                fclose(f);
                
                //Remplacer par les données des mesures du capteur dans la balise {{data}} du code HTML
                char *startData = strstr(file_buf,"{{choices}}");
                char htmlToSend[4096];
                char montantDisplay[4096], temp[4096];
                strcpy(htmlToSend,"\0");
                strncat(htmlToSend, file_buf, startData - file_buf);
                strcpy(montantDisplay,"\0");

                //Complétion des données avec la BDD                
                printf("Récupération des données capteurs...\n");
                sqlite3 *db;
                char *err_msg;
                sqlite3_stmt *stmt;
                char req[300];
                char type[50], ref[50];
                int idCapteur, idPiece, port; 
                
                //On recherche l'ensemble des capteurs dans la BDD
                sqlite3_open("sql/mabdd.db", &db);
                sprintf(req, "SELECT capteur.ID_CAPTEUR, capteur.id_piece, capteur.type, capteur.reference, capteur.port FROM capteur;");
                printf("%s\n",req);
                sqlite3_prepare_v2(db, req, -1, &stmt, 0);
                while(sqlite3_step(stmt)!=SQLITE_DONE){
                    //Récupération des types et montants
                    idCapteur = atoi(sqlite3_column_text(stmt,0));
                    idPiece = atoi(sqlite3_column_text(stmt,1));
                    strcpy(type, sqlite3_column_text(stmt,2));
                    strcpy(ref, sqlite3_column_text(stmt,3));
                    port = atoi(sqlite3_column_text(stmt,4));
                    sprintf(temp, "<OPTION>%d, %d, '%s', '%s', %d",idCapteur, idPiece, type, ref, port);
                    strcat(montantDisplay, temp);
                }
                //printf("data extracted:%s\n", montantDisplay);
                //Ferme la base de données
                sqlite3_reset(stmt);
                sqlite3_close(db);

                //Envoyer le tableau au client
                strcat(htmlToSend, montantDisplay);
                strcat(htmlToSend, startData + strlen("{{choices}}"));
                write(fd_client, htmlToSend, sizeof(htmlToSend) - 1);

            }

			else if(!strncmp(buf, "POST /action_pageAdd", 20)){
				char htmlToSend[4096];
       			memset(htmlToSend, 0, 4096);
 				sprintf(htmlToSend, "HTTP/1.1 200 OK\nConnection:close\nContent-Type: text/html\n\n");
                write(fd_client, htmlToSend, strlen(htmlToSend));
				strcpy(htmlToSend, "<p>La requete POST envoyee est :<br></p>");
				strcat(htmlToSend, buf);
                write(fd_client, htmlToSend, strlen(htmlToSend));
				insertCapteur(buf);	
			}
			else if(!strncmp(buf, "POST /action_pageSuppr", 20)){
				char htmlToSend[4096];
       			memset(htmlToSend, 0, 4096);
 				sprintf(htmlToSend, "HTTP/1.1 200 OK\nConnection:close\nContent-Type: text/html\n\n");
                write(fd_client, htmlToSend, strlen(htmlToSend));
				strcpy(htmlToSend, "<p>La requete POST envoyee est :<br></p>");
				strcat(htmlToSend, buf);
                write(fd_client, htmlToSend, strlen(htmlToSend));
				deleteCapteur(buf);	
			}

            close(fd_client);
            printf("closing connection...\n");
            exit(0);
        }
        /* Parent process */
        close(fd_client);
    }   
    return 0;
}


