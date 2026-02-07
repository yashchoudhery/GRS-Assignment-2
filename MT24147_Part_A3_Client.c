// /*
//  * Name: Yash Choudhery
//  * Roll No: MT24147
//  * Assignment: PA02 - Part A3 (Zero-Copy Client)
//  *
//  * AI Declaration:
//  * I used AI to generate the client-side threading logic and argument parsing.
//  * Prompt: "Write a multithreaded C client that connects to a server IP/Port.
//  * Each thread receives data for a fixed duration and calculates throughput.
//  * Use command line arguments for IP, Port, Threads, Message Size, and Duration."
//  *
//  * Note: The client logic for A3 is functionally identical to A1/A2. 
//  * The Zero-Copy optimization is a sender-side (Server) kernel feature.
//  * The Client simply consumes the data stream.
//  */



#include "MT24147_Part_A_Common.h"
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    char *server_ip;
    int port;
    size_t message_size;
    int duration;
    unsigned long long bytes_received;
} client_thread_args_t;

// Helper to get current time in seconds
double get_time_sec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

void *client_thread_func(void *arg) {
    client_thread_args_t *args = (client_thread_args_t *)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *recv_buf = malloc(65536); 
    
    if (!recv_buf) return NULL;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        free(recv_buf);
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(args->port);

    if (inet_pton(AF_INET, args->server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        free(recv_buf);
        return NULL;
    }

    // --- RETRY LOGIC (Fixes Connection Refused) ---
    int connected = 0;
    for (int retries = 0; retries < 10; retries++) {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
            connected = 1;
            break;
        }
        sleep(1); // Wait 1 sec before retrying
    }

    if (!connected) {
        close(sock);
        free(recv_buf);
        return NULL;
    }

    // Handshake
    size_t size_req = args->message_size;
    if (send(sock, &size_req, sizeof(size_req), 0) < 0) {
        close(sock);
        free(recv_buf);
        return NULL;
    }

    // Receive Loop
    double end_time = get_time_sec() + args->duration;
    unsigned long long total_bytes = 0;
    
    while (get_time_sec() < end_time) {
        ssize_t valread = recv(sock, recv_buf, 65536, 0);
        if (valread <= 0) break; 
        total_bytes += valread;
    }

    args->bytes_received = total_bytes;
    free(recv_buf);
    close(sock);
    return NULL;
}

int main(int argc, char const *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <IP> <Port> <Threads> <Size> <Duration>\n", argv[0]);
        return -1;
    }

    char *server_ip = (char *)argv[1];
    int port = atoi(argv[2]);
    int num_threads = atoi(argv[3]);
    size_t msg_size = (size_t)atoi(argv[4]);
    int duration = atoi(argv[5]);

    printf("Starting Client (A3): %d threads, %zu bytes/field, %ds duration -> %s:%d\n", 
           num_threads, msg_size, duration, server_ip, port);

    pthread_t threads[num_threads];
    client_thread_args_t args[num_threads];

    for (int i = 0; i < num_threads; i++) {
        args[i].server_ip = server_ip;
        args[i].port = port;
        args[i].message_size = msg_size;
        args[i].duration = duration;
        args[i].bytes_received = 0;
        pthread_create(&threads[i], NULL, client_thread_func, (void *)&args[i]);
    }

    unsigned long long total_bytes = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_bytes += args[i].bytes_received;
    }

    // --- METRICS CALCULATION ---
    double total_mb = (double)total_bytes / (1024 * 1024);
    double throughput_gbps = (total_mb * 8) / (duration * 1000); 

    // Latency Calculation
    size_t full_msg_size = msg_size * 8; 
    
    double latency_us = 0.0;
    if (full_msg_size > 0 && total_bytes > 0) {
        double total_messages = (double)total_bytes / full_msg_size;
        if (total_messages > 0) {
            latency_us = (duration * 1000000.0) / total_messages;
        }
    }

    printf("------------------------------------------------\n");
    printf("[A3 Zero-Copy Result]\n");
    printf("Total Data Received: %.2f MB\n", total_mb);
    printf("Throughput: %.4f Gbps\n", throughput_gbps);
    printf("Latency: %.4f us\n", latency_us);
    printf("------------------------------------------------\n");

    return 0;
}