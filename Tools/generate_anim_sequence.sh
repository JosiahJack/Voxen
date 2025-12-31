#!/bin/bash
current_index=6545

generate_sequence() {
    local prefix=$1
    local start_frame=$2
    local end_frame=$3
    local pad=${4:-6}  # default padding 6 digits

    for frame in $(seq $start_frame $end_frame); do
        frame_str=$(printf "%0${pad}d" $frame)
        ((current_index++))
        echo "#Models/${prefix}_${frame_str}.obj"
        echo "index: $current_index"
    done
    echo  # blank line after each sequence
}


generate_sequence "npc_mutant_cyborg" 1 258
