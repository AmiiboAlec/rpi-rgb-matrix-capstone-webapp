#include <random>
#include <cmath>
#include "ball.hpp"
#include "paddle.hpp"
#include "objectManager.hpp"

extern int width;
extern int height;
//extern unsigned char *double_buffer;
extern objectManager objManager;
extern int fps;

ball::ball() {
    std::default_random_engine gen;
    std::uniform_real_distribution<float> d(0.3, 0.6);
    x = width / 2.0;
    y = height / 2.0;
    angle = d(gen) * 100;
    timescale = 1.0/fps;
    color = {0xff, 0xff, 0xff};
}

void ball::doAction() {
    float x_component = sin(angle);
    float y_component = cos(angle);
    float old_x = x;
    x += x_component * velocity * timescale;
    y += y_component * velocity * timescale;
    if (y < 0) {
        y *= -1;
    }
    if (y > height - 1) {
        y = ((y - height) * -1) + height;
    }
    auto &temp = objManager.get_paddles();
    for (auto &paddle : temp) {
        paddleInfo coords = paddle->get_paddle_info();
        if (((old_x < coords.horizontal) != (x < coords.horizontal)) && ((y < coords.top) && (y > coords.bottom))) {
            x = ((x - coords.horizontal) * -1) + coords.horizontal; //reflect ball if hit. TODO: Add angle alteration based on where on the paddle was hit
        }
    }
    if (x < 0) {
        objManager.incrementRightScore();
        x = width / 2.0;
        y = height / 2.0;
    }
    if (x > width) {
        objManager.incrementLeftScore();
        x = width / 2.0;
        y = height / 2.0;
    }
}

ballInfo ball::get_ball_info() const {
    return {x, y};
}

ball::ball(rgbpixel color) {
    std::default_random_engine gen;
    std::uniform_real_distribution<float> d(0.3, 0.6);
    x = width / 2.0;
    y = height / 2.0;
    angle = d(gen) * 100;
    timescale = 1.0/fps;
    this->color = color;
}

void ball::draw(unsigned char *double_buffer) {
    int x = this->x;
    int y = this->y;
    double_buffer[(y * width) + x] = color.r;
    double_buffer[(y * width) + x + (width * height)] = color.g;
    double_buffer[(y * width) + x + (width * height * 2)] = color.b;
}