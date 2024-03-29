#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>
#include <string>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <thread>
#include <cmath>
#include <chrono>
#include <sys/ipc.h>
#include <sys/shm.h>

#define shared_mem_key 25375

using namespace std::chrono_literals;

int width = 64; int height = 32;
unsigned char *smfb = NULL;
int killflag = 0;

struct rgbpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

rgbpixel HSVtoRGB(double H, double S,double V){
    if(H>360 || H<0 || S>100 || S<0 || V>100 || V<0){
        std::cout<<"The givem HSV values are not in valid range"<< std::endl;
        return {0,0,0};
    }
    double s = S/100;
    double v = V/100;
    double C = s*v;
    double X = C*(1-abs(fmod(H/60.0, 2)-1));
    double m = v-C;
    double r,g,b;
    if(H >= 0 && H < 60){
        r = C,g = X,b = 0;
    }
    else if(H >= 60 && H < 120){
        r = X,g = C,b = 0;
    }
    else if(H >= 120 && H < 180){
        r = 0,g = C,b = X;
    }
    else if(H >= 180 && H < 240){
        r = 0,g = X,b = C;
    }
    else if(H >= 240 && H < 300){
        r = X,g = 0,b = C;
    }
    else{
        r = C,g = 0,b = X;
    }
    int R = (r+m)*255;
    int G = (g+m)*255;
    int B = (b+m)*255;
    return {R,G,B};
}

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}

rgbpixel convert(rgb in) {
    rgbpixel out;
    out.r = in.r * 255;
    out.g = in.g * 255;
    out.b = in.b * 255;
    return out;
}

void main_loop() {
    using namespace std::chrono;
    double xy_scale_factor = 1/20.0;
    for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) {
        auto current_ms = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
        double current_seconds = current_ms / 1000000000.0;
        rgbpixel value = HSVtoRGB(fmod(((x * xy_scale_factor) + (y * xy_scale_factor) + current_seconds), 1) * 360, 100, 100); //change to new function
        int pixelIndex = y * width + x;
        int planeIndex = height * width;

        //smfb[pixelIndex] = value.r;
        //smfb[pixelIndex + planeIndex] = value.g;
        //smfb[pixelIndex + planeIndex + planeIndex] = value.b;
    }
}

void stop_loop(int signum) {
    killflag = 1;
}

int main (int argc, char **argv) {
    signal(SIGTERM, stop_loop);
    //key_t key = shared_mem_key;
    //int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT);
    //smfb = (unsigned char*) shmat(shmid, NULL, 0);
    while (!killflag) {
        main_loop();
        std::this_thread::sleep_for(10ms);
    }
}