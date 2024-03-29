#!/usr/bin/python3

import math
from typing import List, Tuple
import time
import colorsys
import sys
import sysv_ipc
import signal
import argparse

SHARED_MEMORY_KEY = 25375

clk_id1 = time.CLOCK_REALTIME #dunno which of these will give better results
clk_id2 = time.CLOCK_MONOTONIC

width = 64
height = 32
circle = [(1,2)]
kill_flag = False

hands_dict = {}

def parse_hex(input: str) -> Tuple[int, int, int]:
    return tuple(int(input[i:i+2], 16) for i in (0,2,4))

num_of_outside_points = 4
decay = 0.25
time_scale = 4
hand_base_color = (0x39,0xff,0x14)
outside_base_color = (0xA3,0xFF,0xF9)
continue_flag = False
#print(args)

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--decay", help="Rate at which the outside phosphors decay. Ranges from (-infinity, 1). Default is 0.25", type=float, default=0.25)
parser.add_argument("-n", "--num", help="Number of dots tracing around the outside of the clock. Default is 4.", type=int, default=4)
parser.add_argument("-t", "--time", help="Time it takes for a dot to go around the circle. Default is 4 seconds", type=float, default=4)
parser.add_argument("-i", "--inner-color", help="RGB Color of the clockhands. Format is RRGGBB, or standard hexadecimal color notation", type=str, default="39FF14")
parser.add_argument("-o", "--outer-color", help="RGB Color of the outside of the clock. Format is RRGGBB, or standard hexadecimal color notation.\nNote: Only makes use of the hue and saturation components, does not use value", type=str, default="A3FFF9")

args = parser.parse_args()

try:
    decay = args.decay
    time_scale = args.time
    num_of_outside_points = args.num
    hand_base_color = parse_hex(args.inner_color)
    outside_base_color = parse_hex(args.outer_color)
    outside_base_color = colorsys.rgb_to_hsv(outside_base_color[0] / 256, outside_base_color[1] / 256, outside_base_color[2] / 256)[0:2]
except:
    print("One or more of your arguments were invalid. Use \"-h\" for proper usage")

smfb = sysv_ipc.SharedMemory(SHARED_MEMORY_KEY, flags = sysv_ipc.IPC_CREAT, mode = 0o666, size = (width * height * 3) + 1)

class color:
    def __init__(self, red: int, green: int, blue: int, lifespan: float, time_created: float):
        self.red = red
        self.green = green
        self.blue = blue
        self.lifespan = lifespan
        self.time_created = time_created
        return
    
    def mul(self, other: float, current_time: float):
        time_elapsed = current_time - self.time_created
        time_remaining = self.lifespan - time_elapsed
        return color(self.red * other, self.green * other, self.blue * other, time_remaining, current_time)

    def get_current_color(self, current_time: float):
        time_elapsed = current_time - self.time_created
        if (time_elapsed > self.lifespan):
            return False
        time_remaining = self.lifespan - time_elapsed
        time_remaining *= 1 / self.lifespan
        return self.mul(time_remaining, current_time)
    


def kill_signal(signum, frame):
    global kill_flag
    kill_flag = True

def build_circle() -> List[Tuple[int, int]]:
    global width
    global height
  
    #Calculate constants
    t_center = [(width - 1) / 2, (height - 1) / 2]
    top_point = [width / 2, 0] # all the RGB matrices have the longer dimension be width
    radius = math.dist(t_center, top_point)

    #Calculate initial 1/8th sweep
    output_list = []
    x, y = 0.5, 0
    while (True):
        next_point = (x, (radius ** 2 - x ** 2) ** 0.5)
        y = next_point[1]
        if (y < x):
            break
        output_list.append(next_point)
        x += 1

    #print(type(output_list))
    #Reflect across y = x
    output_list += list(map(lambda x: (x[1], x[0]), output_list))

    #Reflect across x axis
    output_list += list(map(lambda x: (-1 * x[0], x[1]), output_list))
  
    #Reflect across y axis
    temp_list = list(map(lambda x: (x[0], -1 * x[1]), output_list))
    temp_list.reverse()
    output_list += temp_list
    #print(len(output_list))
  
    output_list = sorted(output_list, key=lambda x: math.atan2(x[1], x[0]), reverse = True)
    #translate
    output_list = map(lambda x: (x[0] + t_center[0], x[1] + t_center[1]), output_list)

    #discretize
    output_list = map(lambda x: (int(round(x[0])), int(round(x[1]))), output_list)
  
    return list(output_list)

circle = build_circle()



def draw_outside(frame_buffer: bytearray) -> None:
    BASE_COLOR = 176/360
    circle_brightness_dict = {}
    def circle_brightness_func(input: float, num: int, decay: float, scaling_factor = 1.0) -> float:
        def max (x, y):
            return x if (x > y) else y   
        output = max(((num * scaling_factor * input) % scaling_factor) - scaling_factor * decay, 0) * (1/(1-decay))
        return output
    ms_index = time.clock_gettime(clk_id1) * -1
    ms_index /= time_scale
    ms_index %= 1
    for i in range(len(circle)):
        circle_brightness_dict[circle[i]] = circle_brightness_func(1 - (i/len(circle)) + ms_index, num_of_outside_points, decay) #add in time based offset
    
    offset_constant = width * height
    for i in circle_brightness_dict:
        red, green, blue = colorsys.hsv_to_rgb(outside_base_color[0], outside_base_color[1], circle_brightness_dict[i])
        w = i[0]
        h = i[1]
        frame_buffer[h * width + w] = int(red * 255)
        frame_buffer[h * width + w + offset_constant] = int(green * 255)
        frame_buffer[h * width + w + (offset_constant * 2)] = int(blue * 255)
    return

def draw_hands(frame_buffer: bytearray): 
    DEGREES_TO_RADIANS_FACTOR = (2 * math.pi) / 360
    current_datetime = time.localtime()
    current_computertime = time.clock_gettime(clk_id1)
    global hands_dict
    MINUTE_HAND_LEN = 12
    HOUR_HAND_LEN = 8
    MINUTE_HAND_REDRAW_RATE = 0.65
    HOUR_HAND_REDRAW_RATE = 0.8
    MINUTE_HAND_DECAY_RATE = 0.8
    HOUR_HAND_DECAY_RATE = 1
    center = ((width - 1) / 2, (height - 1) / 2)
    current_mins = int(time.strftime("%M", current_datetime)) + (int(time.strftime("%S", current_datetime)) / 60)
    current_hrs = int(time.strftime("%I", current_datetime)) + (current_mins / 60)

    min_degrees = 360 - (current_mins * 360 / 60)
    hr_degrees = 360 - (current_hrs * 360 / 12)
    min_len = (current_computertime % MINUTE_HAND_REDRAW_RATE) * (MINUTE_HAND_LEN / MINUTE_HAND_REDRAW_RATE)
    hr_len = (current_computertime % HOUR_HAND_REDRAW_RATE) * (HOUR_HAND_LEN / HOUR_HAND_REDRAW_RATE)
    new_min_point = [min_len * math.sin(min_degrees * DEGREES_TO_RADIANS_FACTOR), min_len * math.cos(min_degrees * DEGREES_TO_RADIANS_FACTOR)]
    new_hr_point = [hr_len * math.sin(hr_degrees * DEGREES_TO_RADIANS_FACTOR), hr_len * math.cos(hr_degrees * DEGREES_TO_RADIANS_FACTOR)]

    new_min_point[0] += center[0]
    new_min_point[1] += center[1]
    new_hr_point[0] += center[0]
    new_hr_point[1] += center[1]

    #print("Min = " + str(current_mins))
    #print("Hour = " + str(current_hrs))

    new_min_point[0] = int(new_min_point[0])
    new_min_point[1] = int(new_min_point[1])
    new_hr_point[0] = int(new_hr_point[0])
    new_hr_point[1] = int(new_hr_point[1])

    hands_dict[tuple(new_min_point)] = color(hand_base_color[0],hand_base_color[1],hand_base_color[2], MINUTE_HAND_DECAY_RATE, current_computertime)
    hands_dict[tuple(new_hr_point)] = color(hand_base_color[0],hand_base_color[1],hand_base_color[2], HOUR_HAND_DECAY_RATE, current_computertime)

    offset_constant = width * height
    for i in hands_dict:
        w = i[0]
        h = i[1]
        new_color = hands_dict[i].get_current_color(current_computertime)
        if (new_color == False):
            del i
            continue
        frame_buffer[(height - h) * width + (width - w)] = int(new_color.red)
        frame_buffer[(height - h) * width + (width - w) + offset_constant] = int(new_color.green)
        frame_buffer[(height - h) * width + (width - w) + offset_constant * 2] = int(new_color.blue)
    return

def draw():
    frame_buffer = bytearray(width * height * 3 + 1)
    draw_outside(frame_buffer)
    draw_hands(frame_buffer)
    smfb.write(frame_buffer, 0)

signal.signal(signal.SIGTERM, kill_signal)
signal.signal(signal.SIGINT, kill_signal)
last_iteration = 0
while (not kill_flag):
    draw()
    time.sleep(0.01) 
smfb.write(bytearray(width * height * 3), 0)