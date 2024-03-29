#define _USE_MATH_DEFINES

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <cmath>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/LU>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <utility>
#include "rgbpixel.hpp"

#define shared_mem_key 25375

#define real double

bool kill_flag = false;
int width = 64;
int height = 32;
unsigned char *smfb;

enum trig {
    sin_t = 1,
    cos_t = 2,
    neg_sin_t = 3,
    neg_cos_t = 4
};

class image {
    //using std::vector;
    int width;
    int height;
    std::vector<std::vector<unsigned char>> red_channel;
    std::vector<std::vector<unsigned char>> green_channel;
    std::vector<std::vector<unsigned char>> blue_channel;

    public:
    enum scaling_modes {
        NEAREST_NEIGHBOR,
        BILINEAR,
        BICUBIC
    };
    enum outside_modes {
        LOOP = false,
        EMPTY = true
    };

    image(int width, int height) {
        this->width = width;
        this->height = height;
        red_channel = std::vector<std::vector<unsigned char>>(height);
        green_channel = std::vector<std::vector<unsigned char>>(height);
        blue_channel = std::vector<std::vector<unsigned char>>(height);
        for (auto &a : red_channel) a = std::vector<unsigned char>(width);
        for (auto &a : green_channel) a = std::vector<unsigned char>(width);
        for (auto &a : blue_channel) a = std::vector<unsigned char>(width);
    }

    image () {}

    image(std::string filename) {
        //libpng stuff here
    }

    void set_pixel(int x_idx, int y_idx, rgbpixel color) {
        red_channel[y_idx][x_idx] = color.r;
        green_channel[y_idx][x_idx] = color.g;
        blue_channel[y_idx][x_idx] = color.b;
    }

    rgbpixel get_color(double x, double y, int scaling_mode = BILINEAR, int outside_image = LOOP) {
        if (outside_image == LOOP) {
            x = fmod(x, width);
            y = fmod(y, height);
            x = (x < 0) ? -x : x;
            y = (y < 0) ? -y : y;
        }
        double intx, fracx, inty, fracy;
        fracx = modf(x, &intx);
        fracy = modf(y, &inty);
        switch (scaling_mode) {
            case NEAREST_NEIGHBOR: {
            int x_idx, y_idx; rgbpixel output;
            x_idx = (fracx < 0.5) ? (int) intx : (int) intx + 1;
            y_idx = (fracy < 0.5) ? (int) inty : (int) inty + 1;
            output.r = red_channel[y_idx][x_idx];
            output.g = green_channel[y_idx][x_idx];
            output.b = blue_channel[y_idx][x_idx];
            return output;
            }
            case BICUBIC:
            std::cout << "Bicubic not yet implemented, falling back to bilinear\n";
            case BILINEAR:{
                struct intermediate_color {
                    double r;
                    double g;
                    double b;
                };
                int x_idx = (int) intx;
                int y_idx = (int) inty;
                rgbpixel top_right, bottom_right, top_left, bottom_left;
                intermediate_color top_middle, bottom_middle, final_output;
                top_right = {red_channel[y_idx][x_idx], green_channel[y_idx][x_idx], blue_channel[y_idx][x_idx]};
                bottom_right = {red_channel[y_idx + 1][x_idx], green_channel[y_idx + 1][x_idx], blue_channel[y_idx + 1][x_idx]};
                top_left = {red_channel[y_idx][x_idx + 1], green_channel[y_idx][x_idx + 1], blue_channel[y_idx][x_idx + 1]};
                bottom_left = {red_channel[y_idx + 1][x_idx + 1], green_channel[y_idx + 1][x_idx + 1], blue_channel[y_idx + 1][x_idx + 1]};
                top_middle.r = (top_right.r * (1 - fracx)) + (top_left.r * fracx);
                top_middle.g = (top_right.g * (1 - fracx)) + (top_left.g * fracx);
                top_middle.b = (top_right.b * (1 - fracx)) + (top_left.b * fracx);
                bottom_middle.r = (bottom_right.r * (1 - fracx)) + (bottom_left.r * fracx);
                bottom_middle.g = (bottom_right.g * (1 - fracx)) + (bottom_left.g * fracx);
                bottom_middle.b = (bottom_right.b * (1 - fracx)) + (bottom_left.b * fracx);
                final_output.r = (top_right.r * (1 - fracy)) + (bottom_middle.r * fracy);
                final_output.g = (top_right.g * (1 - fracy)) + (bottom_middle.g * fracy);
                final_output.b = (top_right.b * (1 - fracy)) + (bottom_middle.b * fracy);
                return {(unsigned char) final_output.r, (unsigned char) final_output.g, (unsigned char) final_output.b};
            }
        }
        throw std::runtime_error("Invalid image scaling option");
    }
    /*
    rgbpixel operator[] (double x, double y) {
        return get_color(x, y);
    }
    */
};

using namespace std::chrono_literals;
class my_3x3_matrix{
    std::chrono::system_clock::time_point creation;
    class element{
        double coefficient;
        double constant;
        std::chrono::system_clock::time_point creation;
        int trig_flag;
        public:
        element(double coefficient = 0, double constant = 0, int trig_flag = 0) {
            creation = std::chrono::system_clock::now();
            this->coefficient = coefficient;
            this->constant = constant;
            this->trig_flag = trig_flag;
        }
        
        double evaluate(const double seconds) {
            //std::cout << "I count " << seconds << " seconds." << std::endl;
            switch(trig_flag) {
                case trig::sin_t:
                return std::sin((seconds * coefficient + constant) * M_PI);
                case trig::cos_t:
                return std::cos((seconds * coefficient + constant) * M_PI);
                case trig::neg_sin_t:
                return -1 * std::sin((seconds * coefficient + constant) * M_PI);
                case trig::neg_cos_t:
                return -1 * std::cos((seconds * coefficient + constant) * M_PI);
                default: 
                return seconds * coefficient + constant;
            }
        }
    };
    element elements[9];
    public:

    struct double_3x3 {
        double elements[9];
        double& operator[] (int idx) {return elements[idx];}
    };

    my_3x3_matrix() {
        creation = std::chrono::system_clock::now();
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                elements[y * 3 + x] = (y == x) ? element(0, 1) : element();
            }
        }
    }

    my_3x3_matrix(const my_3x3_matrix &t) {
        creation = t.creation;
        for (int i = 0; i < 9; i++) {
            elements[i] = t.elements[i];
        }
    }
    
    element& operator[] (int idx) {
        return elements[idx];
    }

    void update_element(int idx, double constant, double coefficient = 0, int mode = 0) {
        elements[idx] = element(coefficient, constant, mode);
    }

    double get_element(int idx) {
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = now - creation;
        double seconds = diff.count();
        return elements[idx].evaluate(seconds);
    }

    Eigen::Matrix3d evaluate() {
        using Eigen::Matrix3d;

        Matrix3d output;
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - creation;
        double seconds = diff.count();
        
        for (int i = 0; i < 3; i++) {
            output.row(i) << elements[3 * i].evaluate(seconds), elements[3 * i + 1].evaluate(seconds), elements[3 * i + 2].evaluate(seconds);
        }
        return output;
    }

    std::string to_string() {
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - creation;
        double seconds = diff.count();
        char buff[512];
        sprintf(buff, "%9.2f   %9.2f   %9.2f\n%9.2f   %9.2f   %9.2f\n%9.2f   %9.2f   %9.2f", 
        elements[0].evaluate(seconds), elements[1].evaluate(seconds), elements[2].evaluate(seconds),
        elements[3].evaluate(seconds), elements[4].evaluate(seconds), elements[5].evaluate(seconds),
        elements[6].evaluate(seconds), elements[7].evaluate(seconds), elements[8].evaluate(seconds));
        return std::string(buff);
    }
};

std::vector<my_3x3_matrix> transformations;
image src;

my_3x3_matrix parse_matrix_string_alt(const std::string& input) {
    using std::stringstream;
    using std::vector;
    using std::string;
    my_3x3_matrix output;
    string cleaned_input = input;
    cleaned_input.erase(std::remove(cleaned_input.begin(), cleaned_input.end(), '['), cleaned_input.end());
    cleaned_input.erase(std::remove(cleaned_input.begin(), cleaned_input.end(), ']'), cleaned_input.end());
    stringstream reader(cleaned_input);
    vector<string> rows;
    vector<string> inputs;
    for (string line; std::getline(reader, line, ';');) {
        rows.push_back(line);
    }
    int counter = 0;
    for (const auto &a : rows) {
        vector<string> elements;
        stringstream reader(a);
        
        for (string line; std::getline(reader, line, ','); counter++) {
            int trig = 0;
            if (line.find("-sin") != string::npos) {
                trig = trig::neg_sin_t;
                int begin = line.find("(");
                int end = line.find(")");
                line = line.substr(begin + 1, (end - begin) - 1);
            }
            else if (line.find("-cos") != string::npos) {
                trig = trig::neg_cos_t;
                int begin = line.find("(");
                int end = line.find(")");
                line = line.substr(begin + 1, (end - begin) - 1);
            }
            else if (line.find("cos") != string::npos) {
                trig = trig::cos_t;
                int begin = line.find("(");
                int end = line.find(")");
                line = line.substr(begin + 1, (end - begin) - 1);
            }
            else if (line.find("sin") != string::npos) {
                trig = trig::sin_t;
                int begin = line.find("(");
                int end = line.find(")");
                line = line.substr(begin + 1, (end - begin) - 1);
            }

            if (counter >= 9) throw std::runtime_error(string("Too many elements"));
            int temp = line.find('t');
            
            if (temp == string::npos) {
                //std::cout << line << std::endl;
                output.update_element(counter, std::stod(line), 0, trig); // has no time component, only has 
            }
            else {
                int split_point = line.find("+");
                string str1 = (split_point != string::npos) ? line.substr(0, split_point) : line;
                string str2 = (split_point != string::npos) ? line.substr(split_point + 1) : "0";
                if (str2.find('t') != string::npos) std::swap(str1, str2);
                char* buff = (char*) calloc(50, 1);
                for (int i = 0; str1.c_str()[i] != 0; i++) {
                    if (str1[i] != 't') buff[i] = str1[i];
                }
                if (buff[0] == '\0') buff[0] = '1';
                double coefficient = std::stod(buff);
                if (str1.find("-") != string::npos) coefficient *= -1;
                double constant = std::stod(str2);
                if (str2.find("-") != string::npos) constant *= -1;
                output.update_element(counter, constant, coefficient, trig);
                free(buff);
            }
        }
    }
    return output;
}

void sigHandler(int signum) {
    switch (signum) {
        case SIGINT:
        case SIGTERM:
        kill_flag = true;
    }
    return;
}

std::pair<real, real> parse_linear_expression (const std::string &input) {
    std::pair<real, real> output;
    int nums_parsed;
    nums_parsed = (instruction.c_str(), "%lft+%lf", &output.first, &output.second);
    if (nums_parsed != 2) throw std::exception("Incorrect data format");
    return output;
}

class transformation {
    #define WIDTH 3
    #define HEIGHT 3

    protected: 
    real coefficient;
    real constant;

    public:
    transformation (const std::string &instruction) {
        auto nums = parse_linear_expression(instruction);
        coefficient = nums.first;
        constant = nums.second;
    }

    virtual Eigen::Matrix3d evaluate(real lifespan) = 0;

    Eigen::Matrix3d operator()(real lifespan) {
        return evaluate(lifespan);
    }

    real calc_matrix_factor(real lifespan) {
        return (lifespan * coefficient) + constant;
    }

};

class rotation : public transformation {
    public:
    rotation(const std::string &instruction) : transformation(instruction) {}

    Eigen::Matrix3d evaluate(real lifespan) { //currently rotates around 0,0
        Eigen::Matrix3d output;
        auto matrix_factor = calc_matrix_factor(lifespan);
        output(0,0) = cos(matrix_factor * (M_PI * 2));
        output(1,0) = sin(matrix_factor * (M_PI * 2));
        output(0,1) = -output(1,0);
        output(1,1) = output(0,0);
        output(0,2) = 0;
        output(1,2) = 0;
        output(2,0) = 0;
        output(2,1) = 0;
        output(2,2) = 1;
        return output;
    }
};

class x_translation : public transformation {
    public:
    x_translation(const std::string &instruction) : transformation(instruction) {}

    Eigen::Matrix3d evaluate(real lifespan) {
        Eigen::Matrix3d output;
        auto matrix_factor = calc_matrix_factor(lifespan);
        output(0,0) = 1;
        output(0,1) = 0;
        output(0,2) = matrix_factor;
        output(1,0) = 0;
        output(1,1) = 1;
        output(1,2) = 0;
        output(2,0) = 0;
        output(2,1) = 0;
        output(2,2) = 1;
        return output;
    }
};

class y_translation : public transformation {
    public:
    y_translation(const std::string &instruction) : transformation(instruction) {}

    Eigen::Matrix3d evaluate(real lifespan) {
        Eigen::Matrix3d output;
        auto matrix_factor = calc_matrix_factor(lifespan);
        output(0,0) = 1;
        output(0,1) = 0;
        output(0,2) = 0;
        output(1,0) = 0;
        output(1,1) = 1;
        output(1,2) = matrix_factor;
        output(2,0) = 0;
        output(2,1) = 0;
        output(2,2) = 1;
        return output;
    }
}

class x_stretch : public transformation {
    public:
    x_stretch(const std::string &instruction) : transformation(instruction) {}

    Eigen::Matrix3d evaluate(real lifespan) {
        Eigen::Matrix3d output;
        auto matrix_factor = calc_matrix_factor(lifespan);
        output(0,0) = matrix_factor;
        output(0,1) = 0;
        output(0,2) = 0;
        output(1,0) = 0;
        output(1,1) = 1;
        output(1,2) = 0;
        output(2,0) = 0;
        output(2,1) = 0;
        output(2,2) = 1;
        return output;
    }
}

class y_stretch : public transformation {
    public:
    y_stretch(const std::string &instruction) : transformation(instruction) {}

    Eigen::Matrix3d evaluate(real lifespan) {
        Eigen::Matrix3d output;
        auto matrix_factor = calc_matrix_factor(lifespan);
        output(0,0) = 1;
        output(0,1) = 0;
        output(0,2) = 0;
        output(1,0) = 0;
        output(1,1) = matrix_factor;
        output(1,2) = 0;
        output(2,0) = 0;
        output(2,1) = 0;
        output(2,2) = 1;
        return output;
    }
}

class x_shear : public transformation {
    public:
    x_shear (const std::string &instruction) : transformation(instruction) {}

    Eigen::Matrix3d evaluate(real lifespan) {
        Eigen::Matrix3d output;
        auto matrix_factor = calc_matrix_factor(lifespan);
        output(0,0) = 1;
        output(0,1) = matrix_factor;
        output(0,2) = 0;
        output(1,0) = 0;
        output(1,1) = 1;
        output(1,2) = 0;
        output(2,0) = 0;
        output(2,1) = 0;
        output(2,2) = 0;
        return output;
    }
}

class y_shear : public transformation {
    public:
    y_shear (const std::string &instruction) : transformation(instruction) {}

    Eigen::Matrix3d evaluate(real lifespan) {
        Eigen::Matrix3d output;
        auto matrix_factor = calc_matrix_factor(lifespan);
        output(0,0) = 1;
        output(0,1) = 0;
        output(0,2) = 0;
        output(1,0) = matrix_factor;
        output(1,1) = 1;
        output(1,2) = 0;
        output(2,0) = 0;
        output(2,1) = 0;
        output(2,2) = 0;
        return output;
    }
}

int main(int argc, char **argv) {
    using std::string;
    using std::vector;
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    int shmid = shmget(shared_mem_key, width * height * 3, 0666 | IPC_CREAT);
    smfb =  (unsigned char*) shmat(shmid, NULL, 0);
    string matrix_string = "";
    std::vector<std::unique_ptr<transformation>> n_transformations;
    for (int i = 1; i < argc; i++) {
        switch (argv[i][0]) {
            case 'r': //rotation
                n_transformations.push_back(std::make_unique<rotation>(&argv[i][1]));
                break;
            case 't': //translation
                switch (argv[i][1]) {
                    case 'x':
                        n_transformations.push_back(std::make_unique<x_translation>(&argv[i][2]));
                        break;
                    case 'y':
                        n_transformations.push_back(std::make_unique<y_translation>(&argv[i][2]));
                        break;
                    default:
                        throw std::exception("Invalid Translation formatting " + argv[i]);
                }
                break;
            case 's': //stretch
                switch (argv[i][1]) {
                    case 'x':
                        n_transformations.push_back(std::make_unique<x_stretch>(&argv[i][2]));
                        break;
                    case 'y':
                        n_transformations.push_back(std::make_unique<y_stretch>(&argv[i][2]));
                        break;
                    default:
                        throw std::exception("Invalid Translation formatting " + argv[i]);
                }
                break;
            case 'h': //shear
                switch (argv[i][1]) {
                    case 'x':
                        n_transformations.push_back(std::make_unique<x_shear>(&argv[i][2]));
                        break;
                    case 'y':
                        n_transformations.push_back(std::make_unique<y_shear>(&argv[i][2]));
                        break;
                    default:
                        throw std::exception("Invalid Translation formatting " + argv[i]);
                }
                break;
        }
    }

    vector<string> matrix_vector;
    for (int i = 1; i < argc; i++) {
        matrix_string += argv[i];
    }
    {
        int start = matrix_string.find("[");
        int end = matrix_string.find("]");
        while (true) {
            if (start == string::npos) break;
            matrix_vector.push_back(matrix_string.substr(start, (end - start) + 1));
            start = matrix_string.find("[", start + 1);
            end = matrix_string.find("]", end + 1);
        }
    }

    for (const auto &a : matrix_vector) {
        transformations.push_back(parse_matrix_string_alt(a));
    }

    //main thread 
    while (!kill_flag) {
        Eigen::Matrix3d transform;
        transform << 1, 0, 0, 0, 1, 0, 0, 0, 1;
        for (int i = transformations.size() - 1; i >= 0; i--) {
            transform = transform * transformations[i].evaluate();
        }
        transform = transform.inverse();
        for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) {
            Eigen::Vector3d vec = {(double) x, (double) y, 1};
            vec = transform * vec;
            auto color = src.get_color(vec[0], vec[1]);
            smfb[y * width + x] = color.r;
            smfb[y * width + x + (width * height)] = color.g;
            smfb[y * width + x + (width * height * 2)] = color.b;
        }
    }
}