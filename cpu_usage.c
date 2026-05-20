#include <stdio.h>
#include <string.h>
#include "cpu_usage.h"

int get_cpu_count() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return 0;
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "cpu", 3) == 0 && line[3] != ' ') {
            count++;
        }
    }
    fclose(fp);
    return count;
}

void fetch_cpu_data(CPUState *states, int count) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return;
    char line[256];
    // Читаем count + 1 строк (первая - общая cpu, затем cpu0, cpu1...)
    for (int i = 0; i <= count; i++) {
        if (fgets(line, sizeof(line), fp)) {
            unsigned long long u, n, s, id, io, ir, si, st;
            char name[16];
            sscanf(line, "%s %llu %llu %llu %llu %llu %llu %llu %llu", 
                   name, &u, &n, &s, &id, &io, &ir, &si, &st);
            states[i].idle_all = id + io;
            states[i].total = u + n + s + id + io + ir + si + st;
        }
    }
    fclose(fp);
}

double calculate_load_pct(CPUState prev, CPUState curr) {
    unsigned long long d_total = curr.total - prev.total;
    unsigned long long d_idle = curr.idle_all - prev.idle_all;
    if (d_total == 0) return 0.0;
    return (double)(d_total - d_idle) / d_total * 100.0;
}