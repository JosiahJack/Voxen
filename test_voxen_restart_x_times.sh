#!/bin/bash
VOXEN="./voxen"
ITERATIONS=50
echo "Starting $ITERATIONS voxen test iterations..."
for ((i=1; i<=ITERATIONS; i++)); do
    echo "Iteration $i/$ITERATIONS"
    $VOXEN &
    VOXEN_PID=$!
    sleep 1.3 #secs
    if ! pidof voxen >/dev/null; then
        echo -e "\033[31mError: voxen (PID $VOXEN_PID) crashed\033[0m"
        exit 1
    fi

    kill -9 $VOXEN_PID 2>/dev/null
done

echo "Completed $ITERATIONS iterations"
exit 0
