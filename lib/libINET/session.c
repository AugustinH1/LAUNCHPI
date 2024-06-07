#include "session.h"

/**
 *	\fn			void adr2struct (struct sockaddr_in *addr, char *adrIP, short port)
 *	\brief		Transformer une adresse au format humain en structure SocketBSD
 *	\param		addr : structure d'adressage BSD d'une socket INET
 *	\param		adrIP : adresse IP de la socket créée
 *	\param		port : port de la socket créée
 *	\note		Le domaine dépend du mode choisi (TCP/UDP)
 *	\result		paramètre *adr modifié
 */
void adr2struct (struct sockaddr_in *addr, char *adrIP, short port){
    addr->sin_family = PF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(adrIP);
    memset(&addr->sin_zero, 0, 8);
}

/**
 *	\fn			socket_t creerSocket (int mode)
 *	\brief		Création d'une socket de type DGRAM/STREAM
 *	\param		mode : mode connecté (STREAM) ou non (DGRAM)
 *	\result		socket créée selon le mode choisi
 */
socket_t creerSocket (int mode){
    socket_t sock;
    CHECK(sock.fd = socket(PF_INET, mode, 0), "Can't create");
    sock.mode = mode;
    #ifdef DEBUG
        printf("Socket créée mode=[%d], fd=[%d]\n", mode, sock.fd);
    #endif
    return sock;
}

/**
 *	\fn			socket_t creerSocketAdr (int mode, char *adrIP, short port)
 *	\brief		Création d'une socket de type DGRAM/STREAM
 *	\param		mode : adresse IP de la socket créée
 *	\param		adrIP : adresse IP de la socket créée
 *	\param		port : port de la socket créée
 *	\result		socket créée dans le domaine choisi avec l'adressage fourni
 */
socket_t creerSocketAdr (int mode, char *adrIP, short port){
    socket_t sock = creerSocket(mode);
    adr2struct(&sock.addrLoc, adrIP, port);
    CHECK(bind(sock.fd, (struct sockaddr *)&sock.addrLoc, sizeof(sock.addrLoc)), "can't bind");
    
    socklen_t lenth = sizeof(sock.addrLoc);
    CHECK(getsockname(sock.fd, (struct sockaddr *)&sock.addrLoc, &lenth), "can't get sock name");

    #ifdef DEBUG
        printf("Socket [%d] binde avec ip=[%s] port=[%d]\n",sock.fd, inet_ntoa(sock.addrLoc.sin_addr), ntohs(sock.addrLoc.sin_port));
    #endif
    return sock;
}

/**
 *	\fn			creerSocketEcoute (char *adrIP, short port)
 *	\brief		Création d'une socket d'écoute avec l'adressage fourni en paramètre
 *	\param		adrIP : adresse IP du serveur à mettre en écoute
 *	\param		port : port TCP du serveur à mettre en écoute
 *	\result		socket créée avec l'adressage fourni en paramètre et dans un état d'écoute
 *	\note		Le domaine est nécessairement STREAM
 */
socket_t creerSocketEcoute (char *adrIP, short port){
    
    socket_t sock = creerSocketAdr(SOCK_STREAM, adrIP, port);
    CHECK(listen(sock.fd, 5), "Can't calibrate");
    #ifdef DEBUG
        printf("Socket [%d] mise en écoute ip=[%s] port=[%d]\n",sock.fd ,inet_ntoa(sock.addrLoc.sin_addr), ntohs(sock.addrLoc.sin_port));
    #endif
    return sock;
}

/**
 *	\fn			socket_t accepterClt (const socket_t sockEcoute)
 *	\brief		Acceptation d'une demande de connexion d'un client
 *	\param		sockEcoute : socket d'écoute pour réception de la demande
 *	\result		socket (dialogue) connectée par le serveur avec un client
 */
socket_t accepterClt (const socket_t sockEcoute){
    socket_t sock;
    socklen_t lenth = sizeof(sock.addrDst);
    CHECK(sock.fd = accept(sockEcoute.fd, (struct sockaddr *)&sock.addrDst, &lenth), "Can't accept");
    sock.mode = sockEcoute.mode;
    #ifdef DEBUG
        printf("Appel accepté de ip=[%s] port=[%d]\n", inet_ntoa(sock.addrDst.sin_addr), ntohs(sock.addrDst.sin_port));
    #endif
    return sock;
}
/**
 *	\fn			socket_t connecterClt2Srv (char *adrIP, short port)
 *	\brief		Crétaion d'une socket d'appel et connexion au seveur dont
 *				l'adressage est fourni en paramètre
 *	\param		adrIP : adresse IP du serveur à connecter
 *	\param		port : port TCP du serveur à connecter
 *	\result		socket connectée au serveur fourni en paramètre
 */
socket_t connecterClt2Srv (char *adrIP, short port){
    socket_t sock = creerSocket(SOCK_STREAM);
    adr2struct(&sock.addrDst, adrIP, port);
    CHECK(connect(sock.fd, (struct sockaddr *)&sock.addrDst, sizeof(sock.addrDst)), "can't connect");
    socklen_t lenth = sizeof(sock.addrLoc);
    CHECK(getsockname(sock.fd, (struct sockaddr *)&sock.addrLoc, &lenth), "can't get sock name");
    #ifdef DEBUG
        printf("connexion a ip=[%s] port=[%d] via ip=[%s] port=[%d]\n",
        inet_ntoa(sock.addrDst.sin_addr), ntohs(sock.addrDst.sin_port),
        inet_ntoa(sock.addrLoc.sin_addr), ntohs(sock.addrLoc.sin_port));
    #endif
    return sock;
}
