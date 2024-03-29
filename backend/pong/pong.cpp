#include <iostream>
#include <utility>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>

#include "objectManager.hpp"
#include "ball.hpp"
#include "paddle.hpp"

#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8

int width = 64;
int height = 32;
unsigned char *smfb;
unsigned char *double_buffer;
int fps = 100;

objectManager objManager;

int main (int argc, char **argv) {
    constexpr int paddle_height = 6;
    constexpr int paddle_margins = 4;
    objManager = objectManager();

    objManager.create_paddle(paddle_margins, paddle_height);
    objManager.create_paddle(width - paddle_margins, paddle_height);
    objManager.create_ball();

    while (true) {
        auto start = std::chrono::system_clock::now();
        auto milliseconds_per_frame = std::chrono::milliseconds((int)((1 / fps) * 1000));
        objManager.doAction();
        memcpy(smfb, objManager.draw(), width * height * 3);
        std::this_thread::sleep_for(milliseconds_per_frame - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start));
    }
}