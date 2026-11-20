#include "protocol.h"

int main(void) {
    if (init_sockets() != 0) {
        fprintf(stderr, "Errore inizializzazione socket\n");
        return 1;
    }

    char port_str[32];
    printf("Inserisci la porta su cui avviare il server: ");
    fflush(stdout);
    if (!fgets(port_str, sizeof(port_str), stdin)) {
        cleanup_sockets();
        return 1;
    }
    port_str[strcspn(port_str, "\r\n")] = '\0';
    int port = atoi(port_str);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Porta non valida\n");
        cleanup_sockets();
        return 1;
    }

    sock_t listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd == SOCK_ERR) {
        perror("socket");
        cleanup_sockets();
        return 1;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)port);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        CLOSESOCKET(listen_fd);
        cleanup_sockets();
        return 1;
    }

    if (listen(listen_fd, 10) < 0) {
        perror("listen");
        CLOSESOCKET(listen_fd);
        cleanup_sockets();
        return 1;
    }

    printf("Server in ascolto sulla porta %d...\n", port);
    fflush(stdout);

    while (1) {
        struct sockaddr_in client_addr;
#ifdef _WIN32
        int addr_len = sizeof(client_addr);
#else
        socklen_t addr_len = sizeof(client_addr);
#endif
        sock_t client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd == SOCK_ERR) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN] = "sconosciuto";
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        printf("connesso con il client il cui nome IP: %s\n", client_ip);
        fflush(stdout);

        if (send_line(client_fd, "connessione avvenuta") < 0) {
            CLOSESOCKET(client_fd);
            continue;
        }

        while (1) {
            char line[BUF_SIZE];
            int r = recv_line(client_fd, line, sizeof(line));
            if (r <= 0) {
                break;
            }

            long long n1, n2, n3;
            if (sscanf(line, "%lld %lld %lld", &n1, &n2, &n3) != 3) {
                send_line(client_fd, "ERRORE_INPUT");
                continue;
            }

            long long res = n1 + n2 - n3;

            printf("la somma di %lld+%lld-%lld=%lld\n", n1, n2, n3, res);
            fflush(stdout);

            if (res == 100) {
                send_line(client_fd, "QUIT");
                break;
            } else {
                char out_res[64];
                snprintf(out_res, sizeof(out_res), "%lld", res);
                if (send_line(client_fd, out_res) < 0) {
                    break;
                }
            }
        }

        CLOSESOCKET(client_fd);
    }

    CLOSESOCKET(listen_fd);
    cleanup_sockets();
    return 0;
}
