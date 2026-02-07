/*
 * Name: Yash Choudhery
 * Roll No: MT24147
 * Assignment: PA02 - Part A2 (One-Copy Server)
 *
 * AI Declaration:
 * I used AI to generate the sendmsg/iovec implementation.
 * Prompt: "Write a C server that uses sendmsg and struct iovec to send 
 * 8 scattered buffers without combining them first (One-Copy)."
 */

#include "MT24147_Part_A_Common.h"
#include <sys/uio.h> // Required for struct iovec

// Structure to pass arguments to the thread
typedef struct {
    int client_sock;
} thread_args_t;

void *handle_client(void *args) {
    thread_args_t *t_args = (thread_args_t *)args;
    int sock = t_args->client_sock;
    free(t_args);

    // 1. Receive the desired message size
    size_t size_per_field;
    if (recv(sock, &size_per_field, sizeof(size_per_field), 0) <= 0) {
        close(sock);
        return NULL;
    }

    // 2. Prepare the Data (Heap Allocation)
    Message msg;
    generate_random_message(&msg, size_per_field);

    printf("[Server A2] Starting One-Copy transfer. Size per field: %zu bytes.\n", size_per_field);

    // 3. Prepare the IO Vector (The "Shopping List" for the Kernel)
    struct iovec iov[NUM_FIELDS];
    struct msghdr message_header = {0};

    // We point the IO Vector entries to our existing scattered strings
    // No new buffer allocation needed!
    for(int i = 0; i < NUM_FIELDS; i++) {
        iov[i].iov_base = msg.fields[i]; // Point to the data
        iov[i].iov_len = msg.lengths[i]; // Say how long it is
    }

    // Setup the message header to point to our IO Vector
    message_header.msg_iov = iov;
    message_header.msg_iovlen = NUM_FIELDS;

    // 4. The Sending Loop
    while (1) {
        // --- ONE COPY OPERATION ---
        // sendmsg() reads directly from the 8 locations in 'iov' 
        // and copies them into the Kernel Socket Buffer.
        // The "User-to-User" copy is completely skipped.
        
        ssize_t sent_bytes = sendmsg(sock, &message_header, 0);

        if (sent_bytes <= 0) {
            break; 
        }
    }

    // Cleanup
    printf("[Server A2] Client disconnected.\n");
    free_message(&msg);
    close(sock);
    return NULL;
}

int main() {
    signal(SIGPIPE, SIG_IGN); // Prevent crash when client disconnects
    signal(SIGPIPE, SIG_IGN); // Prevent crash when client disconnects
    signal(SIGPIPE, SIG_IGN); // Prevent crash when client disconnects
    int server_fd, *new_sock;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to port 8080
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

    printf("Server (One-Copy/A2) listening on port %d\n", PORT);

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