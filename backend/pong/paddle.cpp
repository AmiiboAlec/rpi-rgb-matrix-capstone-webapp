#include "objectManager.hpp"
#include "paddle.hpp"
#include <random>
#include <algorithm>
#include <memory>

extern int width;
extern int height;
extern objectManager objManager;
//extern unsigned char *double_buffer;
extern int fps;

paddle::paddle(int horizontal, int height) {
    x = horizontal;
    length = height;
    timescale = 1.0 / fps;
    color = {0xff, 0xff, 0xff};
}

paddle::paddle(int horizontal, int height, rgbpixel color) {
    x = horizontal;
    length = height;
    timescale = 1.0/fps;
    this->color = color;
}

void paddle::move(bool direction) { //down is false, up is true
    y += (max_velocity * timescale) * (direction * -1);
    if (y < 0) y = 0;
    if (y + length > height) y = height - length;
}

void paddle::draw(unsigned char *double_buffer) {
    int y = this->y;
    for (int i = y; i < y + length; i++) {
        double_buffer[(y * width) + x] = color.r;
        double_buffer[(y * width) + x + (width * height)] = color.g;
        double_buffer[(y * width) + x + (width * height * 2)] = color.b;
    }
}

void paddle::doAction() {
    if (!movement_timer) { //recalculate target direction
        bool reverse = false;
        if (width / 2 < x) {
            reverse = true;
        }
        auto &balls = objManager.get_balls();
        std::vector<ball*> local_balls(balls.size());
        for (int i = 0; i < balls.size(); i++) local_balls[i] = balls[i].get();
        std::sort(local_balls.begin(), local_balls.end(), [] (ball *a, ball *b) {
            return a->get_ball_info().x < b->get_ball_info().x;
        });
        ball *nearest;
        if (reverse) for (int i = local_balls.size() - 1; i >= 0; i--) {
            if (local_balls[i]->get_ball_info().x < x) {
                nearest = local_balls[i];
                break;
            }
        }
        else for (int i = 0; i < local_balls.size(); i++) {
            if (local_balls[i]->get_ball_info().x > x) {
                nearest = local_balls[i];
                break;
            }
        }
        float center = y + (length / 2);
        target_direction = center > nearest->get_ball_info().y;
        std::uniform_int_distribution<int> d(50, 300);
        movement_timer = d(gen);
    }
    move(target_direction);
    movement_timer--;
}

paddleInfo paddle::get_paddle_info() const {
    paddleInfo output;
    output.horizontal = x;
    output.top = y;
    output.bottom = y + length;
    return output;
}