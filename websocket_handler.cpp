#include <iostream>
#include "read-config.cpp"
#include "sock_funcs.cpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <sys/socket.h>
#include <spawn.h>
#include <unordered_map>
#include <fstream>
#include <mutex>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

unsigned int width, height;
int topsock;
int bottom_not_sock;
short port_num; //CURRENTLY UNDEFINED
pid_t child;
std::ifstream fifo_in;
std::ofstream fifo_out;
server js_sock;
websocketpp::connection_hdl handle;

void child_message(int signum) {
    static std::mutex d_lock;
    if (d_lock.try_lock()) {
        for (std::string line; std::getline(fifo_in, line);) {
            try{ 
                js_sock.send(handle, line);
            }
            catch (...) {
                std::cout << "WRITE TO CLOSED WS";
            }

        }
    }
    else {
        return;
    }
}

void on_message(server *s, websocketpp::connection_hdl hdl, message_ptr msg) {
    handle = hdl;
    auto message = msg->get_payload();
    char new_display[] = "NEW\n";
    if (*(int*) new_display == *(int*) message.c_str()) { //evil way of avoiding a goto
        kill(child, SIGTERM);
        std::stringstream reader(message);
        std::string line;
        std::getline(reader, line); //skip the NEW line
        std::getline(reader, line);
        posix_spawn(&child, line.c_str(), NULL, NULL, NULL, NULL); //THIS LINE IS NOT FINISHED
        //connect socket to child (Listen, connect)
    }
    else {
        fifo_out << message;
        fifo_out.flush();
        kill(child, SIGUSR1);
    }
}

int main (int argc, char **argv) {
    auto def_config = read_config_file();
    width = def_config.width;
    height = def_config.height;

    signal(SIGUSR1, child_message);

    topsock = connect_incoming_socket("toplevel");

    try {
        js_sock.init_asio();
        js_sock.set_message_handler(bind(&on_message, &js_sock, ::_1, ::_2));
        js_sock.listen(port_num);
        js_sock.start_accept();
        js_sock.run();
    }
    catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "Other exception" << std::endl;
    }
    close(topsock);
    remove("bottomlevel");
}