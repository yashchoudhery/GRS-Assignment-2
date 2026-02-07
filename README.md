# GRS_PA02: Network I/O Primitives Analysis

**Student Name:** Yash Choudhery  
**Roll No:** MT24147  
**Course:** Graduate Systems (CSE638)  
**Assignment:** PA02  
**GitHub Repo:** [https://github.com/yashchoudhery/GRS-Assignment-2](https://github.com/yashchoudhery/GRS-Assignment-2)

---

## Project Overview
This project experimentally studies the cost of data movement in network I/O by implementing three different socket communication methods. The goal is to analyze the impact of User-Kernel data copying on CPU cycles, cache behavior, throughput, and latency.

The three implementations are:
1.  **Part A1 (Two-Copy):** Standard `send()`/`recv()` where data is serialized into a user buffer before sending. This involves copying data from Application Buffer → Kernel Socket Buffer.
2.  **Part A2 (One-Copy):** Optimized `sendmsg()` using `struct iovec` (Scatter-Gather I/O) to avoid user-space serialization. Data is copied directly from Application Structs → Kernel Socket Buffer.
3.  **Part A3 (Zero-Copy):** Advanced `sendmsg()` with the `MSG_ZEROCOPY` flag, enabling the Network Interface Card (NIC) to read data directly from user memory via DMA, bypassing CPU copy overheads.

---

## Directory Structure
The repository is structured as follows:

GRS_PA02/ ├── MT24147_Part_A_Common.h # Shared header (Struct definitions, constants, includes) ├── MT24147_Part_A1_Server.c # Two-Copy Server (Standard send) ├── MT24147_Part_A1_Client.c # Two-Copy Client (Standard recv + Retry Logic) ├── MT24147_Part_A2_Server.c # One-Copy Server (Scatter-Gather sendmsg) ├── MT24147_Part_A2_Client.c # One-Copy Client (Standard recv + Retry Logic) ├── MT24147_Part_A3_Server.c # Zero-Copy Server (MSG_ZEROCOPY + Error Queue) ├── MT24147_Part_A3_Client.c # Zero-Copy Client (Standard recv + Retry Logic) ├── MT24147_Part_C_Server_MachineA.sh # Server Automation Script (Runs perf & CSV logging) ├── MT24147_Part_C_Client_MachineB.sh # Client Automation Script (Generates Load) ├── MT24147_Part_D_Plots.ipynb # Python Notebook for Data Visualization ├── MT24147_client_results.csv # Generated Throughput & Latency Data (Client Side) ├── Makefile # Compilation and Execution Manager └── README.md # Project Documentation


---

## Prerequisites
* **OS:** Linux (Required for `perf`, `MSG_ZEROCOPY`, and `sendmsg`).
* **Setup:** Two distinct physical/virtual machines (recommended for accurate network profiling).
    * **Machine A:** Acts as the **Server** (Sender).
    * **Machine B:** Acts as the **Client** (Receiver).
* **Tools:** `perf` (linux-tools), `python3`, `jupyter`, `matplotlib`, `gcc`, `make`.
* **Privileges:** `sudo` access is required on the Server machine to run `perf` for hardware counter access.

---

## Compilation & Execution Guide

### 1. Compilation
Run this command on **BOTH** Machine A and Machine B to compile all binaries.
```bash
make
This generates server_a1, client_a1, server_a2, etc.

2. Automated Experiments (Multi-Machine Setup)
This experiment uses a synchronized script approach to ensure accurate profiling without connection failures.

Step 1: Start Server (Machine A)
Navigate to the project folder.

Run the server automation script. This script handles perf profiling, logs CPU cycles/cache misses to a CSV every second, and manages the server processes.

Bash
chmod +x MT24147_Part_C_Server_MachineA.sh
sudo ./MT24147_Part_C_Server_MachineA.sh
The script will start server_a1 and wait. It will display:

✅ SERVER LIVE ON (8080). LOGGING STATS EVERY SEC.

Step 2: Start Client (Machine B)
Open MT24147_Part_C_Client_MachineB.sh and ensure SERVER_IP matches Machine A's IP address.

Run the client load generator:

Bash
chmod +x MT24147_Part_C_Client_MachineB.sh
sudo ./MT24147_Part_C_Client_MachineB.sh
The client will loop through all message sizes (64B to 1MB) and thread counts (1 to 8), recording Throughput and Latency to MT24147_client_results.csv.

Step 3: Synchronization
The Client script runs through "Phase 1 (Two-Copy)". When it finishes, it will pause and ask you to switch the server.

Go to Machine A, press [ENTER]. The script will kill server_a1 and start server_a2.

Go to Machine B, press [ENTER]. The client will start the "One-Copy" tests.

Repeat for "Zero-Copy".

3. Manual Execution (Debugging)
If you wish to run a specific test case manually without scripts:

On Machine A (Server):

Bash
# Example: Run Two-Copy Server
./server_a1
On Machine B (Client):

Bash
# Usage: ./client <IP> <Port> <Threads> <MsgSize> <Duration>
# Example: 4 Threads, 32KB Message, 10 Seconds
./client_a1 192.168.1.101 8080 4 32768 10
Data Analysis & Visualization (Part D)
The experiment generates two main data sources:

Client Side (MT24147_client_results.csv): Contains Throughput (Gbps) and Latency (µs).

Server Side (*_perf_stats.csv): Contains CPU Cycles, Cache Misses, and Context Switches logged every second.

Generating Plots
Transfer the CSV files from your Linux machine to your local machine (if using a GUI-based Jupyter).

Open the Jupyter Notebook:

Bash
jupyter notebook MT24147_Part_D_Plots.ipynb
Data Import: Copy the relevant values from the CSV files into the data arrays in the first cell of the notebook. (Note: Average the perf stats for the active duration of each test case).

Run all cells to generate the 4 required plots:

Throughput vs Message Size

Latency vs Thread Count

Cache Misses vs Message Size

CPU Cycles per Byte Transferred

AI Declaration
Compliance with Assignment Guidelines: I have used Generative AI tools (ChatGPT/Gemini) to assist in the development of this assignment. Below is a detailed declaration of the components where AI was used and the prompts provided.

1. Code Generation (Boilerplate & Logic)
Component: Part A1 Server (Threading Logic)

Prompt: "Write a multithreaded C server that accepts TCP connections. Inside the thread, serialize a struct of 8 strings into a single buffer and send it using send(). Explain the copies."

Usage: Used to generate the pthread setup and the memory copying loop structure.

Component: Part A2 Server (Scatter-Gather I/O)

Prompt: "Write a C server that uses sendmsg and struct iovec to send 8 scattered buffers without combining them first (One-Copy)."

Usage: Used to understand the syntax of struct msghdr and struct iovec.

Component: Part A3 Server (Zero-Copy)

Prompt: "Write a C server function that uses sendmsg with MSG_ZEROCOPY. Include the necessary setsockopt setup and a helper function to read the MSG_ERRQUEUE to process completion notifications."

Usage: Critical for implementing the recv_completion_notifications function to handle kernel signals regarding DMA completion.

2. Automation Scripting
Component: Part C Bash Scripts

Prompt: "Write a bash script that runs perf stat on a specific PID and appends the output to a CSV file every second."

Usage: Used to create the MT24147_Part_C_Server_MachineA.sh script for granular profiling.