#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "fontpack.hpp"
#include <chrono>
#include <cmath>
#include <sstream>
#include <signal.h>
#include <thread>
#include <memory>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

#define shared_mem_key 25375

constexpr int text_height = 8;
constexpr int text_width = 6;

unsigned char *smfb;
unsigned int width = 64;
unsigned int height = 32;
bool kill_flag = false;

std::chrono::time_point<std::chrono::system_clock> start_of_program;

struct rgbpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

std::vector<rgbpixel> string_to_pixels(const std::string &src) {
    std::vector<rgbpixel> output;
    std::stringstream reader(src);
    for (std::string line; std::getline(reader, line, '|');) {
        rgbpixel temp;
        sscanf(line.c_str(), "%2hhX%2hhX%2hhX", &temp.r, &temp.g, &temp.b);
        output.push_back(temp);
    }
    return output;
}

class text_row {
    private:
    std::string text;
    std::vector<rgbpixel> colors;
    std::vector<rgbpixel> background_colors;
    float scroll_speed; // in pixels per second
    float current_scroll_value; // might remove later
    int row_number;

    public:

    void draw_row() {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        auto time_passed = std::chrono::duration<double>(now - start_of_program).count();
        auto draw_column = [this] (int start, unsigned char bitmap, rgbpixel color, rgbpixel bg_color) {
            //printf("%d, %hhX\n", start, bitmap);
            for (int i = 0; i < text_height; i++) {
                //if (start >= (row_number + 1) * width * text_height) break;
                bool is_pixel_on;
                is_pixel_on = bitmap & (0x80);
                bitmap <<= 1;
                if (is_pixel_on) {
                    smfb[start] = color.r;
                    smfb[start + (width * height)] = color.g;
                    smfb[start + (width * height * 2)] = color.b;
                }
                else {
                    smfb[start] = bg_color.r;
                    smfb[start + (width * height)] = bg_color.g;
                    smfb[start + (width * height * 2)] = bg_color.b;
                }
                start += width;
            }
        };
        for (int i = 0; i < width; i++) {
            unsigned int index_into_pixel_string = scroll_speed * time_passed + i;
            index_into_pixel_string %= text.length() * text_width;
            int index_into_char_string = index_into_pixel_string / text_width;
            int index_into_font = index_into_pixel_string % text_width;
            char char_to_draw = text[index_into_char_string];
            auto bitmap = fontpack[char_to_draw];
            draw_column(i + (row_number * width * text_height), bitmap[index_into_font], colors[index_into_char_string], background_colors[index_into_char_string]);
        }
    }


    text_row(const std::string &msg, int row_num) {
        row_number = row_num;
        std::stringstream reader(msg);
        std::getline(reader, text, '\t');
        std::string color_string;
        std::getline(reader, color_string, '\t');
        std::string background_color_string;
        std::getline(reader, background_color_string, '\t');
        std::string scroll_string;
        std::getline(reader, scroll_string, '\t');

        colors = string_to_pixels(color_string);
        background_colors = string_to_pixels(background_color_string);

        scroll_speed = std::atof(scroll_string.c_str());
    }

    text_row() {}

};

void exit_gracefully(int signum) {
    kill_flag = true;
}

//TEXT \t HEX COLORS \t SCROLL VALUE
int main (int argc, char **argv) {
    bool test_mode = false;
    key_t key = shared_mem_key;
    int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT);
    smfb = (unsigned char*) shmat(shmid, NULL, 0);
    signal(SIGTERM, exit_gracefully);
    signal(SIGINT, exit_gracefully);

    struct sockaddr saddr = {AF_UNIX, '\0'};
    socklen_t saddrlen = sizeof(struct sockaddr);
    strncpy(saddr.sa_data, "child_sock", 13);
    saddr.sa_data[13] = '\0';
    //int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    //int conn = connect(sock, &saddr, saddrlen);
    start_of_program = std::chrono::system_clock::now();
    int number_of_rows = height / text_height;
    std::unique_ptr<text_row[]> rows = std::make_unique<text_row[]>(number_of_rows);
    if (test_mode) for (int i = 0; i < number_of_rows; i++) {
        char buff[100];
        sprintf(buff, "%s\t%s\t%s\t%f", "Test", "FFFFFF|FFFF00|FF00FF|FF0000", "00FFFF|00FF00|0000FF|000000", (float) i);
        rows[i] = text_row(buff, i);
    }
    else for (int i = 0; i < number_of_rows; i++) {
        rows[i] = text_row(argv[i + 1], i);
    }

    while (!kill_flag) {
        auto start_of_loop = std::chrono::system_clock::now();
        auto milliseconds_per_frame = std::chrono::milliseconds(10);
        for (int i = 0; i < number_of_rows; i++) {
            rows[i].draw_row();
        }
        std::this_thread::sleep_for(milliseconds_per_frame - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_of_loop));
    }
    for (int i = 0; i < width * height * 3; i++) {
        smfb[i] = '\0';
    }
    //close(sock);
}