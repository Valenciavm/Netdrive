#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

#include "car.h"  // Importamos la lógica del carro

// ====== Protocolo PTT ======
struct Message {
    char header[4];   // "PTT"
    char action[12];  // "UPLOAD", "DATA", etc.
    char data[150];   // Telemetría del carro
    char footer[4];   // "END"
};

// Contador global de conexiones activas
int active_connections = 0;
pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;

// Serializa el struct Message en un buffer de bytes
void parseMessage(struct Message msg, char *buffer) {
    memcpy(buffer, msg.header, sizeof(msg.header));
    memcpy(buffer + sizeof(msg.header), msg.action, sizeof(msg.action));
    memcpy(buffer + sizeof(msg.header) + sizeof(msg.action), msg.data, sizeof(msg.data));
    memcpy(buffer + sizeof(msg.header) + sizeof(msg.action) + sizeof(msg.data), msg.footer, sizeof(msg.footer));
}

// ====== Manejo de clientes (cada uno en su hilo) ======
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    pthread_mutex_lock(&conn_mutex);
    active_connections++;
    pthread_mutex_unlock(&conn_mutex);

    char buffer[1024];
    bool connection_active = true;

    // Estado del carro individual por cliente
    struct CarState car;
    initCar(&car);

    printf("[HILO] ✅ Nueva conexión establecida (fd=%d)\n", client_fd);

    while (connection_active) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = read(client_fd, buffer, sizeof(buffer));

        if (bytes <= 0) {
            printf("[HILO] ❌ Cliente desconectado (fd=%d)\n", client_fd);
            break;
        }

        printf("[HILO] 📩 [%d bytes] Recibidos desde fd=%d: \"%s\"\n", bytes, client_fd, buffer);

        if (strncmp(buffer, "EXIT", 4) == 0) {
            printf("[HILO] 🛑 Cliente (fd=%d) pidió cerrar la conexión.\n", client_fd);
            break;
        }

        // ====== Actualizar el carro según comando ======
        updateCarTelemetry(&car, buffer);

        // ====== Preparar respuesta ======
        struct Message response;
        strcpy(response.header, "PTT");
        strcpy(response.action, "DATA");

        char telemetry[150];
        generateCarTelemetry(car, telemetry, sizeof(telemetry));
        strcpy(response.data, telemetry);

        strcpy(response.footer, "END");

        char msg_buffer[sizeof(response)];
        memset(msg_buffer, 0, sizeof(msg_buffer));
        parseMessage(response, msg_buffer);

        // Enviar mensaje de respuesta
        ssize_t sent = send(client_fd, msg_buffer, sizeof(response), 0);
        printf("[HILO] 📤 [%ld bytes] Enviados a fd=%d\n", sent, client_fd);
    }

    close(client_fd);
    printf("[HILO] 🔒 Conexión cerrada (fd=%d)\n", client_fd);

    pthread_mutex_lock(&conn_mutex);
    active_connections--;
    printf("[SERVIDOR] 📊 Conexiones activas: %d\n", active_connections);
    pthread_mutex_unlock(&conn_mutex);

    pthread_exit(NULL);
}

// ====== MAIN ======
int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket failed"); exit(EXIT_FAILURE); }

    printf("[SERVIDOR] ✅ Socket creado (fd=%d)\n", server_fd);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVIDOR] 📡 Bind exitoso. Escuchando en puerto 2000...\n");

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVIDOR] 🔥 Esperando conexiones...\n");

    while (1) {
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (*client_fd < 0) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        // ✅ Mostrar datos del cliente
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        printf("[SERVIDOR] 📶 Cliente conectado desde %s:%d (fd=%d)\n", client_ip, client_port, *client_fd);

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);

        printf("[SERVIDOR] ✅ Hilo creado para cliente (fd=%d)\n", *client_fd);
    }

    close(server_fd);
    return 0;
}
