#!/usr/bin/python3

import time
import sys
import signal
import os
import socket

killflag = False
f = open("test.txt", "w")
filestdout = open("output.txt", "w")
def output(out):
    global filestdout
    filestdout.write(out + "\n")
    filestdout.flush()

ws_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
ws_sock.connect("child_sock")
#to_ws = open("/tmp/ws_in", "w")
#from_ws = open("/tmp/ws_out", "r")
output("SOCK CONNECTED")

def send_back(message: str):
    global ws_sock
    ws_sock.send(bytes(message, "UTF-8"))
    os.kill(os.getppid(), signal.SIGUSR1)

def read_sock() -> str:
    BLOCK_SIZE = 1024
    global ws_sock
    buff = ''
    while (True):
        data = ws_sock.recv(BLOCK_SIZE)
        buff += str(data, "UTF-8")
        if len(data) != BLOCK_SIZE:
            return buff

def sighandler(signum, frame):
    global killflag
    killflag = True
    return

def message_recv(signum, frame):
    output("message called")
    message = read_sock()
    output("sock read")
    f.write(message + "\n")
    f.flush()
    send_back("Message Read")

f.write("ABCDE")

for i in sys.argv:
    f.write(i + "\n")
f.flush()
signal.signal(signal.SIGTERM, sighandler)
signal.signal(signal.SIGUSR1, message_recv)

while not killflag:
    #to_ws.write("hello\n")
    #to_ws.flush()
    #os.kill(os.getppid(), signal.SIGUSR1)
    time.sleep(1)

ws_sock.close()
filestdout.close()
f.write("Stopped\n")
f.flush()
f.close()