#ifndef BALL_H
#define BALL_H

#include "gameObject.hpp"
#include "paddle.hpp"

struct ballInfo {
    float x;
    float y;
};

class ball : public gameObject{
    private:
    static constexpr float velocity = 5;
    float x;
    float y;
    float angle;
    rgbpixel color;
    public:
    ball();
    ball(rgbpixel color);
    void draw(unsigned char *buffer);
    void doAction();
    ballInfo get_ball_info() const;
    void reflect(paddle& p);
};

#endif