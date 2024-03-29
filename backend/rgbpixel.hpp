#pragma once
#include <functional>

struct rgbpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

rgbpixel operator+ (rgbpixel a, rgbpixel b) {
    unsigned short temp[3] = {a.r + b.r, a.g + b.g, a.b + b.b};
    for (int i = 0; i < 3; i++) {
        if (temp[i] > 255) temp[i] = 255;
    }
    return {(unsigned char) temp[0], (unsigned char) temp[1], (unsigned char) temp[2]};
}

void operator+= (rgbpixel &a, rgbpixel b) {
    a = a + b;
}

rgbpixel operator* (rgbpixel a, float b) {
    float temp[3] = {a.r * b, a.g * b, a.b * b};
    for (int i = 0; i < 3; i++) {
        if (temp[i] > 255) temp[i] = 255;
    }
    return {(unsigned char) temp[0], (unsigned char) temp[1], (unsigned char) temp[2]};
}

void operator*= (rgbpixel &a, float b) {
    a = a * b;
}

bool operator== (rgbpixel x, rgbpixel y) {
    return x.r == y.r && x.g == y.g && x.b == y.b;
}

template<>
struct std::hash<rgbpixel>
{
    std::size_t operator()(const rgbpixel& s) const {
        union {
            unsigned char temp[4];
            int a;
        };
        temp[0] = s.r;
        temp[1] = s.g;
        temp[2] = s.b;
        temp[3] = s.r ^ s.g ^ s.b;
        return std::hash<int>{}(a);
    }
};