import sys
from PIL import Image
from os import path

im = Image.open(sys.argv[1])
pix = im.load()
ext = path.splitext(sys.argv[1])
output = open(ext[0] + ".txt", "w")
out_list = []
width, height = im.size
for y in range(height): 
    for x in range(width):
        r,g,b = pix[x,y]
        out_list.append("%02X%02X%02X" % (r, g, b))
output.write('|'.join(out_list))
output.close()