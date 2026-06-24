
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <pthread.h>
#include <unistd.h>   
 
/* поток для детали A делается 1 секунду. */
void *make_A(void *arg) {
    printf("Начали делать деталь A...\n");
    fflush(stdout);
    sleep(1);
    printf("Деталь A готова (1 сек)\n");
    fflush(stdout);
    return NULL;
}
 
/* поток для детали B делается 2 секунды. */
void *make_B(void *arg) {
    printf("Начали делать деталь B...\n");
    fflush(stdout);
    sleep(2);
    printf("Деталь B готова (2 сек)\n");
    fflush(stdout);
    return NULL;
}
 
/* поток для детали C делается 3 секунды. */
void *make_C(void *arg) {
    printf("Начали делать деталь C...\n");
    fflush(stdout);
    sleep(3);
    printf("Деталь C готова (3 сек)\n");
    fflush(stdout);
    return NULL;
}
 
int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    pthread_t tA, tB, tC;
 
    printf("запуск производственной линии\n");
    fflush(stdout);
 
    /* Запускаем изготовление всех деталей одновременно. */
    pthread_create(&tA, NULL, make_A, NULL);
    pthread_create(&tB, NULL, make_B, NULL);
    pthread_create(&tC, NULL, make_C, NULL);
 
    /* Ждём, пока деталь A и B будут готовы - тогда соберём модуль. */
    pthread_join(tA, NULL);
    pthread_join(tB, NULL);
    printf(">>> модуль собран из деталей A и B\n");
    fflush(stdout);
 
    /* Ждём деталь C - тогда соберём готовый винтик. */
    pthread_join(tC, NULL);
    printf(">>> винтик собран из модуля и детали C\n");
    fflush(stdout);
 
    printf("=== Винтик готов! ===\n");
    return 0;
}