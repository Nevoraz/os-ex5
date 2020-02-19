#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>

int counts[127];
int counts_tmp[127];
int sigintFlag = 0; // if SIGINT was recived so it equals 1

void read_data(int *counts, int connfd);
int count_data(int *counts, char *buffer, int N);
void print_counts(int* counts);
void combine_counts(int *counts, int *counts_tmp);

void sigint_handler_flag(int sig){
    sigintFlag = 1;
}

void sigint_handler_exit(int sig){
    print_counts(counts);
    exit(0);
}

void general_handler(int sig){
    strerror(errno);
}

int main(int argc, char *argv[])
{
    int listenfd  = -1;
    int connfd    = -1;
    int i;
    int enable = 1;
    struct sockaddr_in serv_addr;
    struct sockaddr_in my_addr;
    struct sockaddr_in peer_addr;
    socklen_t addrsize = sizeof(struct sockaddr_in );
    signal(SIGINT, sigint_handler_exit);
    signal(ETIMEDOUT, general_handler);
    signal(ECONNRESET, general_handler);
    signal(EPIPE, general_handler);
    for (i = 0 ; i < 127 ; i++ ){
        counts[i] = 0;
        counts_tmp[i] = 0;
    }
    listenfd = socket( AF_INET, SOCK_STREAM, 0 );
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    memset( &serv_addr, 0, addrsize );
    serv_addr.sin_family = AF_INET;
    // INADDR_ANY = any local machine address
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if( 0 != bind( listenfd,
                  (struct sockaddr*) &serv_addr,
                  addrsize ) ){
        strerror(errno);
        exit(1);
    }
    if( 0 != listen( listenfd, 10 ) ){
        strerror(errno);
        exit(1);
    }
    while( 1 ){
        // Accept a connection.
        // Can use NULL in 2nd and 3rd arguments
        // but we want to print the client socket details
        connfd = accept( listenfd,
                        (struct sockaddr*) &peer_addr,
                        &addrsize);
        signal(SIGINT, sigint_handler_flag);
        if( connfd < 0 ){
            strerror(errno);
            exit(1);
        }
        getsockname(connfd, (struct sockaddr*) &my_addr,   &addrsize);
        getpeername(connfd, (struct sockaddr*) &peer_addr, &addrsize);
        read_data(counts_tmp, connfd);
        close(connfd);
        combine_counts(counts, counts_tmp);
        if (sigintFlag == 1){
            print_counts(counts);
            exit(0);
        }
        signal(SIGINT, sigint_handler_exit);
    }
}

void read_data(int *counts_tmp, int connfd){
    int C;
    char N[sizeof(int)];
    char *buffer;
    int tmp;
    // read the message from client and copy it in buffer
    tmp = read(connfd, N, sizeof(int));
    buffer = malloc(atoi(N) * sizeof(char));
    tmp = read(connfd, buffer, atoi(N));
    C = count_data(counts, buffer, atoi(N));
    // and send that buffer to client
    sprintf(buffer, "%d", C);
    tmp = write(connfd, buffer, sizeof(int));
    free(buffer);
}

int count_data(int *counts, char *buffer, int N){
    int i;
    int result = 0;
    for (i = 0 ; i < N ; i++ ){
        if ((int)buffer[i] >= 32 && (int)buffer[i] <= 126){
            result++;
            counts[(int)buffer[i]]++;
        }
    }
    return result;
}

void print_counts(int* counts){
    int i;
    char c;
    for (i = 32 ; i <= 126 ; i++ ){
        c = i;
        printf("char ’%c’ : %u times\n", c, counts[i]);
    }
}


void combine_counts(int *counts, int *counts_tmp){
    int i;
    for (i = 0 ; i < 126 ; i++ )
        counts[i] = counts[i] + counts_tmp[i];
}
