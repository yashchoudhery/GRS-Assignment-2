/*
 * Name: Yash Choudhery
 * Roll No: MT24147
 * Assignment: PA02 - Analysis of Network I/O primitives
 * Part: Common Header for Part A
 *
 * AI Declaration:
 * Portions of this code were generated/refined using an LLM.
 * Prompt used: "Create a C header file for a network assignment defining a struct 
 * with 8 char pointers (strings) and helper functions to generate random data."
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>

#define PORT 8080
#define NUM_FIELDS 8

// The structure required by the assignment
// It contains 8 dynamically allocated string fields.
typedef struct {
    char *fields[NUM_FIELDS]; // 8 pointers to strings
    size_t lengths[NUM_FIELDS]; // Length of each string
} Message;

// Helper to fill the structure with random data
void generate_random_message(Message *msg, size_t size_per_field) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < NUM_FIELDS; i++) {
        msg->lengths[i] = size_per_field;
        msg->fields[i] = (char *)malloc(size_per_field);
        if (!msg->fields[i]) {
            perror("Malloc failed");
            exit(EXIT_FAILURE);
        }
        for (size_t j = 0; j < size_per_field; j++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            msg->fields[i][j] = charset[key];
        }
    }
}

// Helper to free the memory
void free_message(Message *msg) {
    for (int i = 0; i < NUM_FIELDS; i++) {
        free(msg->fields[i]);
    }
}

#endif