#include "car.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>

void initCar(struct CarState *car) {
    car->speed = 0;
    car->battery = 100;
    car->temp = 25.0;
    car->direction = 0; // 0 = norte
}

void updateCarTelemetry(struct CarState *car, const char *command) {
    // Cambiar velocidad
    if (strcmp(command, "SPEED UP") == 0 && car->speed < 120) {
        car->speed += 5;
    } else if (strcmp(command, "SLOW DOWN") == 0 && car->speed > 0) {
        car->speed -= 5;
    }

    // Cambiar dirección
    if (strcmp(command, "TURN LEFT") == 0) {
        car->direction = (car->direction - 15 + 360) % 360;
    } else if (strcmp(command, "TURN RIGHT") == 0) {
        car->direction = (car->direction + 15) % 360;
    }

    // Batería se reduce poco a poco
    if (car->battery > 0) {
        car->battery -= 1;
    }

    // Temperatura depende de la velocidad
    if (car->speed > 0 && car->temp < 45.0) {
        car->temp += 0.2;
    } else if (car->speed == 0 && car->temp > 20.0) {
        car->temp -= 0.1;
    }
}

void generateCarTelemetry(struct CarState car, char *buffer, size_t size) {
    snprintf(buffer, size, "speed=%d;dir=%d;battery=%d;temp=%.1f",
             car.speed, car.direction, car.battery, car.temp);
}