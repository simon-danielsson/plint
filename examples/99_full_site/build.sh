#!/usr/bin/env bash

gcc -fsanitize=address -o 99_full_site 99_full_site.c variables_routes.c
./99_full_site

