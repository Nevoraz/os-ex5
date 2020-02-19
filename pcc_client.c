#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
void send_data(int sockfd, char * filePath);

int main(int argc, char *argv[])
{
    int  sockfd     = -1;
    char recv_buff[1024];

    struct sockaddr_in serv_addr; // where we Want to get to
    struct sockaddr_in my_addr;   // where we actually connected through
    struct sockaddr_in peer_addr; // where we actually connected to
    socklen_t addrsize = sizeof(struct sockaddr_in );

    memset(recv_buff, 0,sizeof(recv_buff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        strerror(errno);
        exit(1);
    }

    // print socket details
    getsockname(sockfd,
                (struct sockaddr*) &my_addr,
                &addrsize);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2])); // Note: htons for endiannes
//    inet_aton(argv[1], &serv_addr.sin_addr);
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    // Note: what about the client port number?
    // connect socket to the target address
    if( connect(sockfd,
                (struct sockaddr*) &serv_addr,
                sizeof(serv_addr)) < 0)
    {
        strerror(errno);
        exit(1);
    }
    // print socket details again
    getsockname(sockfd, (struct sockaddr*) &my_addr,   &addrsize);
    getpeername(sockfd, (struct sockaddr*) &peer_addr, &addrsize);
    send_data(sockfd, argv[3]);
    close(sockfd); // is socket really done here?
    return 0;
}


void send_data(int sockfd, char * filePath){
    int N = 0;
    int tmp;
    char C[sizeof(int)];
    char *buffer;
    FILE *fp;
    struct stat st;
    // opening the file
    if ((fp = fopen(filePath,"r")) == NULL){
        strerror(errno);
        exit(1);
    }
    stat(filePath, &st);
    N = (int)st.st_size;
    buffer = malloc(N * sizeof(char));
    sprintf(buffer, "%d", N);
    // sending N, the size of the file
    tmp = write(sockfd, buffer, sizeof(int));
    tmp = fread(buffer, N, 1, fp);
    // sending the data
    tmp = write(sockfd, buffer, N);
    free(buffer);
    fclose(fp);
    //    reading input from server
    tmp = read(sockfd, C, sizeof(int));
    if (tmp){
        tmp = 1;
    }
    printf("# of printable characters: %u\n", atoi(C));
}
