#!/usr/bin/python3

import asyncio
import websockets
import signal
from os import kill, remove, path
from subprocess import Popen
import socket

if path.exists("child_sock"):
    remove("child_sock")

client = None
child = -1
child_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
child_sock.bind("child_sock")
CURRENT_DIRECTORY = "/home/pi/LED-project-new/"
child_conn = -1
child_address = None
#mkfifo("/tmp/ws_in", 0o666)
#mkfifo("/tmp/ws_out", 0o666)

#inp = open("/dev/null", "r")
#out = open("/dev/null", "w")

def input_from_fifo(signum, frame):
    BLOCK_SIZE = 1024
    global child_conn, client
    buffer = ''
    try:
        while True:
            data = child_conn.recv(BLOCK_SIZE)
            buffer += str(data, "UTF-8")
            if len(data) != BLOCK_SIZE:
                break
        #data = inp.read()
    finally:
        #print(data)
        if (client != None):
            print("A")
            #server.send_message(client=client, msg=buffer)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        else:
            print("NO CLIENT")

def new_client(c):
    global client
    print("CLIENT CONNECTED\n")
    client = c

def client_left():
    global client
    print("CLIENT DISCONNECTED")
    client = None

def message_recv(message: str, websock):
    #global inp
    #global out
    global child, child_sock, child_conn, child_address
    print("message received")
    #print("MESSAGE RECIEVED\n" + str(client) + "\n" + str(server) + "\n" + str(message))
    if message[0:4] == "NEW\n":
        if (child_conn != -1):
            kill(child, signal.SIGTERM)
            #waitpid(child, 0) #commenting this out until it causes problems
        print(message)
        #inp.close()
        #out.close()
        lines = message.split('\n')
        newlines = [CURRENT_DIRECTORY + lines[1]]
        for i in range(2, len(lines)):
            newlines.append(lines[i])
        child = Popen(newlines, close_fds=True).pid
        print("Child pid: " + str(child))
        child_sock.setblocking(0)
        child_sock.listen(1)
        child_conn, child_address = child_sock.accept()
        print("accepted")
        #child_conn.send(b"hello")
        #inp = open("/tmp/ws_in", "r")
        #out = open("/tmp/ws_out", "w")
    elif message[0:4] == "KILL":
        kill(child, signal.SIGTERM)
        remove("child_sock")
        exit()
    else:
        #out.write(message)
        #out.flush()
        try:
            child_conn.send(bytes(message + "\a", 'utf-8'))
        except Exception as error:
            print(error)
        finally:
            if (child != -1): #May remove and just use SIGIO signals
                kill(child, signal.SIGUSR1)
    
signal.signal(signal.SIGUSR1, input_from_fifo)

async def websocketfunc(websocket):
    try:
        new_client(websocket)
        async for message in websocket:
            message_recv(message, websocket)
    except websockets.exceptions.ConnectionClosed:
        client_left()

async def main():
    async with websockets.serve(websocketfunc, "0.0.0.0", 9002):
        await asyncio.Future()

asyncio.run(main())

#inp.close()
#out.close()
#remove("/tmp/ws_in")
#remove("/tmp/ws_out")
remove("child_sock")