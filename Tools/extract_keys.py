#!/usr/bin/env python3

keys = set()

for level in range(14):
    filename = f"./Data/level{level}.txt"

    try:
        with open(filename, "r", encoding="utf-8") as f:
            for line in f:
                # Split into pipe-delimited segments
                for segment in line.rstrip("\n").split("|"):
                    # Ignore hierarchy names or anything lacking a colon
                    colon = segment.find(":")
                    if colon == -1:
                        continue

                    keys.add(segment[:colon])

    except FileNotFoundError:
        print(f"Warning: {filename} not found")

for key in sorted(keys):
    print(key)
