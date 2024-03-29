#!/usr/bin/python3

import sysv_ipc
import signal
import colorsys
import time

SHARED_MEMORY_KEY = 25375
kill_flag = False
width, height = 64, 32
smfb = sysv_ipc.SharedMemory(SHARED_MEMORY_KEY, flags = sysv_ipc.IPC_CREAT, mode = 0o666, size = (width * height * 3) + 1)
clk_id1 = time.CLOCK_REALTIME
SCALE_DOWN_XY_CONSTANT = 1/8

def stop_loop(signum, frame):
    global kill_flag 
    kill_flag = True


signal.signal(signal.SIGTERM, stop_loop)

local_framebuffer = bytearray(width * height * 3)

last_iteration = time.clock_gettime(clk_id1)

while not kill_flag:
    current_iteration = time.clock_gettime(clk_id1)
    print(current_iteration - last_iteration)
    last_iteration = current_iteration
    for y in range(height):
        for x in range(width):
            colindex = y * width + x
            planeindex = width * height
            red, green, blue = colorsys.hsv_to_rgb((x * SCALE_DOWN_XY_CONSTANT) + (y * SCALE_DOWN_XY_CONSTANT) + time.clock_gettime(clk_id1), 1, 1)
            local_framebuffer[colindex] = int(red * 255)
            local_framebuffer[colindex + planeindex] = int(green * 255)
            local_framebuffer[colindex + (planeindex * 2)] = int(blue * 255)
    smfb.write(local_framebuffer, 0)
    time.sleep(0.01)