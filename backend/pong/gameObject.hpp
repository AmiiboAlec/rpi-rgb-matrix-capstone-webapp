#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <memory>

struct bbox {
    std::pair<int, int> top_left;
    std::pair<int, int> bottom_right;
};

class gameObject {
    protected:
    float timescale;
    public:
    virtual void draw(unsigned char *buffer) = 0;
    virtual void doAction() = 0;
    //virtual bbox get_bbox() = 0;
    //virtual bool detect_collision(const std::unique_ptr<gameObject> &b) = 0;
};

#endif