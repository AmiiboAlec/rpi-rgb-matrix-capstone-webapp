#include <vector>
#include <functional>
#include <memory>
#include <stdio.h>
#include "ball.hpp"
#include "paddle.hpp"
#include "objectManager.hpp"
#include <cstring>

extern int height;
extern int width;

objectManager::objectManager() {
    leftscore = 0;
    rightscore = 0;
    buffer = new unsigned char[width * height * 3];
}

objectManager::objectManager(rgbpixel left, rgbpixel right) {
    leftColor = left;
    rightColor = right;
    leftscore = 0;
    rightscore = 0;
    buffer = new unsigned char[width * height * 3];
}

objectManager::~objectManager() {delete[] buffer;}

std::vector<std::unique_ptr<paddle>>& objectManager::get_paddles() {
    return _paddles;
}
std::vector<std::unique_ptr<ball>>& objectManager::get_balls() {
    return _balls;
}

std::vector<gameObject*> objectManager::get_objects() {
    std::vector<gameObject*> output;
    auto &temp1 = get_paddles();
    for (auto &a : temp1) {
        output.push_back(a.get());
    }
    auto &temp2 = get_balls();
    for (auto &a : temp2) {
        output.push_back(a.get());
    }
    return output;
}

void objectManager::create_paddle(int horizontal_location, int height, rgbpixel color) {
    _paddles.push_back(std::make_unique<paddle>(horizontal_location, height, color));
}

void objectManager::create_paddle(int horizontal_location, int height) {
    create_paddle(horizontal_location, height, {0xff, 0xff, 0xff});
}

void objectManager::create_ball(rgbpixel color) {
    _balls.push_back(std::make_unique<ball>(color));
}

void objectManager::create_ball() {
    create_ball({0xff, 0xff, 0xff});
}

void objectManager::doAction() {
    for (auto &a : _paddles) a->doAction();
    for (auto &a : _balls) a->doAction();
}

unsigned char* objectManager::draw() {
    memset(buffer, '\0', width * height * 3);
    for (int i = 0; i < height; i++) {
        int x = (width / 2) - 1;
        if ((i + 2) % 8 > 4) {
            buffer[(i * width + x)] = leftColor.r;
            buffer[(i * width + x) + (width * height)] = leftColor.g;
            buffer[(i * width + x) + (width * height * 2)] = leftColor.b;
            buffer[(i * width + x)] = rightColor.r;
            buffer[(i * width + x) + (width * height)] = rightColor.g;
            buffer[(i * width + x) + (width * height * 2)] = rightColor.b;
        }
    }
    if (scoreTimer) {
        //draw score
        scoreTimer--;
    }
    for (auto& b : get_balls()) {
        b->draw(buffer);
    }
    for (auto& p : get_paddles()) {
        p->draw(buffer);
    }
    return buffer;
}

void objectManager::incrementLeftScore() {
    leftscore++;
    scoreTimer = 100;
}

void objectManager::incrementRightScore() {
    rightscore++;
    scoreTimer = 100;
}

    
