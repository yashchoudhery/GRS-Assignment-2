#!/bin/bash
# Server Script for Machine A (Fixed for Port 8080)
# Author: Yash Choudhery (MT24147)
# Modified to log PERF stats every second in CSV format.

echo "---------------------------------------------------"
echo "[Server Machine A] Starting Automation on PORT 8080..."
echo "---------------------------------------------------"

# Kill old servers
sudo pkill server_a1
sudo pkill server_a2
sudo pkill server_a3
echo "Clearing Port 8080... Done."
sleep 2

run_server_phase() {
    SERVER_BIN=$1
    NAME=$2
    CSV_FILE="${NAME}_perf_stats.csv"
    
    echo ""
    echo "##########################################################"
    echo "  PHASE: $NAME "
    echo "  Server: ./$SERVER_BIN"
    echo "  Port: 8080"
    echo "##########################################################"
    
    # 1. Start Server
    ./$SERVER_BIN > /dev/null 2>&1 &
    SERVER_PID=$!
    
    sleep 2
    
    echo "-> Server PID: $SERVER_PID is RUNNING on Port 8080"
    
    # 2. Start Perf (CSV Mode, Every 1 Second)
    echo "-> Attaching PERF tool (Logging to $CSV_FILE)..."
    
    # Add a header to the CSV file manually first
    echo "Time,CounterValue,Unit,EventName,RunTime,Percentage" > $CSV_FILE
    
    # Run perf with -I 1000 (every 1s) and -x, (CSV format)
    # We use --append to add to the file we just created
    sudo perf stat -p $SERVER_PID \
        -e cycles,instructions,L1-dcache-load-misses,cache-misses,context-switches \
        -I 60000 -x, \
        --append -o "$CSV_FILE" &
    
    PERF_PID=$!
    
    # 3. INFINITE WAIT
    echo ""
    echo "✅ SERVER LIVE ON (8080). LOGGING STATS EVERY SEC."
    echo "👉 Go to Machine B and RUN YOUR CLIENT."
    echo "👉 The CSV will capture data continuously."
    echo "👉 When Client finishes ALL tests, press [ENTER] here."
    read -p "Waiting..."
    
    # 4. Cleanup
    echo "Stopping $NAME..."
    sudo kill -2 $PERF_PID 2>/dev/null
    wait $PERF_PID 2>/dev/null
    sudo kill -9 $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    
    echo "-> CSV Saved: $CSV_FILE"
}

# Run Phases
run_server_phase "server_a1" "A1_TwoCopy"
run_server_phase "server_a2" "A2_OneCopy"
run_server_phase "server_a3" "A3_ZeroCopy"

echo "Done."

#!/bin/bash
# Server Script for Machine A
# Logs perf stats to CSV every second

# echo "---------------------------------------------------"
# echo "[Server Machine A] Starting Automation on PORT 8080..."
# echo "---------------------------------------------------"

# # Kill old servers
# sudo pkill server_a1
# sudo pkill server_a2
# sudo pkill server_a3
# sleep 2

# run_server_phase() {
#     SERVER_BIN=$1
#     NAME=$2
#     # This will be the output CSV for this phase
#     CSV_FILE="${NAME}_perf_stats.csv"
    
#     echo ""
#     echo "##########################################################"
#     echo "  PHASE: $NAME "
#     echo "  Logging PERF to: $CSV_FILE"
#     echo "##########################################################"
    
#     # 1. Start Server
#     ./$SERVER_BIN > /dev/null 2>&1 &
#     SERVER_PID=$!
    
#     sleep 2
#     echo "-> Server PID: $SERVER_PID is RUNNING"
    
#     # 2. Attach Perf (CSV Mode)
#     # -I 1000 = Sample every 1000ms (1 sec)
#     # -x,     = CSV output format
#     sudo perf stat -p $SERVER_PID \
#         -e cycles,instructions,L1-dcache-load-misses,cache-misses,context-switches \
#         -I 1000 -x, \
#         --append -o "$CSV_FILE" &
    
#     PERF_PID=$!
    
#     # 3. Wait for User
#     echo "✅ SERVER READY. Run Client on Machine B."
#     read -p ">>> Press [ENTER] when Client finishes this phase <<<"
    
#     # 4. Cleanup
#     echo "Stopping..."
#     sudo kill -2 $PERF_PID 2>/dev/null
#     wait $PERF_PID 2>/dev/null
#     sudo kill -9 $SERVER_PID 2>/dev/null
#     wait $SERVER_PID 2>/dev/null
# }

# # Run Phases
# run_server_phase "server_a1" "A1_TwoCopy"
# run_server_phase "server_a2" "A2_OneCopy"
# run_server_phase "server_a3" "A3_ZeroCopy"

# echo "Done."