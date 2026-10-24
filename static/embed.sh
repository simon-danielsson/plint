#!/usr/bin/env bash

root="$(dirname "$(readlink -f "$0")")"
cd "$root"

chmod +x ./embed.sh

if [[ ! -f "./embed" ]]; then
    gcc -flto -O3 -o embed embed.c
fi

rm -f *.h

for file in *; do
    [[ "$file" == *embed* ]] && continue
    ./embed "$file"
done
