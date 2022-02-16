/*
 * chat-client.c
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define BUF_SIZE 4096
#define TIME_STRING_SIZE 9

void *receiveFromServer(void *data);

int conn_fd;

int main(int argc, char *argv[])
{
    char *dest_hostname;
    char *dest_port;
    struct addrinfo hints;
    struct addrinfo *res;
    char send_buf[BUF_SIZE];
    int n;
    int rc;
    pthread_t receiving_thread;

    dest_hostname = argv[1];
    dest_port     = argv[2];

    /* create a socket */
    if ((conn_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
    }

    /* get the IP address of the server */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if((rc = getaddrinfo(dest_hostname, dest_port, &hints, &res)) != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(rc));
        exit(2);
    }

    /* connect to the server */
    if(connect(conn_fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        exit(3);
    }

    printf("Connected\n");

    pthread_create(&receiving_thread, NULL, receiveFromServer, NULL);

    // infinite loop of reading from terminal and sending the data
    while((n = read(0, send_buf, BUF_SIZE)) > 0) {
        if(send(conn_fd, send_buf, n, 0) == -1) {
            perror("send");
            printf("Failed to send message.");
            exit(4);
        }
    }
    close(conn_fd);
    if (n == -1) {
        perror("read");
        exit(5);
    }
    printf("Exiting.\n");
    exit(0);
}

void *
receiveFromServer(void *data) {
    char receive_buf[BUF_SIZE];
    ssize_t bytes_received;
    /* receive and print data until the other end closes the connection */
    while((bytes_received = recv(conn_fd, receive_buf, BUF_SIZE, 0)) > 0) {
        time_t current_time;
        time(&current_time);
        char timeString[TIME_STRING_SIZE];
        strftime(timeString, TIME_STRING_SIZE, "%H:%M:%S", localtime(&current_time));
        printf("%s: %s", timeString, receive_buf);
    }
    if (bytes_received == -1) {
        perror("recv");
        exit(6);
    }
    printf("Connection closed by remote host.\n");
    exit(0);
    return NULL;
}
