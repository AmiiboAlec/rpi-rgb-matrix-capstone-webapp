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

#define shared_mem_key 25375

unsigned char *smfb = NULL;
int width = 64, height = 32;
int sock, conn;
int killflag = 0;
std::ofstream output("cppoutput.txt");

struct rgbpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

std::vector<std::string> split_text(std::string &input, char delimiter) {
    //output << "Splitting\n" << input << std::endl;
    std::stringstream reader(input);
    std::vector<std::string> output;
    for (std::string line; std::getline(reader, line, delimiter);) {
        output.push_back(line);
    }
    return output;
}

void write_screen(std::string &inst) {
    //output << "Writing " << inst << std::endl; 
    unsigned char *r = smfb;
    unsigned char *g = smfb + (width * height);
    unsigned char *b = smfb + (width * height * 2);
    int x, y;
    rgbpixel color;
    sscanf(inst.c_str(), "Pixel %d,%d: %2hhX%2hhX%2hhX", &x, &y, &color.r, &color.g, &color.b);
    int true_index = y * width + x;
    r[true_index] = color.r;
    g[true_index] = color.g;
    b[true_index] = color.b;
}

void sighandler(int signum) {
    #define BLOCK_SIZE 1024
    static std::mutex int_loc;
    std::string x = "";
    if (int_loc.try_lock()) {
retry:
        char buff[BLOCK_SIZE + 1];
        while (true) {
            buff[BLOCK_SIZE] = '\0';
            int bytes_read = read(sock, buff, 1024);
            buff[bytes_read] = '\0'; //manually null terminating
            x += buff;
            if (bytes_read != BLOCK_SIZE) {
                break;
            }
        }
        auto messages = split_text(x, '\a');
        for (auto msg : messages) {
            write_screen(msg);
        }
        int data_in_socket;
        ioctl(sock, FIONREAD, &data_in_socket);
        if (data_in_socket == 0) {
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

void close_gracefully(int signum) {
    output << "Handled Sigterm" << std::endl;
    killflag = 1;
}

int main (int argc, char **argv) {
    //set up signal handlers
    output << "Is this file even running" << std::endl;
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

    //attach shared memory
    key_t key = shared_mem_key;
    int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT);
    smfb = (unsigned char*) shmat(shmid, NULL, 0);

    while (!killflag) { //replace with pause();
        sleep(1);
    }
    //close(sock);
    output << "Closed socket" << std::endl;
    //shmdt(smfb);
    output.close();
    return 0;
}