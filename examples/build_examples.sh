#!/usr/bin/env bash

declare -A dirs=()

while IFS= read -rd '' file; do
    dirs["${file%/*}"]=1
done < <(find . -type f -name '*.c' -print0)

for dir in "${!dirs[@]}"; do
    mapfile -d '' -O0 c_files < <(find "$dir" -maxdepth 1 -type f -name '*.c' -print0 | sort -z)

    name="${dir##*/}"

    echo "Compiling $dir -> $name"
    cc -o "$dir/$name" "${c_files[@]}"
done
