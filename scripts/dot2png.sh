#!/bin/bash
set -e


print_gradient() {
    local text="$1"
    local len=${#text}
    
    if [[ "$len" -le 1 ]]; then
        printf "%s\n" "$text"
        return
    fi
    
    for (( i=0; i<$len; i++ )); do
        local r=$(( 170 + (230 - 170) * i / (len - 1) ))
        local g=$(( 120 + (215 - 120) * i / (len - 1) ))
        local b=255
        
        printf "\033[38;2;%d;%d;%dm%s" "$r" "$g" "$b" "${text:$i:1}"
    done
    
    printf "\033[0m\n"
}



DOT_DIR="${1:-output/dot}"
PNG_DIR="${2:-output/png}"

mkdir -p "$PNG_DIR"

for dot_file in "$DOT_DIR"/*.dot; do
    [ -f "$dot_file" ] || continue
    name=$(basename "$dot_file" .dot)
    dot -Tpng "$dot_file" -o "$PNG_DIR/$name.png" && \
    print_gradient "Converted $name.dot into $name.png"
done