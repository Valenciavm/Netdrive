#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

#include "car.h"  // Lógica del carro

// ====== Protocolo PTT ======
struct Message {
    char header[4];   // "PTT"
    char action[12];  // "UPLOAD", "DATA", etc.
    char data[150];   // Telemetría del carro
    char footer[4];   // "END"
};

// Serializa el struct Message en un buffer de bytes
void parseMessage(struct Message msg, char *buffer) {
    memcpy(buffer, msg.header, sizeof(msg.header));
    memcpy(buffer + sizeof(msg.header), msg.action, sizeof(msg.action));
    memcpy(buffer + sizeof(msg.header) + sizeof(msg.action), msg.data, sizeof(msg.data));
    memcpy(buffer + sizeof(msg.header) + sizeof(msg.action) + sizeof(msg.data), msg.footer, sizeof(msg.footer));
}

// ====== Variables globales ======
#define MAX_CLIENTS 10
int clients_fds[MAX_CLIENTS] = {0};
pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;
struct CarState car;                 // Estado global del carro
pthread_mutex_t car_lock = PTHREAD_MUTEX_INITIALIZER;

// ====== Función que maneja a cada cliente ======
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[1024];
    printf("[HILO] Cliente conectado (fd=%d)\n", client_fd);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = read(client_fd, buffer, sizeof(buffer));

        if (bytes <= 0) {
            printf("[HILO] Cliente (fd=%d) desconectado.\n", client_fd);
            break;
        }

        printf("[HILO] Mensaje recibido de fd=%d: %s\n", client_fd, buffer);

        if (strncmp(buffer, "EXIT", 4) == 0) {
            printf("[HILO] Cliente pidió salir.\n");
            break;
        }

        // Actualizar telemetría del carro con mutex
        pthread_mutex_lock(&car_lock);
        updateCarTelemetry(&car, buffer);
        pthread_mutex_unlock(&car_lock);

        // Preparar respuesta
        struct Message response;
        strcpy(response.header, "PTT");
        strcpy(response.action, "DATA");

        char telemetry[150];
        pthread_mutex_lock(&car_lock);
        generateCarTelemetry(car, telemetry, sizeof(telemetry));
        pthread_mutex_unlock(&car_lock);

        strcpy(response.data, telemetry);
        strcpy(response.footer, "END");

        char msg_buffer[sizeof(response)];
        memset(msg_buffer, 0, sizeof(msg_buffer));
        parseMessage(response, msg_buffer);

        send(client_fd, msg_buffer, sizeof(response), 0);
    }

    close(client_fd);
    pthread_exit(NULL);
}

// ====== Hilo que envía telemetría a todos los clientes cada X segundos ======
void *send_telemetry(void *arg) {
    (void)arg; // parámetro no usado

    while (1) {
        sleep(5); // cada 5 segundos

        pthread_mutex_lock(&clients_lock);

        char data[150];
        pthread_mutex_lock(&car_lock);
        generateCarTelemetry(car, data, sizeof(data));
        pthread_mutex_unlock(&car_lock);

        struct Message telemetry;
        strcpy(telemetry.header, "PTT");
        strcpy(telemetry.action, "DATA");
        strcpy(telemetry.data, data);
        strcpy(telemetry.footer, "END");

        char msg_buffer[sizeof(telemetry)];
        memset(msg_buffer, 0, sizeof(msg_buffer));
        parseMessage(telemetry, msg_buffer);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients_fds[i];
            if (fd != 0) { // fd válido
                if (send(fd, msg_buffer, sizeof(msg_buffer), 0) < 0) {
                    perror("[TELEMETRIA] Error enviando a cliente");
                } else {
                    printf("[TELEMETRIA] Enviada a fd=%d\n", fd);
                }
            }
        }

        pthread_mutex_unlock(&clients_lock);
    }

    pthread_exit(NULL);
}

// ====== MAIN ======
int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Inicializar carro
    initCar(&car);

    // Crear socket TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket failed"); exit(EXIT_FAILURE); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVIDOR] Escuchando en el puerto 2000...\n");

    // Hilo que envía telemetría a todos los clientes
    pthread_t broadcast_thread;
    pthread_create(&broadcast_thread, NULL, send_telemetry, NULL);
    pthread_detach(broadcast_thread);

    // Ciclo principal de aceptar clientes
    while (1) {
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (*client_fd < 0) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        // Guardar fd en array de clientes
        pthread_mutex_lock(&clients_lock);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients_fds[i] == 0) {
                clients_fds[i] = *client_fd;
                break;
            }
        }
        pthread_mutex_unlock(&clients_lock);

        // Crear hilo para cliente
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);

        printf("[SERVIDOR] Nuevo cliente conectado (fd=%d)\n", *client_fd);
    }

    close(server_fd);
    return 0;
}
