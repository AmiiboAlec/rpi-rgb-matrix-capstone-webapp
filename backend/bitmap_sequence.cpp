#include <memory> //used for std::unique_ptr
#include <vector> //used in several places
#include "read-config.cpp" 
#include "rgbpixel.hpp"
#include "sock_funcs.cpp"
#include <fcntl.h> //used for mmap symbols
#include <sys/socket.h> //used for sockets
#include <unistd.h>
#include <chrono> //used for std::this_thread.sleep_for()
#include <thread> //used for std::this_thread.sleep_for()
#include <sstream> //used for message parser
#include <map> //used for indexed color mode
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

unsigned int width, height;
unsigned char *smfb;
int num_of_frames = -1;
unsigned int delay = 100; //in milliseconds. assume 100 if not specified
int sock;
bool kill_flag = false;

void terminate(int signum) {
    kill_flag = true;
}

class colorList {
    std::unique_ptr<unsigned char[]> list;
    int length;
    bool indexed;

    public:
    colorList(const std::vector<rgbpixel> &clist) {
        indexed = false;
        list = std::make_unique<unsigned char[]>(clist.size() * 3);
    }

    rgbpixel operator[] (const int index) {
        return {list[index], list[index + length], list[index + (length * 2)]};
    }

    void draw() {
        if (!indexed) {
            memcpy(smfb, list.get(), width * height * 3);
        }
    }
};

std::vector<colorList> frames;

void onmessage(const std::string &message) {
    std::stringstream reader(message);
    std::vector<rgbpixel> pixlist;
    for (std::string line; std::getline(reader, line, '|');) {
        rgbpixel x;
        sscanf(line.c_str(), "%2hhX%2hhX%2hhX", &x.r, &x.g, &x.b);
        pixlist.push_back(x);
    }
    frames.push_back(colorList(pixlist));
}

int main (int argc, char **argv) {
    auto def_config = read_config_file();

    width = def_config.width;
    height = def_config.height;

    signal(SIGTERM, terminate);

    sock = connect_incoming_socket("bottomlevel");
    auto key = ftok("smfb", 65);
    int shmid = shmget(key, width * height * 3, 0666 | IPC_CREAT);
    smfb = (unsigned char*) shmat(shmid, NULL, 0);
    
    {
        char buff[1024];
        read(sock, buff, 1023);
        buff[1023] = '\0';
        sscanf(buff, "Frames: %u\nDelay: %u", &num_of_frames, &delay);
    }

    char *recv_buffer = (char*) malloc(width * height * 8);
    while (num_of_frames != frames.size()) {
        read(sock, recv_buffer, (width * height * 8) - 1);
        recv_buffer[(width * height * 8) - 1] = '\0'; //manual null termination, just in case the read command goes to the end of the string
        onmessage(std::string(recv_buffer));
    }
    
    int frameCounter = 0;
    while (!kill_flag) {
        auto beginning = std::chrono::system_clock::now();
        if (++frameCounter == frames.size()) frameCounter = 0;
        frames[frameCounter].draw();
        std::this_thread::sleep_until(beginning + std::chrono::milliseconds(delay));
    }

    close(sock);
    shmdt(smfb);
    return 0;
}