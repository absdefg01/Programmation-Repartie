
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

/*  La réception de messages UDP nécessite de préciser l’adresse IP et le numéro de port sur lesquels les messages sont reçus. 
Ces informations sont définies dans une prise socket représentée par la structure struct sockaddr_in en IPv4. */

#define SERVADDR "0.0.0.0"         // Définition de l'adresse IP d'écoute
#define SERVPORT "0"               // Définition du port d'écoute si 0, port choisi dynamiquement
#define MAXBUFFERLEN 1024
#define MAXHOSTLEN 64
#define MAXPORTLEN 6
#define MAXFILELEN 128


int main(int argc, char** argv){
	
	int ecode;                      // Code retour des fonctions	
	char serverName[128];           // Nom du récepteur
	char serverAddr[MAXHOSTLEN];    // Adresse du récepteur
	char serverPort[MAXPORTLEN];    // Port du récepteur
	int descSock;                // Descripteur de socket d’écoute
	struct addrinfo hints;          // Contrôle la fonction getaddrinfo
	struct addrinfo *res=0;           // Contient le résultat de la fonction getaddrinfo
	struct sockaddr_storage myinfo; // Informations sur la prise locale
	struct sockaddr_storage from;   // Informations sur l’émetteur
	socklen_t len,src_addr_len;                  // Variables utilisées pour stocker les longueurs des structures de socket
	char buffer[MAXBUFFERLEN];      // Tampon de communication entre le client et le serveur
	FILE *f;                        // descripteur de fichier de stockage des données reçues
	char ficRec[MAXFILELEN];
	int fini;
char chaine[256];


/* A priori les communications étant unidirectionnelles, nous n’avons pas besoin de prise locale !!!! */

if (argc != 3){ perror("Mauvaise utilisation de la commande: <nom serveur> <numero de port>\n"); exit(1);}
	if (strlen(argv[1]) >= MAXHOSTLEN){ perror("Le nom de la machine serveur est trop long\n"); exit(2);}
	if (strlen(argv[2]) >= MAXPORTLEN){ perror("Le numero de port du serveur est trop long\n"); exit(2);}
	
	strncpy(serverName, argv[1], MAXHOSTLEN);
	serverName[MAXHOSTLEN-1] = '\0';
	strncpy(serverPort, argv[2], MAXPORTLEN);
	serverPort[MAXPORTLEN-1] = '\0';

/* récupération des informations sur la prise socket distante */
	// Mise à zéro de hints
memset(&hints,0,sizeof(hints));
hints.ai_family=AF_UNSPEC;
hints.ai_socktype=SOCK_DGRAM;
hints.ai_protocol=0;
hints.ai_flags=AI_ADDRCONFIG;
ecode = getaddrinfo(serverName,serverPort,&hints,&res);
	if (ecode) {
		fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
		exit(1);
	}
/* Le paramètre hints contient des précisions sur la conversion du nom en adresse */

/* Le champ address family peut être mis à unspecified permettant de récupérer des adresses IPv4 comme IPv6 */

 
/* Le champ type est mis à SOCK_DGRAM autorisant UDP et excluant TCP  */
/* Le protocole est maintenu à unspecified. Ce champ n’a de sens que lorsqu’on a précisé la famille d’adresses */
/* Si AF_INET ou AF_INET6 avaient été précisés, nous aurions alors mis IPPROTO_UDP ou encore tout simplement 0 (protocole par défaut) */


/* Le drapeau AI_ADDRCONFIG est également positionné afin de retourner les adresses configurées IPv4 comme IPv6 */
/* Le paramètre res récupère la liste chaînée de structures addrinfo contenant la ou les adresses résolues */
/* La place mémoire occupée par la liste doit être libérée par appel à freeaddrinfo mais pas avant la création de la prise et de sa publication */

// Initialisation de la socket de communication IPv4/UDP
	descSock = socket(res->ai_family, res->ai_socktype,res->ai_protocol);
	if (descSock == -1) {
		perror("Erreur création socket distante \n");
		exit(2);
	}

	// Comme la communication est unidirectionnelle et que l’émetteur ne reçoit pas de messages il n’y a pas lieu de publier une prise socket locale au niveau du système
	// Nous n'avons plus besoin de cette liste chainée addrinfo

	freeaddrinfo(res);
	
/* émission des messages » */
/* le programme se terminant lorsque la chaine « stop » est reçue */

ecode=gethostname(ficRec, sizeof(ficRec));
	if (ecode == -1)
	{ 
		perror("Emetteur : gethostname"); 
		exit(5);
	}

fini=0;
while (fini==0)
	{
	bzero(buffer,sizeof(buffer));
	strcpy(buffer,ficRec);
	printf("Entrer une chaine au clavier \n");
	gets(chaine);
	if (strncmp(chaine,"stop",4)==0)fini=1;	
	strcat(buffer,chaine);
	if (sendto(descSock,buffer,strlen(buffer)+1,0,res->ai_addr,res->ai_addrlen)==-1)
	{ error ("Emetteur : sendto");
	exit(6);	
	}
}
//Fermeture de la prise : fin de la communication
	close(descSock);
}
	
