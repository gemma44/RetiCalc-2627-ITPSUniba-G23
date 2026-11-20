#include "protocol.h"

int main(void) {
    if (init_sockets() != 0) {
        fprintf(stderr, "Errore inizializzazione socket\n");
        return 1;
    }

    char ip_str[128];
    char port_str[32];

    printf("Inserisci l'indirizzo IP del server: ");
    fflush(stdout);
    if (!fgets(ip_str, sizeof(ip_str), stdin)) {
        cleanup_sockets();
        return 1;
    }
    ip_str[strcspn(ip_str, "\r\n")] = '\0';

    printf("Inserisci la porta del server: ");
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

    sock_t sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == SOCK_ERR) {
        perror("socket");
        cleanup_sockets();
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, ip_str, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Indirizzo IP non valido\n");
        CLOSESOCKET(sock_fd);
        cleanup_sockets();
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        CLOSESOCKET(sock_fd);
        cleanup_sockets();
        return 1;
    }

    char welcome[BUF_SIZE];
    if (recv_line(sock_fd, welcome, sizeof(welcome)) <= 0) {
        fprintf(stderr, "Connessione chiusa dal server prima del messaggio iniziale\n");
        CLOSESOCKET(sock_fd);
        cleanup_sockets();
        return 1;
    }
    printf("%s\n", welcome);
    fflush(stdout);

    while (1) {
        char input_line[BUF_SIZE];
        long long n1, n2, n3;

        printf("Inserisci tre numeri interi separati da spazio (n1 n2 n3): ");
        fflush(stdout);
        if (!fgets(input_line, sizeof(input_line), stdin)) {
            break;
        }

        if (sscanf(input_line, "%lld %lld %lld", &n1, &n2, &n3) != 3) {
            printf("Input non valido. Inserisci esattamente tre numeri interi.\n");
            fflush(stdout);
            continue;
        }

        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg), "%lld %lld %lld", n1, n2, n3);
        if (send_line(sock_fd, msg) < 0) {
            perror("send");
            break;
        }

        char reply[BUF_SIZE];
        int r = recv_line(sock_fd, reply, sizeof(reply));
        if (r <= 0) {
            printf("Connessione chiusa dal server.\n");
            break;
        }

        if (strcmp(reply, "QUIT") == 0) {
            printf("Risultato uguale a 100: chiusura della connessione.\n");
            fflush(stdout);
            break;
        } else if (strcmp(reply, "ERRORE_INPUT") == 0) {
            printf("Errore: il server non ha riconosciuto l'input.\n");
            fflush(stdout);
        } else {
            printf("Risultato ricevuto dal server: %s\n", reply);
            fflush(stdout);
        }
    }

    CLOSESOCKET(sock_fd);
    cleanup_sockets();
    return 0;
}
