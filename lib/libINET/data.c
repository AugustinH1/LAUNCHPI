#include "data.h"


void envoyerMessSTREAM(socket_t *sockEch, char *buffer){
    CHECK(send(sockEch->fd, buffer, strlen(buffer)+1, MSG_OOB), "Can't send");
    //CHECK(write(sockEch->fd, buffer, strlen(buffer)+1), "Can't send");
    
}

void envoyerMessDGRAM(socket_t *sockEch, buffer_t buffer, char *ip, int port){
    struct sockaddr_in addr;
    adr2struct(&addr, ip, port);

    CHECK(sendto(sockEch->fd, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&addr, sizeof(addr)), "Can't send");
}

/**
 *	\fn			void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...)
 *	\brief		Envoi d'une requête/réponse sur une socket
 *	\param 		sockEch : socket d'échange à utiliser pour l'envoi
 *	\param 		quoi : requête/réponse à serialiser avant l'envoi
 *	\param 		serial : pointeur sur la fonction de serialisation d'une requête/réponse
 *	\note		si le paramètre serial vaut NULL alors quoi est une chaîne de caractères
 *	\note		Si le mode est DGRAM, l'appel nécessite en plus l'adresse IP et le port.
 *	\result		paramètre sockEch modifié pour le mode DGRAM
 */
void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...){
    buffer_t buff;
    
    if(serial != NULL)
        serial(quoi, buff);
    else
        strcpy(buff,(char *) quoi);

    #ifdef DEBUG
        printf("Requête à envoyer: %s\n", buff);
    #endif

    if(sockEch->mode == SOCK_STREAM)
        envoyerMessSTREAM(sockEch, buff);
    
    if(sockEch->mode == SOCK_DGRAM){
        va_list pArg;
        va_start(pArg, serial);
        char *adrIp = va_arg(pArg, char *);
        int port = va_arg(pArg, int);
        envoyerMessDGRAM(sockEch, buff, adrIp, port);
        va_end(pArg);
    }
}

/**
 *	\fn			void recevoir(socket_t *sockEch, generic quoi, pFct deSerial)
 *	\brief		Réception d'une requête/réponse sur une socket
 *	\param 		sockEch : socket d'échange à utiliser pour la réception
 *	\param 		quoi : requête/réponse reçue après dé-serialisation du buffer de réception
 *	\param 		deSerial : pointeur sur la fonction de dé-serialisation d'une requête/réponse
 *	\note		si le paramètre deSerial vaut NULL alors quoi est une chaîne de caractères
 *	\result		paramètre quoi modifié avec le requête/réponse reçue
 *				paramètre sockEch modifié pour le mode DGRAM
 */
void recevoir(socket_t *sockEch, generic quoi, pFct deSerial){
    buffer_t buffer;

    if(sockEch->mode == SOCK_STREAM){
        CHECK(read(sockEch->fd, buffer, sizeof(buffer)), "Can't read");
    }
    
    if(sockEch->mode == SOCK_DGRAM){
        printf("Sock_Dgram\n");
        socklen_t lenth = sizeof(sockEch->addrDst);
        CHECK(recvfrom(sockEch->fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sockEch->addrDst, &lenth), "Can't read");
    }

    if(deSerial == NULL)
            strcpy(quoi, buffer);
        else
            deSerial(buffer, quoi);

    #ifdef DEBUG
        printf("Requête reçue a traiter: %s\n", buffer);
    #endif

}