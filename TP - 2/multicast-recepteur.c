
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

#define MAXBUFFERLEN 1024
#define MAXHOSTLEN 64
#define MAXPORTLEN 6
#define MAXFILELEN 128


int main(int argc, char *argv[]) 
{
   char *multicastAddrString; // First arg: multicast addr (v4 or v6!)
  char *service;             // Second arg: port/service
	
	int ecode;                      // Code retour des fonctions	
	char serverAddr[MAXHOSTLEN];    // Adresse du récepteur
	char serverPort[MAXPORTLEN];    // Port du récepteur
	int descSock;                // Descripteur de socket d’écoute
	struct addrinfo hints;          // Contrôle la fonction getaddrinfo
	struct addrinfo *AdrMulticast;           // Contient le résultat de la fonction getaddrinfo
	struct sockaddr_storage myinfo; // Informations sur la prise locale
	struct sockaddr_storage from;   // Informations sur l’émetteur
	socklen_t len,src_addr_len;                  // Variables utilisées pour stocker les longueurs des structures de socket
	char buffer[MAXBUFFERLEN];      // Tampon de communication entre le client et le serveur
	FILE *f;                        // descripteur de fichier de stockage des données reçues
	char ficRec[MAXFILELEN];
	int fini;
	int nbrec;

if (argc != 3)
    {perror("Paramètre(s) : <Adresse Multicast> <Port>");exit(1);}
multicastAddrString = argv[1];
service = argv[2];

	// Mise à zéro de hints
memset(&hints,0,sizeof(hints));
hints.ai_family=AF_INET;
hints.ai_socktype=SOCK_DGRAM;
hints.ai_protocol=0;
hints.ai_flags|=AI_NUMERICHOST; // Ne pas tenter de résoudre l’adresse
ecode = getaddrinfo(multicastAddrString, service, &hints, &AdrMulticast);
	if (ecode) {
		fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
		exit(1);
	}
 


/* Le paramètre res récupère la liste chaînée de structures addrinfo contenant la ou les adresses résolues */
/* La place mémoire occupée par la liste doit être libérée par appel à freeaddrinfo mais pas avant la création de la prise et de sa publication */

// Initialisation de la socket de communication IPv4/UDP
	descSock = socket(AdrMulticast->ai_family, AdrMulticast->ai_socktype,AdrMulticast->ai_protocol);
	if (descSock == -1) {
		perror("Erreur création socket d’écoute\n");
		exit(2);
	}

	// Publication de la socket au niveau du système
	// Assignation d'une adresse IP et un numéro de port
	ecode = bind(descSock, AdrMulticast->ai_addr, AdrMulticast->ai_addrlen);
	if (ecode == -1) {
		perror("Erreur liaison de la socket d’écoute");
		exit(3);
	}


/* appel à setsockopt() pour demander au noyau de se joindre au groupe multicast  */
	struct ip_mreq joinRequest;
	joinRequest.imr_multiaddr = ((struct sockaddr_in *) AdrMulticast->ai_addr)->sin_addr;
	joinRequest.imr_interface.s_addr = 0; // le système choisira ‘interface
	printf("Adhésion au groupe IPv4 multicast...\n");
    	if (setsockopt(descSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &joinRequest, sizeof(joinRequest)) < 0)
   	{
		perror("setsockopt(IPV4_ADD_MEMBERSHIP) failed");
		exit(4);
	} 
// Nous n'avons plus besoin de cette liste chainée addrinfo
	freeaddrinfo(AdrMulticast);	
	

/* réception de messages et stockage dans le fichier de nom constitué du nom de la machine et du suffixe « .data » */
/* le programme se terminant lorsque la chaine « stop » est reçue */

	ecode=gethostname(ficRec, sizeof(ficRec));
	if (ecode == -1)
	{ 
		perror("Récepteur: gethostname"); 
		exit(5);
	}
	strcat(ficRec,".data");
	f = fopen(ficRec,"a");         // ouverture du fichier en mode création et écriture
	if (f==NULL) {
		perror("Récepteur : fopen");
		exit(6);
	}
	fini=0;
	while (fini==0)
	{
	bzero(buffer,sizeof(buffer));
	src_addr_len=sizeof(from);
	nbrec=recvfrom(descSock,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&from,&src_addr_len);
	if (strncmp(buffer+nbrec-5,"stop",4)==0)fini=1;
	else {printf("chaine reçue %s\n",buffer); fprintf(f,"%s", buffer);}
	}
//Fermeture de la prise : fin de la communication
	close(descSock);
	fclose(f);
}	
	
