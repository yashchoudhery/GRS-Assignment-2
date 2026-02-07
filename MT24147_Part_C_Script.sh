#!/bin/bash

# Name: Yash Choudhery
# Roll No: MT24147
# Assignment: PA02 - Part C (Automated Experiment Script)

# ---------------------------------------------------------
# CONFIGURATION
# ---------------------------------------------------------
SERVER_IP="192.168.224.208"
PORT=8081
DURATION=10  # Seconds to run each test

# Define the parameters to test
# Message Sizes: 64B, 1KB, 32KB, 1MB
MSG_SIZES=(64 1024 32768 1048576)

# Thread Counts: 1, 2, 4, 8
THREAD_COUNTS=(1 2 4 8)

# Output File
OUTPUT_CSV="MT24147_experiment_results.csv"

# ---------------------------------------------------------
# 1. COMPILATION
# ---------------------------------------------------------
echo "---------------------------------------------------"
echo "[Step 1] Compiling all implementations..."
echo "---------------------------------------------------"

# Compile Part A1 (Two-Copy)
gcc MT24147_Part_A1_Server.c -o server_a1 -pthread
gcc MT24147_Part_A1_Client.c -o client_a1 -pthread

# Compile Part A2 (One-Copy)
gcc MT24147_Part_A2_Server.c -o server_a2 -pthread
gcc MT24147_Part_A2_Client.c -o client_a2 -pthread

# Compile Part A3 (Zero-Copy)
gcc MT24147_Part_A3_Server.c -o server_a3 -pthread
gcc MT24147_Part_A3_Client.c -o client_a3 -pthread

echo "Compilation successful."
echo ""

# ---------------------------------------------------------
# 2. PREPARE CSV
# ---------------------------------------------------------
# Write the header to the CSV file
echo "Implementation,MessageSize,Threads,Throughput(Gbps),Latency(us),Cycles,L1_Misses,LLC_Misses,Context_Switches" > $OUTPUT_CSV

# ---------------------------------------------------------
# 3. EXPERIMENT LOOP
# ---------------------------------------------------------

# Helper function to run one experiment
run_experiment() {
    IMPL_NAME=$1
    SERVER_BIN=$2
    CLIENT_BIN=$3
    MSG_SIZE=$4
    THREADS=$5
    
    echo "Running: $IMPL_NAME | Size: $MSG_SIZE | Threads: $THREADS"

    # A. Start the Server in the background
    # We redirect stdout/stderr to /dev/null to keep terminal clean
    ./$SERVER_BIN > /dev/null 2>&1 &
    SERVER_PID=$!
    
    # Give server a moment to initialize
    sleep 2

    # B. Start perf stat on the Server PID
    # We measure: cpu-cycles, cache-misses (LLC), L1-dcache-load-misses, context-switches
    # We run perf for the duration of the test minus a small buffer
    # Output is saved to a temp file
    sudo perf stat -p $SERVER_PID -e cycles,L1-dcache-load-misses,cache-misses,context-switches -o perf_output.txt -- sleep $DURATION &
    PERF_PID=$!

    # C. Start the Client
    # Capture the output to extract Throughput
    ./$CLIENT_BIN $SERVER_IP $PORT $THREADS $MSG_SIZE $DURATION > client_output.txt
    
    # Wait for the client to finish
    wait $PERF_PID

    # D. Cleanup: Kill the server
    sudo kill -9 $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    
    # E. Parse Results
    
    # 1. Extract Throughput (Gbps) from client_output.txt
    # Expected line: "Throughput: 4.5000 Gbps"
    THROUGHPUT=$(grep "Throughput:" client_output.txt | awk '{print $2}')
    
    # 2. Extract Metrics from perf_output.txt
    # Note: perf format can vary. We remove commas for easier parsing.
    
    # Cycles
    CYCLES=$(grep "cycles" perf_output.txt | sed 's/,//g' | awk '{print $1}')
    if [ -z "$CYCLES" ]; then CYCLES="0"; fi

    # L1 Misses
    L1_MISS=$(grep "L1-dcache-load-misses" perf_output.txt | sed 's/,//g' | awk '{print $1}')
    if [ -z "$L1_MISS" ]; then L1_MISS="0"; fi

    # LLC Misses (cache-misses usually refers to Last Level Cache)
    LLC_MISS=$(grep "cache-misses" perf_output.txt | sed 's/,//g' | awk '{print $1}')
    if [ -z "$LLC_MISS" ]; then LLC_MISS="0"; fi

    # Context Switches
    CS=$(grep "context-switches" perf_output.txt | sed 's/,//g' | awk '{print $1}')
    if [ -z "$CS" ]; then CS="0"; fi
    
    # 3. Calculate Application-Level Latency (Microseconds)
    # Approx Latency = (1 / (Throughput_bps / MsgSize_bits)) * 1e6
    # Or simplified: Time_per_msg. Here we assume generic calculation based on throughput.
    # If Throughput is 0, Latency is infinite (set to 0 for CSV safety)
    if (( $(echo "$THROUGHPUT > 0" | bc -l) )); then
        # Latency (us) = (MsgSize * 8) / (Throughput_Gbps * 10^9) * 10^6
        #              = (MsgSize * 8) / (Throughput_Gbps * 1000)
        LATENCY=$(echo "scale=4; ($MSG_SIZE * 8) / ($THROUGHPUT * 1000)" | bc)
    else
        LATENCY="0"
    fi

    # F. Save to CSV
    echo "$IMPL_NAME,$MSG_SIZE,$THREADS,$THROUGHPUT,$LATENCY,$CYCLES,$L1_MISS,$LLC_MISS,$CS" >> $OUTPUT_CSV

    # G. Cleanup temp files
    rm perf_output.txt client_output.txt
    
    # Brief pause to ensure socket is released
    sleep 1
}

# ---------------------------------------------------------
# 4. MAIN EXECUTION
# ---------------------------------------------------------

echo "Starting Experiments... (This will take approx 10-15 mins)"
echo "Results will be saved to $OUTPUT_CSV"

# Loop 1: A1 (Two-Copy)
for SIZE in "${MSG_SIZES[@]}"; do
    for THREADS in "${THREAD_COUNTS[@]}"; do
        run_experiment "A1_TwoCopy" "server_a1" "client_a1" $SIZE $THREADS
    done
done

# Loop 2: A2 (One-Copy)
for SIZE in "${MSG_SIZES[@]}"; do
    for THREADS in "${THREAD_COUNTS[@]}"; do
        run_experiment "A2_OneCopy" "server_a2" "client_a2" $SIZE $THREADS
    done
done

# Loop 3: A3 (Zero-Copy)
for SIZE in "${MSG_SIZES[@]}"; do
    for THREADS in "${THREAD_COUNTS[@]}"; do
        run_experiment "A3_ZeroCopy" "server_a3" "client_a3" $SIZE $THREADS
    done
done

echo "---------------------------------------------------"
echo "All experiments completed."
echo "Data saved in $OUTPUT_CSV"
echo "---------------------------------------------------"