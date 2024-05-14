#!/bin/bash

bin=./huff_codec
dir=./data
width=512

print_header(){
    echo "File Name,Compression Mode,Matching,Uncompressed [bytes],Compressed [bytes],Ratio,Gain,Average bits per pixel"
}

print_stats(){
    compressed_size=$(wc -c $2 | cut -d' ' -f1)
    ratio=$(awk -v c="$compressed_size" -v o="$5" 'BEGIN { printf "%.2f%", (c/o*100)}')
    gain=$(awk -v r="$ratio" 'BEGIN { printf "%.2f%", (100-r)}')
    avgbits=$(awk -v c="$compressed_size" -v o="$5" 'BEGIN { print $1 ((c*8)/o)}')
    if $(cmp $1 $3)
    then
        result="Files match :)"
    else
        result="Files DON'T match :("
    fi
    echo "${1},${4},${result},${5},${compressed_size},${ratio},${gain},${avgbits}"
}

print_header

for file in "$dir"/*.raw
do
    original_size=$(wc -c $file | cut -d' ' -f1)

    eval "$bin -c -a -i ${file} -o ${file}_a -w ${width}"
    eval "$bin -c -i ${file} -o ${file}_s -w ${width}"
    eval "$bin -c -a -m -i ${file} -o ${file}_am -w ${width}"
    eval "$bin -c -m -i ${file} -o ${file}_sm -w ${width}"

    eval "$bin -d -a -i ${file}_a -o ${file}_da"
    eval "$bin -d -i ${file}_s -o ${file}_ds"
    eval "$bin -d -a -m -i ${file}_am -o ${file}_dam"
    eval "$bin -d -m -i ${file}_sm -o ${file}_dsm"

    print_stats $file ${file}_a ${file}_da "Adaptive" $original_size
    print_stats $file ${file}_s ${file}_ds "Static" $original_size
    print_stats $file ${file}_am ${file}_dam "Adaptive Model" $original_size
    print_stats $file ${file}_sm ${file}_dsm "Static Model" $original_size

    rm ${file}_a ${file}_da
    rm ${file}_s ${file}_ds
    rm ${file}_am ${file}_dam
    rm ${file}_sm ${file}_dsm
done
