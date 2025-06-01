#!/bin/bash

# -------- Configuration --------
INTERVAL=0.05 
REPS=35
WARMUPS=5
CMD="bb prove -b ../target/sudoku_game.json -w ../target/sudoku_game.gz -o ../target"
OUTPUT="memory_pss_usage.json"

# -------- Total RAM in system (KiB) --------
total_mem_kib=$(awk '/MemTotal/ {print $2}' /proc/meminfo)

# -------- Init JSON --------
echo "[" > "$OUTPUT"
first_run=1

# Loop
for run in $(seq 1 $REPS); do
    echo "===> Running tests $run..."

    $CMD &
    pid=$!

    if [ $run -le $WARMUPS ]; then
        echo "    (This repetition is not saved)"
        save=0
    else
        save=1
        run_index=$((run - WARMUPS))

        if [ $first_run -eq 0 ]; then
            echo "," >> "$OUTPUT"
        fi
        first_run=0

        echo "  {" >> "$OUTPUT"
        echo "    \"run\": $run_index," >> "$OUTPUT"
        echo "    \"samples\": [" >> "$OUTPUT"
        first_sample=1
    fi

    start_ts=$(date +%s.%N)

    # run until command finish
    while kill -0 "$pid" 2>/dev/null; do
        if [ ! -r /proc/$pid/smaps ]; then
            break
        fi

        pss_kib=$(grep Pss /proc/$pid/smaps 2>/dev/null | awk '{sum += $2} END {print sum}')
        pss_kib=${pss_kib:-0}
        if [ "$pss_kib" -eq 0 ]; then
            break
        fi

        pss_perc=$(awk "BEGIN { printf \"%.2f\", ($pss_kib/$total_mem_kib)*100 }")
        now_ts=$(date +%s.%N)
        rel_ts=$(awk "BEGIN { printf \"%.3f\", ($now_ts - $start_ts) }")

        if [ $save -eq 1 ]; then
            if [ $first_sample -eq 0 ]; then
                echo "," >> "$OUTPUT"
            fi
            first_sample=0

            echo "      {\"timestamp\": $rel_ts, \"kib\": $pss_kib, \"percent\": $pss_perc}" >> "$OUTPUT"
        fi

        sleep "$INTERVAL"
    done

    if [ $save -eq 1 ]; then
        echo "" >> "$OUTPUT"
        echo "    ]" >> "$OUTPUT"
        echo "  }" >> "$OUTPUT"
        echo "    Saved run $run_index"
    fi

done

# -------- End JSON --------
echo "]" >> "$OUTPUT"
echo "Complete"
