#!/bin/bash

target_core=$1

if [ -z "$target_core" ]; then
	echo "[ERROR] Please provide the core number that you wish to check"
    echo "Usage: $0 <core_number>"
	exit 1
fi

# Iterate over the folders inside /proc/irq
for irq_folder in /proc/irq/*; do
    if [ -d "$irq_folder" ]; then
        irq_number=$(basename "$irq_folder")
        smp_affinity_file="$irq_folder/smp_affinity"

        # Check if smp_affinity file exists
        if [ -e "$smp_affinity_file" ]; then
            # Read the content of smp_affinity
            affinity_mask=$(cat "$smp_affinity_file")

            # Check if the affinity mask is associated with the target core
            if (( (1 << target_core) & 0x$affinity_mask )); then
                echo "IRQ $irq_number is associated with Core $target_core. Affinity Mask: $affinity_mask"
            fi
        fi
    fi
done