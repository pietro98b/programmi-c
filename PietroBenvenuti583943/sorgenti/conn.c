#include "../intestazioni/conn.h"

int fai_socket(int dominio, int tipo, int protocollo, Sockaddr_un* indirizzo){
    int sock = socket(dominio, tipo, protocollo);
    if(sock == -1){
        perror("socket");
        return -1;
    }
    strcpy(indirizzo->sun_path, NOME_SOCKET);
    indirizzo->sun_family = AF_UNIX;
    return sock;
}

ssize_t  /* Read "n" bytes from a descriptor */
readn(int fd, void *ptr, size_t n) {
   size_t   nleft;
   ssize_t  nread;

   nleft = n;
   while (nleft > 0) {
     if((nread = read(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount read so far */
     } else if (nread == 0) break; /* EOF */
     nleft -= nread;
     ptr   += nread;
   }
   return(n - nleft); /* return >= 0 */
}

ssize_t  /* Write "n" bytes to a descriptor */
writen(int fd, void *ptr, size_t n) {
   size_t   nleft;
   ssize_t  nwritten;

   nleft = n;
   while (nleft > 0) {
     if((nwritten = write(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount written so far */
     } else if (nwritten == 0) break;
     nleft -= nwritten;
     ptr   += nwritten;
   }
   return(n - nleft); /* return >= 0 */
}
