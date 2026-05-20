#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "cpu_usage.h"
#include "udp_client.h"

SharedData shared;

void handle_sigint(int sig) {
    (void)sig;
    pthread_mutex_lock(&shared.mutex);
    shared.stop_flag = 1;
    pthread_cond_signal(&shared.cond);
    pthread_mutex_unlock(&shared.mutex);
}

int main() {
    int cpu_count = get_cpu_count();
    int total_elements = cpu_count + 1;

    // Инициализация синхронизации
    shared.count = total_elements;
    shared.loads = calloc(total_elements, sizeof(double));
    shared.data_ready = 0;
    shared.stop_flag = 0;
    pthread_mutex_init(&shared.mutex, NULL);
    pthread_cond_init(&shared.cond, NULL);

    CPUState *prev = malloc(sizeof(CPUState) * total_elements);
    CPUState *curr = malloc(sizeof(CPUState) * total_elements);

    pthread_t network_thread;
    pthread_create(&network_thread, NULL, udp_worker_thread, &shared);
    signal(SIGINT, handle_sigint);


    fetch_cpu_data(prev, cpu_count);

    printf("Starting Monitor: %d cores detected.\n", cpu_count);



    while (!shared.stop_flag) {
        sleep(5); // Частота 1 Гц
        fetch_cpu_data(curr, cpu_count);

        pthread_mutex_lock(&shared.mutex);
        for (int i = 0; i < total_elements; i++) {
            shared.loads[i] = calculate_load_pct(prev[i], curr[i]);
            prev[i] = curr[i];
        }
        shared.data_ready = 1;
        pthread_cond_signal(&shared.cond);

        printf("\033[H\033[J"); 
        printf("--- SYSTEM CPU MONITOR ---\n");
        printf("Total Cores: %d\n", cpu_count);
        printf("--------------------------\n");

     
        printf("OVERALL LOAD: %6.2f%%\n", shared.loads[0]);
        printf("--------------------------\n");

        // Печатаем каждое ядро по отдельности
        for (int i = 1; i < total_elements; i++) {
            printf("  Core %2d:     %6.2f%%\n", i - 1, shared.loads[i]);
        }

        printf("--------------------------\n");
        printf("Press Ctrl+C to exit\n");
     
        pthread_mutex_unlock(&shared.mutex);
        
    
    }

    pthread_join(network_thread, NULL);


    free(prev); free(curr); free(shared.loads);
    pthread_mutex_destroy(&shared.mutex);
    pthread_cond_destroy(&shared.cond);

    printf("\nMonitoring stopped safely.\n");
    return 0;
}