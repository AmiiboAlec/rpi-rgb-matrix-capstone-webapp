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
#include "rgbpixel.hpp"

#define shared_mem_key 25375

unsigned char *smfb = NULL;
int width = 64, height = 32;
int sock, conn;
int killflag = 0;
//std::ofstream output("cppoutput.txt");
int delay;

class frameList {
    class frame{
        protected:
        std::unique_ptr<unsigned char[]> datalist;
        frame() {}
        private:
        public:
        frame(const std::string &data) {
            datalist = std::make_unique<unsigned char[]>(width * height * 3);
            unsigned char *r = datalist.get();
            unsigned char *g = r + (width * height);
            unsigned char *b = g + (width * height);
            std::stringstream reader(data);
            int counter = 0;
            for (std::string line; std::getline(reader, line, '|'); ) {
                rgbpixel color;
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

    class indexed_frame : public frame {
        std::unique_ptr<rgbpixel[]> palette;
        indexed_frame(){}
        public:
        indexed_frame(const std::string& data) {
            std::unordered_map<rgbpixel, unsigned char> palette_constructor;
            datalist = std::make_unique<unsigned char[]>(width * height);
            std::stringstream reader(data);
            int counter = 0;
            int palette_counter = 0;
            for (std::string line; std::getline(reader, line, '|'); ) {
                rgbpixel color;
                sscanf(line.c_str(), "%2hhX%2hhX%2hhX", &color.r, &color.g, &color.b);
                try {
                    auto index = palette_constructor.at(color);
                    datalist[counter] = index;
                }
                catch (...) {
                    if (palette_counter == 255) throw std::runtime_error("Too many colors");
                    palette_constructor[color] = palette_counter++;
                    auto index = palette_constructor.at(color);
                    datalist[counter] = index;
                }
                counter++;
            }
            palette = std::make_unique<rgbpixel[]>(palette_counter);
            for (const auto &a : palette_constructor) {
                palette[a.second] = a.first;
            }
        }

        void draw() {
            const int len = width * height;
            for (int i = 0; i < width * height; i++) {
                const rgbpixel p = palette[datalist[i]];
                smfb[i] = p.r;
                smfb[i + len] = p.g;
                smfb[i + (len * 2)] = p.b;
            }
        }
    };
    std::vector<std::unique_ptr<frame>> list;
    int framePointer;
    public:
    frameList() {}

    ~frameList() {}

    void draw() {
        if (++framePointer >= list.size()) framePointer = 0;
        list[framePointer]->draw();
    }

    void append_frame(std::string &frameData) {
        try {
            list.push_back(std::make_unique<indexed_frame>(frameData));
        }
        catch (...) {
            list.push_back(std::make_unique<frame>(frameData));
        }
    }

    bool ready() {
        return list.size() != 0;
    }
};

frameList list;

std::vector<std::string> split_text(const std::string &input, char delimiter) {
    //output << "Splitting\n" << input << std::endl;
    std::stringstream reader(input);
    std::vector<std::string> output;
    for (std::string line; std::getline(reader, line, delimiter);) {
        if (line != "") output.push_back(line);
    }
    return output;
}

void sockhandler(int signum) {
    static std::mutex int_loc;
    if (int_loc.try_lock()) {
        int bytes_remaining;
        std::vector<std::string> messages;
        {
retry:
        std::ostringstream x("");
        ioctl(sock, FIONREAD, &bytes_remaining);
        std::unique_ptr<char[]> buff = std::make_unique<char[]>(bytes_remaining + 1); //this should zero-initialize the array
        int bytes_read = read(sock, buff.get(), bytes_remaining); 
        x << buff.get();
        if (buff[bytes_remaining - 1] != '\a') goto retry;
        messages = split_text(x.str(), '\a');
        }
        
        for (auto message : messages) list.append_frame(message);

        ioctl(sock, FIONREAD, &bytes_remaining);
        if (bytes_remaining == 0) {
            int_loc.unlock();
            return;
        }
        else goto retry;
    }
    else return;
}

void mainloop() {
    int counter = 0;
    while (!killflag) {
        counter++;
        auto beginning_time = std::chrono::steady_clock::now();
        if (list.ready()) list.draw();
        std::this_thread::sleep_until(beginning_time + std::chrono::milliseconds(delay));
    }
}

void close_gracefully(int signum) {
    killflag = 1;
}

int main (int argc, char **argv) {
    //set up signal handlers
    signal(SIGIO, sockhandler);
    signal(SIGTERM, close_gracefully);

    int args = sscanf(argv[1], "Delay: %d", &delay);

    if (args == 1) {
        //open socket
        struct sockaddr saddr = {AF_UNIX, '\0'};
        socklen_t saddrlen = sizeof(struct sockaddr);
        strncpy(saddr.sa_data, "child_sock", 13);
        saddr.sa_data[13] = '\0';
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        conn = connect(sock, &saddr, saddrlen);
    }
    else {
        if (access(argv[1], F_OK) == 0){
            auto f = std::unique_ptr<FILE, decltype(&fclose)>(fopen(argv[1], "r"), &fclose);
            fseek(f.get(), 0L, SEEK_END);
            int size = ftell(f.get());
            fseek(f.get(), 0L, SEEK_SET);
            std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size + 1);
            fread(buffer.get(), size, 1, f.get());
            auto messages = split_text(std::string(buffer.get()), '\a');
            for (auto message : messages) list.append_frame(message);
        }
        else {
            std::cerr << argv[1] << " is not a valid file.\n";
            return 1;
        }
    }

    //attach shared memory
    key_t key = shared_mem_key;
    int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT);
    smfb = (unsigned char*) shmat(shmid, NULL, 0);

    mainloop();
    
    close(sock);
    //output << "Closed socket" << std::endl;
    //shmdt(smfb);
    //output.close();
    return 0;
}