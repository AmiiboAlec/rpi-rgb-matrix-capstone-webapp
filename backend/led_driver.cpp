#include <iostream>
#include "read-config.cpp"
//#include "rgbpixel.hpp"
#include "rpi-rgb-led-matrix/include/led-matrix.h"
#include "sock_funcs.cpp"
#include <chrono>
#include <thread>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include <fstream>

#define shared_mem_key 25375
unsigned int width = 64, height = 32;
unsigned char* smfb;
rgb_matrix::RGBMatrix *canvas;
bool killflag = false; //used for terminating gracefully, this functionality not yet implemented
std::ofstream debug_output;

void terminate(int signum) {
    killflag = true;
}

void main_loop() {
    debug_output << "Entered Main Loop" << std::endl;
    unsigned char *r = smfb;
    unsigned char *g = smfb + (width * height);
    unsigned char *b = smfb + (width * height * 2);

    while (!killflag) {
        auto beginning_time_point = std::chrono::system_clock::now();
        for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) 
            canvas->SetPixel(x, y, r[(y * width) + x], g[(y * width) + x], b[(y * width) + x]);
        std::this_thread::sleep_until(beginning_time_point + std::chrono::milliseconds(10));
    }

    /*unsigned char *copyarray = (unsigned char*) malloc((width * height * 3) + 1);
    for (int i = 0; i < (width * height * 3) + 1; i++) {
        copyarray[i] = 0;
    }
    debug_output << "Filled aux array with NULL" << std::endl;
    while (!killflag) {
        auto beginning_time_point = std::chrono::system_clock::now();
        for (int i = 0; i < (width * height * 3) + 1; i++) {
            if (copyarray[i] != smfb[i]) {
                debug_output << "Index " << i << " was altered" << std::endl;
                copyarray[i] = smfb[i];
            }
        }
        std::this_thread::sleep_until(beginning_time_point + std::chrono::milliseconds(1));
    }
    free(copyarray);*/
}

int main (int argc, char **argv) {
    using std::string; using namespace rgb_matrix;
    debug_output.open("driver_output.txt");
    //auto def_config = read_config_file();
    //width = def_config.width;
    //height = def_config.height;
    debug_output << "file opened" << std::endl;
    //create shared memory frame buffer
    key_t key = shared_mem_key;
    int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT); //The final byte of this will be used as a replacement for SIGTERM
    smfb = (unsigned char*) shmat(shmid, NULL, 0); 
    for (int i = 0; i < (width * height * 3) + 1; i++) {
        smfb[i] = 0;
    }
    debug_output << "Created Shared Memory area" << std::endl;
    //int sock = make_outgoing_socket("toplevel"); //will pause until the other side of the socket has connected. important this is executed after declaring the shared memory region

    //register signals
    signal(SIGTERM, terminate);
    debug_output << "Registered signal" << std::endl;

    //create matrix
    RGBMatrix::Options def_options;
    def_options.brightness = 100;
    def_options.chain_length = 1;
    def_options.cols = width;
    def_options.rows = height;
    def_options.hardware_mapping = "adafruit-hat-pwm";
    RuntimeOptions runtimeopts;

    if (!ParseOptionsFromFlags(&argc, &argv, &def_options, &runtimeopts)) {
        std::cout << "error\n";
        return 1;
    }

    canvas = CreateMatrixFromFlags(&argc, &argv, &def_options, &runtimeopts);

    main_loop();

    debug_output << "Signal handled" << std::endl;
    //close(sock);
    remove("toplevel");
    shmdt(smfb);
    shmctl(shmid, IPC_RMID, NULL);
}