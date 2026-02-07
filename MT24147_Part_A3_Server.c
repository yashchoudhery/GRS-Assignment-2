/*
 * Name: Yash Choudhery
 * Roll No: MT24147
 * Assignment: PA02 - Part A3 (Zero-Copy Server)
 *
 * AI Declaration:
 * I used AI to generate the MSG_ZEROCOPY handling logic.
 * Prompt: "Write a C server function that uses sendmsg with MSG_ZEROCOPY. 
 * Include the necessary setsockopt setup and a helper function to read 
 * the MSG_ERRQUEUE to process completion notifications."
 */

#include "MT24147_Part_A_Common.h"
#include <sys/uio.h>
#include <linux/errqueue.h> // Required for MSG_ZEROCOPY notifications

// Structure to pass arguments to the thread
typedef struct {
    int client_sock;
} thread_args_t;

// Helper: Read the Error Queue to confirm data was sent
// If we don't do this, the kernel will stop accepting new zero-copy sends.
void recv_completion_notifications(int sock) {
    struct msghdr msg = {0};
    char control[100];
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    struct cmsghdr *cm;
    struct sock_extended_err *serr;
    int ret;

    // We keep reading until the queue is empty
    while (1) {
        // MSG_ERRQUEUE tells recvmsg to read from the error queue, not normal data
        ret = recvmsg(sock, &msg, MSG_ERRQUEUE | MSG_DONTWAIT);
        if (ret == -1) {
            // If EAGAIN, queue is empty, we are good.
            break; 
        }

        // Iterate through control messages to find the zero-copy notification
        for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm)) {
            if (cm->cmsg_level == SOL_IP && cm->cmsg_type == IP_RECVERR) {
                serr = (struct sock_extended_err *)CMSG_DATA(cm);
                if (serr->ee_errno == 0 && serr->ee_origin == SO_EE_ORIGIN_ZEROCOPY) {
                    // Successfully processed a zero-copy completion!
                    // In a complex app, we would now free the memory.
                }
            }
        }
    }
}

void *handle_client(void *args) {
    thread_args_t *t_args = (thread_args_t *)args;
    int sock = t_args->client_sock;
    free(t_args);

    // 1. ENABLE ZERO-COPY ON SOCKET
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_ZEROCOPY, &opt, sizeof(opt))) {
        perror("Setsockopt SO_ZEROCOPY failed (kernel might be too old?)");
        close(sock);
        return NULL;
    }

    // 2. Receive message size
    size_t size_per_field;
    if (recv(sock, &size_per_field, sizeof(size_per_field), 0) <= 0) {
        close(sock);
        return NULL;
    }

    // 3. Prepare Data
    Message msg;
    generate_random_message(&msg, size_per_field);

    printf("[Server A3] Starting Zero-Copy transfer. Size per field: %zu bytes.\n", size_per_field);

    // 4. Prepare IO Vector
    struct iovec iov[NUM_FIELDS];
    struct msghdr message_header = {0};

    for(int i = 0; i < NUM_FIELDS; i++) {
        iov[i].iov_base = msg.fields[i];
        iov[i].iov_len = msg.lengths[i];
    }
    message_header.msg_iov = iov;
    message_header.msg_iovlen = NUM_FIELDS;

    // 5. Sending Loop
    while (1) {
        // --- ZERO COPY SEND ---
        // Flag MSG_ZEROCOPY tells kernel: "Don't copy this!"
        ssize_t sent_bytes = sendmsg(sock, &message_header, MSG_ZEROCOPY);

        if (sent_bytes <= 0) {
            break; 
        }

        // --- CRITICAL STEP ---
        // We must periodically clean up the error queue. 
        // If we don't, the kernel's internal counter hits a limit and sendmsg fails.
        recv_completion_notifications(sock);
    }

    // Cleanup
    printf("[Server A3] Client disconnected.\n");
    free_message(&msg);
    close(sock);
    return NULL;
}

int main() {
    signal(SIGPIPE, SIG_IGN); // Prevent crash when client disconnects
    signal(SIGPIPE, SIG_IGN); // Prevent crash when client disconnects
    signal(SIGPIPE, SIG_IGN); // Prevent crash when client disconnects
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach to port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 50) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server (Zero-Copy/A3) listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        pthread_t thread_id;
        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->client_sock = client_fd;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)args) < 0) {
            perror("could not create thread");
            free(args);
            close(client_fd);
        }
        pthread_detach(thread_id);
    }

    return 0;
}