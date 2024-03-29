#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

unsigned char *smfb;

/*class my_1x3_vector {
    private: 
    constexpr int width = 1;
    constexpr int height = 3;
    double elements[width * height];
}
class my_3x3_matrix {
    private:
    constexpr int width = 3;
    constexpr int height = 3;
    double elements[width * height];

    public:
    my_3x3_matrix() {

    }

    double& operator[] (int idx) {
        return elements[idx];
    }


};*/

template <int N, int M>
class my_matrix {
    private:
    int width;
    int height;
    double elements[N * M];

    my_matrix() {
        width = N;
        height = M;

        for (int i = 0; i < width * height; i++) {
            elements[i] = 0;
        }
    }

    my_matrix(double rotation) {
        width = N;
        height = M;

        elements[0] = cos(rotation);
        elements[3] = elements[0];
        elements[2] = sin(rotation);
        elements[1] = -1 * elements[2];
        elements[4] = 1;
        elements[5] = 0;
        elements[6] = 0;
        elements[7] = 0;
        elements[8] = 1;
    }

    my_matrix(double x_translation, double y_translation) {
        width = N;
        height = M;

        elements[0] = 1;
        elements[1] = 0;
        elements[2] = -x_translation;
        elements[3] = 0;
        elements[4] = 1;
        elements[5] = -y_translation;
        elements[6] = 0;
        elements[7] = 0;
        elements[8] = 1;
    }

    double& operator[] (int idx) {
        return elements[idx];
    }

    double& at (int x, int y) {
        return elements[y * width + x];
    }

    
};

struct rgbpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

rgbpixel operator* (rgbpixel x, double scalar) {
    if (scalar < 0) scalar = 0;
    double temp[3];
    temp[0] = x.r * scalar;
    temp[1] = x.g * scalar;
    temp[2] = x.b * scalar;
    for (int i = 0; i < 3; i++) {
        if (temp[i] > 255) temp[i] = 255;
    }
    rgbpixel output = {(unsigned char) temp[0], (unsigned char) temp[1], (unsigned char) temp[2]};
    return output;
}

rgbpixel operator+ (rgbpixel a, rgbpixel b) {
    unsigned short temp[3];
    temp[0] = a.r + b.r;
    temp[1] = a.g + b.g;
    temp[2] = a.b + b.b;
    for (int i = 0; i < 3; i++) {
        if (temp[i] > 255) temp[i] = 255;
    }
    rgbpixel output = {(unsigned char) temp[0], (unsigned char) temp[1], (unsigned char) temp[2]};
    return output;
}


class image {
    private:
    int width;
    int height;
    std::vector<rgbpixel> colors;

    public:
    image (int width, int height) {
        this->width = width;
        this->height = height;
        colors = std::vector<rgbpixel>(width * height);
        for (auto &a : colors) {
            a = {0,0,0};
        }
    }

    rgbpixel& operator[] (int idx) {
        return colors[idx];
    }

    rgbpixel at (int x, int y) const {
        return colors[y * width + x];
    }

    rgbpixel get(double x, double y) const {
        double x_int, x_frac, y_int, y_frac;
        double weight_params[4];
        int x_idx, y_idx;
        rgbpixel topleft, topright, bottomleft, bottomright;
        double temp[3];

        x_frac = modf(x, &x_int);
        y_frac = modf(y, &y_int);
        x_idx = x_int;
        y_idx = y_int;
        
        topleft = at(x_idx, y_idx);
        topright = at(x_idx + 1, y_idx);
        bottomleft = at(x_idx, y_idx + 1);
        bottomright = at(x_idx + 1, y_idx + 1);

        weight_params[0] = (x_idx + 1 - x_frac) * (y_idx + 1 - y_frac);
        weight_params[1] = (x_idx + 1 - x_frac) * (y_frac - y_idx);
        weight_params[2] = (x_frac - x_idx) * (y_idx + 1 - y_frac);
        weight_params[3] = (x_frac - x_idx) * (y_frac - y_idx);

        temp[0] = topleft.r * weight_params[0];
        temp[1] = topleft.g * weight_params[0];
        temp[2] = topleft.b * weight_params[0];

        temp[0] += topright.r * weight_params[1];
        temp[1] += topright.g * weight_params[1];
        temp[2] += topright.b * weight_params[2];

        temp[0] += bottomleft.r * weight_params[2];
        temp[1] += bottomleft.g * weight_params[2];
        temp[2] += bottomleft.b * weight_params[2];

        temp[0] += bottomright.r * weight_params[3];
        temp[1] += bottomright.g * weight_params[3];
        temp[2] += bottomright.b * weight_params[3];

        return {(unsigned char) temp[0], (unsigned char) temp[1], (unsigned char) temp[2]};
    }
};

int main (int argc, char **argv) {

}