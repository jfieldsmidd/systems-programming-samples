/*
 * chat-server.c
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BACKLOG 10
#define BUF_SIZE 4096
#define MSG_BUF_SIZE 128    // 2x NICK_MAX + a good amount for additional message chars
#define NICK_MAX 32
#define IP_MAX 16
#define PORT_MAX 6

struct client_info {
    int fd;
    struct client_info *next;
    struct client_info *prev;
    char client_port[PORT_MAX];
    char client_ip[IP_MAX];
    char nickname[NICK_MAX];
};

// Function declarations
struct client_info *insertClientInfo (struct client_info *prev, struct client_info *next, int fd, struct sockaddr_in *remote_sa);
void removeClientInfo (struct client_info *element);
void *clientThread(void *data);
void sendToAllClients (char *buffer, int length);

// Global vars
struct client_info *list_head;
pthread_mutex_t mutex;

int
main(int argc, char *argv[])
{
    char *listen_port;
    int listen_fd, conn_fd;
    struct addrinfo hints, *res;
    int rc;
    struct sockaddr_in remote_sa;
    uint16_t remote_port;
    socklen_t addrlen;
    char *remote_ip;
    pthread_t new_thread;

    listen_port = argv[1];

    /* create a socket */
    if ((listen_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* bind it to a port */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if((rc = getaddrinfo(NULL, listen_port, &hints, &res)) != 0) {
        printf("getaddrinfo failed: %s\n", gai_strerror(rc));
        exit(2);
    }

    if (bind(listen_fd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("bind");
        exit(3);
    }

    /* start listening */
    ;
    if (listen(listen_fd, BACKLOG) == -1) {
        perror("listen");
        exit(4);
    }

    // initialize linked list for client info - first element doesn't correspond
    // to an actual connection, it's just a placeholder
    list_head = insertClientInfo(NULL, NULL, -1, NULL);
    if (!list_head) {
        perror("insertClientInfo");
        exit(5);
    }

    /* infinite loop of accepting new connections and handling them */
    while(1) {
        /* accept a new connection (will block until one appears) */
        addrlen = sizeof(remote_sa);
        conn_fd = accept(listen_fd, (struct sockaddr *) &remote_sa, &addrlen);
        if (conn_fd == -1) {
            perror("accept");
            exit(6);
        }

        /* announce our communication partner */
        remote_ip = inet_ntoa(remote_sa.sin_addr);
        remote_port = ntohs(remote_sa.sin_port);
        printf("new connection from %s:%d\n", remote_ip, remote_port);

        pthread_mutex_lock(&mutex);
        struct client_info *new_client_info = insertClientInfo(list_head, list_head->next, conn_fd, &remote_sa);
        pthread_mutex_unlock(&mutex);
        if (new_client_info) {
            pthread_create(&new_thread, NULL, clientThread, (void *) new_client_info);
        } else {
            perror("insertClientInfo");
            printf("Unable to add new connection");
        }
    }
}

void *
clientThread(void *data) {
    struct client_info *client = (struct client_info *) data;
    char receive_buf[BUF_SIZE];
    char server_msg_buf[MSG_BUF_SIZE];
    ssize_t bytes_received;

    /* receive and print data until the other end closes the connection */
    while((bytes_received = recv(client->fd, receive_buf, BUF_SIZE, 0)) > 0) {
        // check if client sent nickname change command
        char nick_string[7] = "/nick ";;
        if (strncmp(nick_string, receive_buf, 6) == 0) {
            // handle nickname change command
            int old_nick_len = strlen(client->nickname);
            char old_nick[old_nick_len + 1];
            strncpy(old_nick, client->nickname, old_nick_len + 1);
            int new_nick_len = (bytes_received - 7 <= NICK_MAX - 1) ? (bytes_received - 7) : (NICK_MAX - 1);
            strncpy(client->nickname, receive_buf + 6, NICK_MAX - 1);
            client->nickname[new_nick_len] = '\0';

            // send nickname change message
            snprintf(server_msg_buf, MSG_BUF_SIZE, "User %s is now known as %s.\n", old_nick,
                            client->nickname);
            printf("User %s (%s:%s) is now known as %s.\n", old_nick, client->client_ip,
                            client->client_port, client->nickname);
            pthread_mutex_lock(&mutex);
            sendToAllClients(server_msg_buf, MSG_BUF_SIZE);
            pthread_mutex_unlock(&mutex);
        } else {
            int send_buf_len = strlen(client->nickname) + bytes_received + 3;
            char send_buf[send_buf_len];
            snprintf(send_buf, send_buf_len, "%s: %s", client->nickname, receive_buf);
            // send contents of send_buf to every connection in the linked list
            pthread_mutex_lock(&mutex);
            sendToAllClients(send_buf, send_buf_len);
            pthread_mutex_unlock(&mutex);
        }
    }
    // handle client disconnecting
    printf("Lost connection from %s.\n", client->nickname);
    snprintf(server_msg_buf, MSG_BUF_SIZE, "User %s has disconnected.\n", client->nickname);
    pthread_mutex_lock(&mutex);
    removeClientInfo(client);
    sendToAllClients(server_msg_buf, MSG_BUF_SIZE);
    pthread_mutex_unlock(&mutex);
    if (bytes_received == -1) {
        perror("recv");
    }
    return NULL;
}

// MUST VERIFY SUCCESS OF insertClientInfo: WILL RETURN NULL ON MALLOC FAILURE
struct client_info *
insertClientInfo (struct client_info *prev, struct client_info *next, int fd, struct sockaddr_in *remote_sa) {
    struct client_info *result;
    result = malloc(sizeof(struct client_info));
    if (!result) {
        return NULL;
    }
    result->prev = prev;
    result->next = next;
    result->fd = fd;
    if (remote_sa) {
        sprintf(result->client_port, "%u", ntohs(remote_sa->sin_port));
        strncpy(result->client_ip, inet_ntoa(remote_sa->sin_addr), IP_MAX);
    } else {
        strncpy(result->client_port, "0", PORT_MAX);
        strncpy(result->client_ip, "0", IP_MAX);
    }
    strncpy(result->nickname, "unknown", NICK_MAX);
    if (prev) {
        prev->next = result;
    }
    if (next) {
        next->prev = result;
    }
    return result;
}

void
removeClientInfo (struct client_info *element) {
    if (element->next) {
        struct client_info *next_element = element->next;
        next_element->prev = element->prev;
        element->prev->next = next_element;
    } else {
        element->prev->next = NULL;
    }
    close(element->fd);
    free(element);
}

void
sendToAllClients (char *buffer, int length) {
    struct client_info *current_element;
    current_element = list_head;
    while(current_element->next) {
        if (send(current_element->next->fd, buffer, length, 0) == -1) {
            perror("send");
        }
        current_element = current_element->next;
    }
}
