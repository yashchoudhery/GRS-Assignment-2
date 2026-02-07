

#!/bin/bash
# Client Script for Machine B (With Latency)

SERVER_IP="192.168.224.208" 
PORT=8080
DURATION=10
OUTPUT_CSV="MT24147_client_results.csv"

# Params
MSG_SIZES=(64 1024 32768 1048576)
THREAD_COUNTS=(1 2 4 8)

# Added Latency(us) to header
echo "Implementation,MessageSize,Threads,Throughput(Gbps),Latency(us)" > $OUTPUT_CSV

run_phase() {
    IMPL_NAME=$1
    CLIENT_BIN=$2
    
    echo "------------------------------------------------"
    echo "STARTING PHASE: $IMPL_NAME"
    echo "------------------------------------------------"
    
    for SIZE in "${MSG_SIZES[@]}"; do
        for THREADS in "${THREAD_COUNTS[@]}"; do
            echo "Running: $IMPL_NAME | Size: $SIZE | Threads: $THREADS"
            
            # Run Client
            ./$CLIENT_BIN $SERVER_IP $PORT $THREADS $SIZE $DURATION > client_output.txt
            
            # Extract Throughput
            THROUGHPUT=$(grep "Throughput:" client_output.txt | awk '{print $2}')
            if [ -z "$THROUGHPUT" ]; then THROUGHPUT="0"; fi

            # Extract Latency
            LATENCY=$(grep "Latency:" client_output.txt | awk '{print $2}')
            if [ -z "$LATENCY" ]; then LATENCY="0"; fi
            
            echo "  -> Tput: $THROUGHPUT Gbps | Lat: $LATENCY us"
            echo "$IMPL_NAME,$SIZE,$THREADS,$THROUGHPUT,$LATENCY" >> $OUTPUT_CSV
            
            # Wait 5 seconds to let server cool down
            sleep 5
        done
    done
}

# EXECUTION FLOW
echo "Step 1: Make sure Server A1 is running on Machine A."
read -p "Press Enter to start A1 tests..."
run_phase "A1_TwoCopy" "client_a1"

echo "Step 2: Please switch Machine A to Server A2 now."
read -p "Press Enter to start A2 tests..."
run_phase "A2_OneCopy" "client_a2"

echo "Step 3: Please switch Machine A to Server A3 now."
read -p "Press Enter to start A3 tests..."
run_phase "A3_ZeroCopy" "client_a3"

echo "Done! Results in $OUTPUT_CSV"