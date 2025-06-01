#!/bin/bash

# -------- Configuration --------
INTERVAL=0.05   
REPS=35         
WARMUPS=5
CMD="../build/src/game"
OUTPUT="/stats/execution_times.json"

# -------- Init JSON --------
echo "[" > "$OUTPUT"
first_run=1

# Loop
for run in $(seq 1 $REPS); do
    echo "===> Running tests $run..."

    start=$(date +%s.%N)
    $CMD > /dev/null 2>&1
    end=$(date +%s.%N)

    raw_elapsed=$(echo "$end - $start" | bc)

    if [[ $raw_elapsed == .* ]]; then
      elapsed="0$raw_elapsed"
    else
      elapsed="$raw_elapsed"
    fi

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

        echo "      {\"run\": $run_index, \"elapsed_secs\": $elapsed}" >> "$OUTPUT"

    fi

    if [ $save -eq 1 ]; then
        echo "    Saved run $run_index"
    fi

done

# -------- End JSON --------
echo "]" >> "$OUTPUT"
echo "Complete"

