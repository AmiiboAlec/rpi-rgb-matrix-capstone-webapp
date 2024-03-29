#ifndef PADDLE_H
#define PADDLE_H
//#include <memory>
#include <random>
#include "gameObject.hpp"

struct paddleInfo {
    int horizontal;
    float top;
    float bottom;
};

struct rgbpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class paddle : public gameObject {
    private:
    static constexpr float max_velocity = 5;
    int x;
    float y;
    int length;
    int movement_timer;
    bool target_direction;
    std::default_random_engine gen;
    rgbpixel color;

    void move(bool direction);

    public:

    paddle(int horizontal_location, int length);
    paddle(int horizontal_location, int length, rgbpixel color);
    void doAction();
    void draw(unsigned char *buffer);
    paddleInfo get_paddle_info() const;
    //bool detect_collision (const std::unique_ptr<gameObject> &b);

};

#endif