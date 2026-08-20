#!/usr/bin/env bash

c_files=()

while IFS= read -rd '' file; do
    c_files+=("$file")
done < <(find . -type f -name '*.c*' -print0)

compile() {
    local file="$1"
    local name="${file##*/}"
    name="${name%.*}"
    local dir="${file%/*}"
    command rm "$dir/$name"
    cc -o "$dir/$name" "$file"
}

for file in "${c_files[@]}"; do
    compile "$file"
done


