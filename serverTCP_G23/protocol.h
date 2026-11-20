#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef _WIN32
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET sock_t;
    #define CLOSESOCKET(s) closesocket(s)
    #define SOCK_ERR INVALID_SOCKET
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    typedef int sock_t;
    #define CLOSESOCKET(s) close(s)
    #define SOCK_ERR (-1)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BUF_SIZE 512

static inline int init_sockets(void) {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    return 0;
#endif
}

static inline void cleanup_sockets(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

static inline int recv_all(sock_t s, char *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = recv(s, buf + total, len - total, 0);
        if (n <= 0) return n;
        total += n;
    }
    return total;
}

static inline int send_all(sock_t s, const char *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = send(s, buf + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return total;
}

static inline int send_line(sock_t s, const char *msg) {
    char tmp[BUF_SIZE];
    int n = snprintf(tmp, sizeof(tmp), "%s\n", msg);
    if (n < 0 || n >= (int)sizeof(tmp)) return -1;
    return send_all(s, tmp, n);
}

static inline int recv_line(sock_t s, char *out, int max_len) {
    int idx = 0;
    while (idx < max_len - 1) {
        char c;
        int n = recv(s, &c, 1, 0);
        if (n == 0) {
            if (idx == 0) return 0;
            break;
        }
        if (n < 0) return -1;
        if (c == '\n') break;
        if (c != '\r') out[idx++] = c;
    }
    out[idx] = '\0';
    return idx;
}

#endif
