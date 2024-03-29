#include <string>
#include "rgbpixel.hpp"
#include "read-config.cpp"
#include "sock_funcs.cpp"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>
#include <sstream>
#include <signal.h>
#include <mutex>
#include <fstream>
#include <vector>
#include <unistd.h>

#define shared_mem_key 25375

unsigned char* smfb; 
int width, height;
int sock;
bool kill_flag = false;
std::ifstream in_fifo;
std::ofstream out_fifo;

/*class pseudo_istream {
    constexpr int buff_len = 1024;
    char buffer[buff_len];
    int read_pointer;
    int fd;

    void copyback_readbuffer () {
        memmove(buffer, &buffer[read_pointer], buff_len - read_pointer);
        read_pointer = 0;
    }

    public:
    psuedo_istream(int filedesc) {
        fd = filedesc;
        read_pointer = 0;
        for (int i = 0; i < buff_len; i++) buffer[i] = '\0';
    }

    std::string readmsg() {
        int start = read_pointer;
        int end = -1;
        int i;
        for (i = read_pointer; i < buff_len; i++) {
            if (buffer[i] == '\n') {
                end = i + 1;
                break;
            }
            if (buffer[i] == '\0') {
                read(fd, &buffer[read_pointer], buff_len - read_pointer); 
            };
        }
    }
};*/

void terminate (int signum) {
    kill_flag = true;
}

void readData(int signum) {
    static std::mutex d_lock;
    if (d_lock.try_lock()) {
        in_fifo.clear();
        std::vector<std::string> list;
        for (std::string line; std::getline(in_fifo, line);) {
            list.push_back(line);
        }
        d_lock.unlock();
        for (auto a : list) {
            update_pix(a);
        }
    }
    else {
        return;
    }
}

void update_pix (std::string msg) {
    int length = width * height;
    unsigned char *r = smfb;
    unsigned char *g = smfb + length;
    unsigned char *b = g + length;
    int x, y;
    rgbpixel color;
    sscanf(msg.c_str(), "Pixel %d,%d: %2hhX%2hhX%2hhX", &x, &y, &color.r, &color.g, &color.b);
    int true_index = (y * width) + x;
    r[true_index] = color.r; g[true_index] = color.g; b[true_index] = color.b;
    
}

int main (int argc, char **argv) {
    auto temp = read_config_file();
    width = temp.width;
    height = temp.height;

    auto key = shared_mem_key;
    int shmid = shmget(key, width * height * 3, 0666 | IPC_CREAT);
    smfb =  (unsigned char*) shmat(shmid, NULL, 0);

    signal(SIGTERM, terminate);
    signal(SIGUSR1, readData);
    
    while (!kill_flag) sleep(1);
    close(sock);
    shmdt(smfb);
    return 0;
}