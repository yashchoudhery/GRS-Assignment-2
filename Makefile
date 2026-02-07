# Name: Yash Choudhery
# Roll No: MT24147
# Assignment: PA02 - Analysis of Network I/O primitives

# Compiler and Flags
CC = gcc
CFLAGS = -pthread -Wall

# Source Files
SRC_A1_SERVER = MT24147_Part_A1_Server.c
SRC_A1_CLIENT = MT24147_Part_A1_Client.c
SRC_A2_SERVER = MT24147_Part_A2_Server.c
SRC_A2_CLIENT = MT24147_Part_A2_Client.c
SRC_A3_SERVER = MT24147_Part_A3_Server.c
SRC_A3_CLIENT = MT24147_Part_A3_Client.c

# Output Binaries
BIN_A1_SERVER = server_a1
BIN_A1_CLIENT = client_a1
BIN_A2_SERVER = server_a2
BIN_A2_CLIENT = client_a2
BIN_A3_SERVER = server_a3
BIN_A3_CLIENT = client_a3

# Default Target: Compile All
all: $(BIN_A1_SERVER) $(BIN_A1_CLIENT) $(BIN_A2_SERVER) $(BIN_A2_CLIENT) $(BIN_A3_SERVER) $(BIN_A3_CLIENT)

# ---------------------------------------------------------
# Compilation Rules
# ---------------------------------------------------------

# Part A1 (Two-Copy)
$(BIN_A1_SERVER): $(SRC_A1_SERVER) MT24147_Part_A_Common.h
	$(CC) $(CFLAGS) $(SRC_A1_SERVER) -o $(BIN_A1_SERVER)

$(BIN_A1_CLIENT): $(SRC_A1_CLIENT) MT24147_Part_A_Common.h
	$(CC) $(CFLAGS) $(SRC_A1_CLIENT) -o $(BIN_A1_CLIENT)

# Part A2 (One-Copy)
$(BIN_A2_SERVER): $(SRC_A2_SERVER) MT24147_Part_A_Common.h
	$(CC) $(CFLAGS) $(SRC_A2_SERVER) -o $(BIN_A2_SERVER)

$(BIN_A2_CLIENT): $(SRC_A2_CLIENT) MT24147_Part_A_Common.h
	$(CC) $(CFLAGS) $(SRC_A2_CLIENT) -o $(BIN_A2_CLIENT)

# Part A3 (Zero-Copy)
$(BIN_A3_SERVER): $(SRC_A3_SERVER) MT24147_Part_A_Common.h
	$(CC) $(CFLAGS) $(SRC_A3_SERVER) -o $(BIN_A3_SERVER)

$(BIN_A3_CLIENT): $(SRC_A3_CLIENT) MT24147_Part_A_Common.h
	$(CC) $(CFLAGS) $(SRC_A3_CLIENT) -o $(BIN_A3_CLIENT)

# ---------------------------------------------------------
# Run Automation Script
# ---------------------------------------------------------
run: all
	@echo "Making script executable..."
	chmod +x MT24147_Part_C_Client_MachineB.sh
	@echo "Running Experiments (Requires Sudo for perf)..."
	sudo ./MT24147_Part_C_Client_MachineB.sh

# ---------------------------------------------------------
# Cleanup
# ---------------------------------------------------------
clean:
	rm -f $(BIN_A1_SERVER) $(BIN_A1_CLIENT)
	rm -f $(BIN_A2_SERVER) $(BIN_A2_CLIENT)
	rm -f $(BIN_A3_SERVER) $(BIN_A3_CLIENT)
	rm -f *.o
	rm -f perf_output.txt client_output.txt
	@echo "Cleaned up binaries and temp files."