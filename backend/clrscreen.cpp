#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstddef>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#define shared_mem_key 25375

int width = 64;
int height = 32;

unsigned char* smfb;

int main () {
    key_t key = shared_mem_key;
    int shmid = shmget(key, (width * height * 3) + 1, 0666 | IPC_CREAT);
    smfb = (unsigned char*) shmat(shmid, NULL, 0);
    struct sockaddr saddr = {AF_UNIX, '\0'};
    socklen_t saddrlen = sizeof(struct sockaddr);
    strncpy(saddr.sa_data, "child_sock", 13);
    saddr.sa_data[13] = '\0';
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    int conn = connect(sock, &saddr, saddrlen);

    for (int i = 0; i < width * height * 3; i++) smfb[i] = '\0';
    close(sock);
}