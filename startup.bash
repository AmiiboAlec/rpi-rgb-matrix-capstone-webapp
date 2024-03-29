#!/bin/bash

./cleanup

./led_driver &

sudo -u pi ./websocket_handler &
