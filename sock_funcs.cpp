#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int make_outgoing_socket (const char *filename) //FILENAME MUST BE UNDER 14 CHARACTERS!!
{
    struct sockaddr_un name;
    int sock;
    size_t size;

    /* Create the socket. */
    sock = socket (AF_LOCAL, SOCK_STREAM, 0);
    if (sock < 0)
        {
        perror ("socket");
        exit (EXIT_FAILURE);
        }

    /* Bind a name to the socket. */
    name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, filename, sizeof (name.sun_path));
    name.sun_path[sizeof (name.sun_path) - 1] = '\0';

    /* The size of the address is
        the offset of the start of the filename,
        plus its length (not including the terminating null byte).
        Alternatively you can just do:
        size = SUN_LEN (&name);
    */
    size = (offsetof (struct sockaddr_un, sun_path)
            + strlen (name.sun_path));

    if (bind (sock, (struct sockaddr *) &name, size) < 0)
        {
        perror ("bind");
        exit (EXIT_FAILURE);
        }

    listen(sock, 10);
    int connfd = accept(sock, NULL, NULL);
    return connfd;
}

int make_outgoing_socket_without_connection (const char *filename) {
    struct sockaddr_un name;
    int sock;
    size_t size;

    /* Create the socket. */
    sock = socket (AF_LOCAL, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror ("socket");
        exit (EXIT_FAILURE);
    }

    /* Bind a name to the socket. */
    name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, filename, sizeof (name.sun_path));
    name.sun_path[sizeof (name.sun_path) - 1] = '\0';

    /* The size of the address is
        the offset of the start of the filename,
        plus its length (not including the terminating null byte).
        Alternatively you can just do:
        size = SUN_LEN (&name);
    */
    size = (offsetof (struct sockaddr_un, sun_path)
            + strlen (name.sun_path));

    if (bind (sock, (struct sockaddr *) &name, size) < 0)
    {
        perror ("bind");
        exit (EXIT_FAILURE);
    }

    return sock;
}

int connect_incoming_socket(const char *filename) { //FILENAME MUST BE UNDER 14 CHARACTERS!!
    
    int sock, conn;
    struct sockaddr saddr = {AF_UNIX, '\0'};
    socklen_t saddrlen = sizeof(struct sockaddr);
    strncpy(saddr.sa_data, filename, 13);
    saddr.sa_data[13] = '\0';

    sock = socket(AF_UNIX, SOCK_STREAM, 0);

    conn = connect(sock, &saddr, saddrlen);

    return sock;
}