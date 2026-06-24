/*
 * Лабораторная работа №1 (февраль). Задание 5: Обработка завершения.
 * Модификация задания 4: перед завершением дочерний поток печатает
 * сообщение об этом (выполняет "очистку").
 *
 * Автор: Гадиян Сергей, группа ПИН-Б-З-22-1
 */

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <pthread.h>
#include <unistd.h>

volatile int stop = 0;

/* Функция "очистки": что поток делает перед самым завершением. */
void cleanup(void)
{
    printf("  [Дочерний поток] меня завершают - выполняю очистку и прощаюсь!\n");
    fflush(stdout);
}

void *child_routine(void *arg)
{
    int counter = 0;
    while (!stop) {
        printf("  [Дочерний поток] работаю... сообщение %d\n", ++counter);
        fflush(stdout);
        usleep(300000);
    }
    /* Перед выходом выполняем очистку. */
    cleanup();
    return NULL;
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    pthread_t child;

    if (pthread_create(&child, NULL, child_routine, NULL) != 0) {
        printf("Ошибка: не удалось создать поток\n");
        return 1;
    }

    printf("[Родитель] дочерний поток запущен, ждём 2 секунды...\n");
    fflush(stdout);
    sleep(2);

    stop = 1;
    printf("[Родитель] подан сигнал остановки\n");
    fflush(stdout);

    pthread_join(child, NULL);
    printf("[Родитель] дочерний поток завершён, выходим.\n");

    return 0;
}