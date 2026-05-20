#ifndef CPU_USAGE_H
#define CPU_USAGE_H

typedef struct {
    unsigned long long idle_all;
    unsigned long long total;
} CPUState;

// Возвращает количество ядер процессора
int get_cpu_count();

// Заполняет массив состояний данными из /proc/stat
// count - количество ядер (без учета общего)
void fetch_cpu_data(CPUState *states, int count);

// Считает процент загрузки на основе разницы состояний
double calculate_load_pct(CPUState prev, CPUState curr);

#endif