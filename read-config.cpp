#pragma once

#include <fstream>
#include <string>

struct config_info {
    unsigned int width;
    unsigned int height;

    std::string driver_to_websocket_fifo;
    std::string websocket_to_driver_fifo;

    std::string websocket_to_main_fifo;
    std::string main_to_websocket_fifo;
};

config_info read_config_file() {
    config_info output;
    char filepath_buf[128];
    std::ifstream input("config");
    std::string line;
    std::getline(input, line);
    sscanf(line.c_str(), "WIDTH: %u", &output.width);
    std::getline(input, line);
    sscanf(line.c_str(), "HEIGHT: %u", &output.height);
    std::getline(input, line);
    sscanf(line.c_str(), "TOP-TO-MID-FIFO: %s", filepath_buf);
    output.driver_to_websocket_fifo = filepath_buf;
    std::getline(input, line);
    sscanf(line.c_str(), "MID-TO-TOP-FIFO: %s", filepath_buf);
    output.websocket_to_driver_fifo = filepath_buf;
    std::getline(input, line);
    sscanf(line.c_str(), "MID-TO-BOTTOM-FIFO: %s", filepath_buf);
    output.websocket_to_main_fifo = filepath_buf;
    std::getline(input, line);
    sscanf(line.c_str(), "BOTTOM-TO-MID-FIFO: %s", filepath_buf);
    output.main_to_websocket_fifo = filepath_buf;
    return output;
}

