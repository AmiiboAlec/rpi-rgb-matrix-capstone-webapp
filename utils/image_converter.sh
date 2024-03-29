#!/bin/bash

# usage: ./image_converter.sh -i <input file> -o <output_file> -[nlc] 

while getopts "i:o:nlc" o; do 
    case "${o}" in
        i)
            inp=${OPTARG}
            ;;
        o)
            out=${OPTARG}
            ;;
        n)
            scaling="neighbor"
            ;;
        l)
            scaling="bilinear"
            ;;
        c)
            scaling="bicubic"
            ;;
    esac
done

if [ -z "$inp" ]
then
    echo "usage: ./image_converter.sh -i <input file> -o <output_file> -[nlc]"
    echo "-n: nearest neighbor interpolation"
    echo "-l: bilinear interpolation"
    echo "-v: bicubic interpolation"
    exit 1
fi
mkdir temp
ffmpeg -i "$inp" scale=64:32:flags=$scaling temp/temp%d.png
python3 image_converter.py temp/*.png
sed -e '$s/$/\a/' -s temp/*.txt > $out # concat all the txts together, inserting the bell character as delimiters
rm -rf temp