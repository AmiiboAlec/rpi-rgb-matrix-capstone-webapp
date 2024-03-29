#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <fcntl.h>
#include <string>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>

#define shared_mem_key 25375

unsigned char *smfb = NULL;
int width = 64, height = 32;
int sock, conn;
int killflag = 0;
std::ofstream output("cppoutput.txt");
int delay;

struct rgbpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class frameList {
    class frame{
        std::unique_ptr<unsigned char[]> datalist;
        
        public:
        frame(std::string &data) {
            datalist = std::make_unique<unsigned char[]>(width * height * 3);
            unsigned char *r = datalist.get();
            unsigned char *g = r + (width * height);
            unsigned char *b = g + (width * height);
            std::stringstream reader(data);
            int counter = 0;
            for (std::string line; std::getline(reader, line, '|'); ) {
                register rgbpixel color;
                sscanf(line.c_str(), "%2hhX%2hhX%2hhX", &color.r, &color.g, &color.b);
                r[counter] = color.r;
                g[counter] = color.g;
                b[counter] = color.b;
                counter++;
                if (counter > (width * height)) break;
            }
        }
        void draw() {
            memcpy(smfb, datalist.get(), width * height * 3);
        }
    };
    std::vector<frame> list;
    int framePointer;
    public:
    frameList() {}

    ~frameList() {}

    void draw() {
        if (++framePointer >= list.size()) framePointer = 0;
        list[framePointer].draw();
    }

    void append_frame(std::string &frameData) {
        list.push_back(frame(frameData));
    }

    bool ready() {
        return list.size() != 0;
    }
};

frameList list;

std::vector<std::string> split_text(std::string &input, char delimiter) {
    //output << "Splitting\n" << input << std::endl;
    std::stringstream reader(input);
    std::vector<std::string> output;
    for (std::string line; std::getline(reader, line, delimiter);) {
        if (line != "") output.push_back(line);
    }
    return output;
}

void sighandler(int signum) {
    output << "Message Caught" << std::endl;
    #define BLOCK_SIZE 2003
    static std::mutex int_loc;
    std::string x = "";
    if (int_loc.try_lock()) {
retry:
        output << "locked mutex" << std::endl;
        int bytes_remaining;
        char buff[BLOCK_SIZE + 1];
        
        ioctl(sock, FIONREAD, &bytes_remaining);
        while (bytes_remaining != 0) {
            ioctl(sock, FIONREAD, &bytes_remaining);
            std::cout << bytes_remaining << "bytes remaining" << std::endl;
            buff[BLOCK_SIZE] = '\0';
            int bytes_read = read(sock, buff, BLOCK_SIZE);
            buff[bytes_read] = '\0'; //manually null terminating
            x += buff;
            output << x.size() << std::endl;
            if (bytes_read != BLOCK_SIZE) {
                break;
            }
        }
        auto messages = split_text(x, '\a');
        
        for (auto message : messages) {
            //output << "Split message:\n" << message << std::endl;
            list.append_frame(message);
        }

        int data_in_socket;
        ioctl(sock, FIONREAD, &data_in_socket);
        if (data_in_socket == 0) {
            output << "STILL " << data_in_socket << "LEFT TO READ" << std::endl;
            int_loc.unlock();
            return;
        }
        else {
            x = "";
            goto retry;
        }
        
    }
    else return;
    
    //send(sock, x.c_str(), x.size(), 0);
    //kill(getppid(), SIGUSR1);
}

void mainloop() {
    output << "Entered Main Loop" << std::endl;
    int counter = 0;
    while (!killflag) {
        counter++;
        auto beginning_time = std::chrono::steady_clock::now();
        if (list.ready()) list.draw();
        if (counter == 100) {
            output << "List.ready = " << list.ready() << std::endl;
        }
        std::this_thread::sleep_until(beginning_time + std::chrono::milliseconds(delay));
        //output << "got through main loop" << std::endl;
    }
}

void close_gracefully(int signum) {
    output << "Handled Sigterm" << std::endl;
    killflag = 1;
}

int main (int argc, char **argv) {
    //set up signal handlers
    signal(SIGUSR1, sighandler);
    signal(SIGTERM, close_gracefully);

    //establish connection with websocket manager
    output << "Signals registered" << std::endl;
    struct sockaddr saddr = {AF_UNIX, '\0'};
    socklen_t saddrlen = sizeof(struct sockaddr);
    strncpy(saddr.sa_data, "child_sock", 13);
    saddr.sa_data[13] = '\0';
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    conn = connect(sock, &saddr, saddrlen);
    output << "Socket Connected" << std::endl;

    sscanf(argv[1], "Delay: %d", &delay);

    //attach shared memory
    key_t key = shared_mem_key;
    int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT);
    smfb = (unsigned char*) shmat(shmid, NULL, 0);

    mainloop();
    
    //close(sock);
    output << "Closed socket" << std::endl;
    //shmdt(smfb);
    output.close();
    return 0;
}