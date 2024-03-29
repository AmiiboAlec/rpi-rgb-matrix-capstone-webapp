#!/bin/bash

directory=""

cd $directory

rotation=0 #if installed upside down, set this to 180

./led_driver --led-pixel-mapper Rotate:$rotation 