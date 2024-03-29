#ifndef OBJMANAGER_H
#define OBJMANAGER_H
#include <vector>
#include <functional>
#include <memory>
#include "ball.hpp"
#include "paddle.hpp"

class objectManager {
    std::vector<std::unique_ptr<ball>> _balls;
    std::vector<std::unique_ptr<paddle>> _paddles;

    int leftscore;
    int rightscore;
    rgbpixel leftColor;
    rgbpixel rightColor;
    int scoreTimer;
    unsigned char *buffer;

    public:
    objectManager();
    objectManager(rgbpixel left, rgbpixel right);
    ~objectManager();
    std::vector<std::unique_ptr<paddle>>& get_paddles();
    std::vector<std::unique_ptr<ball>>& get_balls();
    std::vector<gameObject*> get_objects();
    
    void create_paddle(int horizontal_location, int height);
    void create_ball();
    void create_paddle(int horizontal_location, int height, rgbpixel color);
    void create_ball(rgbpixel color);
    void doAction();
    unsigned char* draw();
    void incrementLeftScore();
    void incrementRightScore();
};

#endif