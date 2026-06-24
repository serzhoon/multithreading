/*
 * Лабораторная работа №1 (февраль). Задание 3: Параметры потока.
 * Создаётся ЧЕТЫРЕ потока, выполняющих ОДНУ И ТУ ЖЕ функцию.
 * Каждому через параметр передаётся своя последовательность строк,
 * поэтому потоки печатают разный текст.
 *
 * Автор: Гадиян Сергей, группа ПИН-Б-З-22-1
 * Компиляция: gcc task3_params.c -o task3 -pthread
 */

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>   /* для русских букв в консоли Windows */
#endif
#include <pthread.h>

#define NUM_THREADS 4

/* Структура-параметр: что именно поток должен печатать. */
typedef struct {
    int id;                 /* номер потока (для наглядности) */
    const char *lines[3];   /* последовательность из трёх строк */
} ThreadParam;

/* Одна общая функция для всех потоков. Получает указатель
   на ThreadParam и печатает переданные ей строки. */
void *print_routine(void *arg)
{
    ThreadParam *p = (ThreadParam *)arg;
    int i;
    for (i = 0; i < 3; i++) {
        printf("Поток %d: %s\n", p->id, p->lines[i]);
    }
    return NULL;
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);  /* выводить русский текст (UTF-8) */
#endif
    pthread_t threads[NUM_THREADS];

    /* У каждого потока - свой набор строк. */
    ThreadParam params[NUM_THREADS] = {
        {1, {"яблоко",  "груша",   "слива"}},
        {2, {"красный", "зелёный", "синий"}},
        {3, {"понедельник", "вторник", "среда"}},
        {4, {"один",    "два",     "три"}}
    };

    int i;

    /* Запускаем четыре потока, каждому передаём адрес его параметров. */
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, print_routine, &params[i]);
    }

    /* Ждём завершения всех четырёх. */
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}