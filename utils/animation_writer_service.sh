#!/bin/bash

directory=""

cd $directory

while :
do
file=$( find ./images/ -type f | shuf -n 1 ) #choose a random image from the images directory
./newer_gif_player $file & #play the image
sleep 1h #wait for 1 hour
kill $! #kill the process and load a new image
done
