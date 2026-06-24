/*
 * Лабораторная работа №1 (февраль). Задание 2: Ожидание потока.
 * Модификация задания 1: вывод родительского потока должен
 * производиться ПОСЛЕ завершения дочернего. Для этого pthread_join
 * вызывается ДО цикла печати родителя.
 *
 * Автор: Гадиян Сергей, группа ПИН-Б-З-22-1
 * Компиляция: gcc task2_join.c -o task2 -pthread
 */

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>   /* для русских букв в консоли Windows */
#endif
#include <pthread.h>

void *child_routine(void *arg)
{
    int i;
    for (i = 1; i <= 10; i++) {
        printf("  [Дочерний поток]  строка %d\n", i);
    }
    return NULL;
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);  /* выводить русский текст (UTF-8) */
#endif
    pthread_t child;
    int i;

    if (pthread_create(&child, NULL, child_routine, NULL) != 0) {
        printf("Ошибка: не удалось создать поток\n");
        return 1;
    }

    /* КЛЮЧЕВОЕ ОТЛИЧИЕ ОТ ЗАДАНИЯ 1:
       сначала полностью дожидаемся дочернего потока... */
    pthread_join(child, NULL);

    /* ...и только потом печатает родитель. Теперь его строки
       гарантированно идут ПОСЛЕ всех строк дочернего. */
    for (i = 1; i <= 10; i++) {
        printf("[Родительский поток] строка %d\n", i);
    }

    return 0;
}