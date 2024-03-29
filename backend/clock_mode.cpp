#include <iostream>
#include <string>
#include <chrono>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <unordered_map>
#include <thread>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <ctime>
#include "rgbpixel.hpp"

#define shared_memory_key 25375

int width = 64;
int height = 32;
bool kill_flag = false;
float decay = 0.25;
float time_scale = 4;
int number_of_outside_dots = 4;
rgbpixel hands_base_color = {0x39, 0xff, 0x14};
rgbpixel outside_base_color = {0xA3, 0xFF, 0xF9};
bool continue_flag = false;
unsigned char *smfb;

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

rgbpixel parse_hex(const char *str) {
    rgbpixel output;
    sscanf(str, "%2hhX%2hhX%2hhX", &output.r, &output.g, &output.b);
    return output;
}

rgbpixel hsv_to_rgb(double h, double s, double v) { //h ranges from 0 to 360, S and V range from 0 to 1
    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60, 2) - 1));
    double m = v - c;
    struct {
        double r;
        double g;
        double b;
    } c_prime;
    switch ((int) floor(h / 60)) {
        case 0: c_prime = {c, x, 0}; break;
        case 1: c_prime = {x, c, 0}; break;
        case 2: c_prime = {0, c, x}; break;
        case 3: c_prime = {0, x, c}; break;
        case 4: c_prime = {x, 0, c}; break;
        case 5: c_prime = {c, 0, x}; break;
    }
    return {(c_prime.r + m) * 255, (c_prime.g + m) * 255, (c_prime.b + m) * 255};
}

class color {
    rgbpixel base_color;
    std::chrono::time_point<std::chrono::system_clock> time_created;
    double lifespan;

    public:
    color() {}

    color(rgbpixel base, double lifespan) {
        base_color = base;
        this->lifespan = lifespan;
        time_created = std::chrono::system_clock::now();
    }

    rgbpixel get_current_color(const std::chrono::time_point<std::chrono::system_clock> &current_time) {
        double life_remaining; //as a ratio from 0 to 1
        life_remaining = (lifespan - std::chrono::duration<double>(current_time - time_created).count()) / lifespan;
        return base_color * life_remaining;
    }
};

void draw_pixel(int x, int y, rgbpixel color) {
    int offset_const = width * height;
    smfb[y * width + x] = color.r;
    smfb[y * width + x + offset_const] = color.g;
    smfb[y * width + x + offset_const + offset_const] = color.b;
}

void exit_gracefully(int signum) {
    kill_flag = true;
}

std::vector<std::pair<int, int>> build_circle() {
    std::pair<double, double> t_center = {((width * 1.0) - 1) / 2, ((height * 1.0) - 1) / 2};
    std::pair<double, double> top_point = {(width / 2.0), 0};
    double radius = sqrt(((t_center.first - top_point.first) * (t_center.first - top_point.first)) + ((t_center.second - top_point.second) * (t_center.second - top_point.second)));

    std::vector<std::pair<double, double>> output;

    double x = 0.5, y = 0;
    while (true) {
        std::pair<double, double> next_point;
        next_point.first = x;
        next_point.second = sqrt((radius * radius) - (x * x));
        y = next_point.second;
        if (y < x) break;
        output.push_back(next_point);
        x++;
    }

    int starting_length = output.size();
    for (int i = 0; i < starting_length; i++) output.push_back({output[i].second, output[i].first});
    starting_length = output.size();
    for (int i = 0; i < starting_length; i++) output.push_back({output[i].first * -1, output[i].second});
    starting_length = output.size();
    for (int i = 0; i < starting_length; i++) output.push_back({output[i].first, output[i].second * -1});

    std::sort(output.begin(), output.end(), [] (std::pair<double, double> &a, std::pair<double, double> &b) {
        return atan2(a.second, a.first) < atan2(b.second, b.first);
    });
    std::vector<std::pair<int, int>> true_output;
    for (auto &a : output) {
        a.first += t_center.first;
        a.second += t_center.second;
        true_output.push_back({round(a.first), round(a.second)});
    }
    return true_output;
}

void draw_outside(const std::vector<std::pair<int, int>> &circle_points) {
    std::function<double(double)> circle_brightness_func = [] (double input) {
        const double scaling_factor = 1; //don't remember what this is for
        std::function <double(double, double)> max = [] (double x, double y) {return (x > y) ? x : y;};
        return max(std::fmod((number_of_outside_dots * scaling_factor * input),scaling_factor) - scaling_factor * decay, 0) * (1/(1-decay));
    };
    double current_time = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    current_time /= time_scale;
    current_time = fmod(current_time, 1);
    current_time *= -1;
    current_time += 1;
    double i = 0;
    bool zero_check = true;
    for (auto &a : circle_points) {
        double brightness = circle_brightness_func(1 - (i / circle_points.size()) + current_time);
        if (brightness != 0) zero_check = false;
        draw_pixel(a.first, a.second, outside_base_color * brightness);
        i++;
    }   
    if (zero_check) {
        printf("Error!\n");
    }
}

void draw_hands() {
    static std::unordered_map<std::pair<int, int>, color, pair_hash> hands_dict;
    constexpr float DEGREES_TO_RADIANS_FACTOR = (2 * M_PI) / 360;
    constexpr int MINUTE_HAND_LEN = 12;
    constexpr int HOUR_HAND_LEN = 8;
    constexpr double MINUTE_HAND_REDRAW_RATE = 0.65;
    constexpr double HOUR_HAND_REDRAW_RATE = 0.8;
    constexpr double MINUTE_HAND_DECAY_RATE = 0.8;
    constexpr double HOUR_HAND_DECAY_RATE = 0.8;
    std::function<time_t(std::chrono::time_point<std::chrono::system_clock>)> time_convert = [] (const std::chrono::time_point<std::chrono::system_clock> &time) {
        return std::chrono::system_clock::to_time_t(time);
    };
    auto current_time = std::chrono::system_clock::now();
    auto now = time_convert(current_time);
    auto lt = localtime(&now);
    double computer_time = std::chrono::duration<double>(current_time.time_since_epoch()).count();
    std::pair<double, double> center = {((width * 1.0) - 1)/ 2, ((height * 1.0) - 1) / 2};
    int current_mins = lt->tm_min;
    int current_hours = lt->tm_hour;

    double min_degrees = 360 - (current_mins * 360.0 / 60.0) + 180;
    double hr_degrees = 360 - (current_hours * 360.0 / 12.0) + 180;
    double min_len = fmod(computer_time, MINUTE_HAND_REDRAW_RATE) * (MINUTE_HAND_LEN / MINUTE_HAND_REDRAW_RATE);
    double hr_len = fmod(computer_time, HOUR_HAND_REDRAW_RATE) * (HOUR_HAND_LEN / HOUR_HAND_REDRAW_RATE);

    std::pair<double, double> new_min_point = {min_len * sin(min_degrees * DEGREES_TO_RADIANS_FACTOR), min_len * cos(min_degrees * DEGREES_TO_RADIANS_FACTOR)};
    std::pair<double, double> new_hr_point = {hr_len * sin(hr_degrees * DEGREES_TO_RADIANS_FACTOR), hr_len * cos(hr_degrees * DEGREES_TO_RADIANS_FACTOR)};
    
    std::pair<int, int> output[2] = {{round(new_min_point.first + center.first), round(new_min_point.second + center.second)}, {round(new_hr_point.first + center.first), round(new_hr_point.second + center.second)}};

    hands_dict[output[0]] = color(hands_base_color, MINUTE_HAND_DECAY_RATE);
    hands_dict[output[1]] = color(hands_base_color, HOUR_HAND_DECAY_RATE);

    std::vector<std::pair<int, int>> erase_list;
    for (auto a : hands_dict) {
        auto b = a.first;
        auto c = a.second;
        const rgbpixel empty = {0,0,0};
        auto new_color = c.get_current_color(current_time);
        draw_pixel(b.first, b.second, new_color);
        if (new_color == empty) {
            erase_list.push_back(b);
        }
    }
    for (auto a : erase_list) hands_dict.erase(a);
}

int main (int argc, char **argv) {
    key_t key = shared_memory_key;
    int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT);
    smfb = (unsigned char*) shmat(shmid, NULL, 0);
    int c;
    while ((c = getopt (argc, argv, ":d:n:t:i:o:h")) != -1) switch (c) {
        case 'd':
            //printf("Decay: %s", optarg);
            decay = atof(optarg);
            break;
        case 'n':
            //printf("Outside: %s", optarg);
            number_of_outside_dots = atoi(optarg);
            break;
        case 't':
            //printf("Time Scale: %s", optarg);
            time_scale = atof(optarg);
            break;
        case 'i':
            //printf("hands color: %s", optarg);
            hands_base_color = parse_hex(optarg);
            break;
        case 'o':
            //printf("outside color: %s", optarg);
            outside_base_color = parse_hex(optarg);
            break;
        case 'h': {
            std::string help[] = {
                "-d: Rate at which the outside phosphors decay. Ranges from (-infinity, 1). Default is 0.25",
                "-n: Number of dots tracing around the outside of the clock. Default is 4.",
                "-t: Time it takes for a dot to go around the circle. Default is 4 seconds",
                "-i: RGB Color of the clockhands. Format is RRGGBB, or standard hexadecimal color notation",
                "-o: RGB Color of the outside of the clock. Format is RRGGBB, or standard hexadecimal color notation.\nNote: Only makes use of the hue and saturation components, does not use value"
            };
            for (int i = 0; i < 5; i++) {
                std::cout << help[i] << std::endl;
            }
            exit(0);
        }
        default:
            printf("Invalid arg: %c\n", (char) c);
            break;
    }

    signal(SIGINT, exit_gracefully);
    signal(SIGTERM, exit_gracefully);

    std::thread jobs[2];

    jobs[0] = std::thread([] () {
        auto circle_points = build_circle();
        auto start_of_loop = std::chrono::system_clock::now();
        auto milliseconds_per_frame = std::chrono::milliseconds(10);
        while (!kill_flag) {
            draw_outside(circle_points);
            std::this_thread::sleep_for(milliseconds_per_frame - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_of_loop));
        }
    });
    jobs[1] = std::thread([] () {
        auto start_of_loop = std::chrono::system_clock::now();
        auto milliseconds_per_frame = std::chrono::milliseconds(10);
        while (!kill_flag) {
            draw_hands();
            std::this_thread::sleep_for(milliseconds_per_frame - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_of_loop));
        }
    });
    jobs[0].join();
    jobs[1].join();
    for (int i = 0; i < width * height * 3; i++) smfb[i] = '\0';
}