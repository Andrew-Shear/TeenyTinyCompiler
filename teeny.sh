#!/bin/bash

if [[ $# -eq 0 ]]; then
    for tiny in examples/*.teeny; do
        echo "--------------"
        if src/teenytiny "$tiny"; then
            # compilation was successful!
            gcc out.c -o "$(basename "${tiny%.teeny}")"
        fi
    done
    # if the files don't exist, don't tell the user :)
    rm .header &> /dev/null
    rm .code &> /dev/null
    rm out.c &> /dev/null
else
    for tiny in "$@"; do
        echo "--------------"
        if src/teenytiny "$tiny"; then
            # compilation was successful!
            gcc out.c -o "$(basename "${tiny%.teeny}")"
        fi
    done
    rm .header &> /dev/null
    rm .code &> /dev/null
    rm out.c &> /dev/null
fi
