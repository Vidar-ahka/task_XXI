#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "udp_client.h"

#define PORT 1234
#define SERVER_IP "127.0.0.1"
#define BUF_SIZE 2048

// --- 1. Вспомогательная функция инициализации сокета ---
static int init_udp_socket(struct sockaddr_in *addr) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("UDP: Socket creation failed");
        return -1;
    }

    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(PORT);
    addr->sin_addr.s_addr = inet_addr(SERVER_IP);

    return sock;
}

// --- 2. Вспомогательная функция формирования сообщения ---
// Читает данные из shared и превращает в строку "count;val;val..."
static void format_cpu_message(SharedData *shared, char *buffer, size_t buf_len) {
    // Пишем количество элементов
    int offset = snprintf(buffer, buf_len, "%d", shared->count);

    // Добавляем значения
    for (int i = 0; i < shared->count; i++) {
        if (offset >= (int)buf_len - 16) break; // Предотвращаем переполнение
        offset += snprintf(buffer + offset, buf_len - offset, ";%.2f", shared->loads[i]);
    }
}

// --- 3. Основная функция потока (Обработчик событий) ---
void* udp_worker_thread(void* arg) {
    SharedData *shared = (SharedData*)arg;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    // Выполняем инициализацию
    int sock = init_udp_socket(&server_addr);
    if (sock < 0) return NULL;

    printf("UDP: Thread started. Target: %s:%d\n", SERVER_IP, PORT);

    while (1) {
        pthread_mutex_lock(&shared->mutex);

        // Ожидание события: "Данные готовы" или "Стоп"
        while (!shared->data_ready && !shared->stop_flag) {
            pthread_cond_wait(&shared->cond, &shared->mutex);
        }

        // Проверка выхода
        if (shared->stop_flag) {
            pthread_mutex_unlock(&shared->mutex);
            break;
        }

        // Вызываем функцию формирования строки
        format_cpu_message(shared, buffer, BUF_SIZE);

        // Сбрасываем флаг события
        shared->data_ready = 0;
        pthread_mutex_unlock(&shared->mutex);

        // Отправка данных (выполняется вне мьютекса)
        sendto(sock, buffer, strlen(buffer), 0, 
               (const struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    // --- 4. Очистка ---
    close(sock);
    printf("UDP: Thread finished.\n");
    return NULL;
}