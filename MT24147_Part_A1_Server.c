/*
 * Name: Yash Choudhery
 * Roll No: MT24147
 * Assignment: PA02 - Part A1 (Two-Copy Server)
 *
 * AI Declaration:
 * I used AI to generate the boilerplate for the threaded server loop.
 * Prompt: "Write a multithreaded C server that accepts TCP connections. 
 * Inside the thread, serialize a struct of 8 strings into a single buffer 
 * and send it using send(). Explain the copies."
 */

#include "MT24147_Part_A_Common.h"

// Structure to pass arguments to the thread
typedef struct {
    int client_sock;
} thread_args_t;

void *handle_client(void *args) {
    thread_args_t *t_args = (thread_args_t *)args;
    int sock = t_args->client_sock;
    free(t_args); // Free the argument container

    // 1. Receive the desired message size from the client
    // The client sends a size_t indicating how big each of the 8 strings should be.
    size_t size_per_field;
    if (recv(sock, &size_per_field, sizeof(size_per_field), 0) <= 0) {
        close(sock);
        return NULL;
    }

    // 2. Prepare the Data (Heap Allocation)
    Message msg;
    generate_random_message(&msg, size_per_field);

    // Calculate total size needed for the "Serialized" buffer
    size_t total_size = size_per_field * NUM_FIELDS;
    
    // Allocate the USER-SPACE intermediate buffer
    char *send_buffer = (char *)malloc(total_size);
    if (!send_buffer) {
        perror("Buffer malloc failed");
        free_message(&msg);
        close(sock);
        return NULL;
    }

    printf("[Server] Starting Two-Copy transfer. Size per field: %zu bytes.\n", size_per_field);

    // 3. The Sending Loop
    while (1) {
        // --- COPY 1: USER SPACE TO USER SPACE (Serialization) ---
        // We must flatten the 8 scattered fields into one contiguous buffer.
        size_t offset = 0;
        for (int i = 0; i < NUM_FIELDS; i++) {
            memcpy(send_buffer + offset, msg.fields[i], msg.lengths[i]);
            offset += msg.lengths[i];
        }

        // --- COPY 2: USER SPACE TO KERNEL SPACE ---
        // The send() system call copies data from 'send_buffer' to the OS Socket Buffer.
        ssize_t sent_bytes = send(sock, send_buffer, total_size, 0);

        if (sent_bytes <= 0) {
            // Client disconnected or error
            break; 
        }
    }

    // Cleanup
    printf("[Server] Client disconnected.\n");
    free(send_buffer);
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

    // Forcefully attach socket to the port 8080
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

    printf("Server (Two-Copy) listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // Create a thread for each client
        pthread_t thread_id;
        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->client_sock = client_fd;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)args) < 0) {
            perror("could not create thread");
            free(args);
            close(client_fd);
        }
        
        // Detach thread so resources are released when it finishes
        pthread_detach(thread_id);
    }

    return 0;
}