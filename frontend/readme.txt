Cano's RGB Matrix webapp x comes with ABSOLUTELY NO WARRANTY, to the extent permitted by applicable law.

The simplest of all the modes is the live preview mode. When the live preview mode runs, it attempts to connect to a unix domain socket named "child sock", from which it expects to receive image data formatted as the following sscanf format:
"Pixel %d,%d: %2hhX%2hhX%2hhX". For example, "Pixel 5,4: ABCDEF". The %2hhX is assigning a 2 digit hexadecimal number to an unsigned char. It then updates the shared memory frame buffer one pixel at a time. This mode updates the display relatively slowly.

The image/animation mode requires slightly more setup. 
It will look for a command line parameter telling what the delay between frames on the gif is, in milliseconds. 
It still requires this even for a static image; for a JPG or PNG the webapp defaults to 100 milliseconds.
Then, it also connects to "child sock" and waits to receive image data. However, since it is receiving full frames, it uses a more efficient format.
Each pixel is encoded as a hex code similar to above, each pixel is delimited by a '|', and each frame is delimited by a newline. 
The application will simply loop over whatever frames it has loaded, so there's no need for a terminator.

None of the other appliactions require as much initialization information as those two. As such, they don't read from the socket at all, they just initialize from CLI args.
The text mode has 4 pieces of information it must transmit per line.
Text to be displayed, foreground color, background color, and scroll speed.
The text is encoded as-is, no special formatting necessary.
The two color values are encoded as they are in the image animation mode, with hex colors delimited by the vertical line character.
And the scroll speed is encoded as a decimal number
Each of these values is delimited by a tab character, and each row is delimited by a newline.

The clock mode is more designed for CLI use than all the other modes, using getopt for all the configuration. Run the application with -h to have it print a list of all available configuration options.