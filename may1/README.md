# Май (часть 1) — Производственная линия
Язык C  
[« Назад к списку месяцев](../README.md)

## Задание 24 
Разработайте имитатор производственной линии, изготавливающей винтики (widget).   
Винтик собирается из детали C и модуля, который, в свою очередь, состоит из деталей A и B.   
Для изготовления детали A требуется 1 секунда, В – две секунды, С – три секунды. Задержку изготовления деталей имитируйте при помощи sleep. Используйте семафоры-счетчики.

**Файл с кодом:** [production_line.c](production_line.c)

<details>
<summary>Показать код</summary>

```c
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <pthread.h>
#include <unistd.h>   /* sleep */

/* Поток для детали A - делается 1 секунду. */
void *make_A(void *arg) {
    printf("Начали делать деталь A...\n");
    fflush(stdout);
    sleep(1);
    printf("Деталь A готова (1 сек)\n");
    fflush(stdout);
    return NULL;
}

/* Поток для детали B - делается 2 секунды. */
void *make_B(void *arg) {
    printf("Начали делать деталь B...\n");
    fflush(stdout);
    sleep(2);
    printf("Деталь B готова (2 сек)\n");
    fflush(stdout);
    return NULL;
}

/* Поток для детали C - делается 3 секунды. */
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

    printf("=== Запуск производственной линии ===\n");
    fflush(stdout);

    /* Запускаем изготовление всех деталей одновременно. */
    pthread_create(&tA, NULL, make_A, NULL);
    pthread_create(&tB, NULL, make_B, NULL);
    pthread_create(&tC, NULL, make_C, NULL);

    /* Ждём, пока деталь A и B будут готовы - тогда соберём модуль. */
    pthread_join(tA, NULL);
    pthread_join(tB, NULL);
    printf(">>> Модуль собран из деталей A и B\n");
    fflush(stdout);

    /* Ждём деталь C - тогда соберём готовый винтик. */
    pthread_join(tC, NULL);
    printf(">>> Винтик собран из модуля и детали C\n");
    fflush(stdout);

    printf("=== Винтик готов! ===\n");
    return 0;
}
```

</details>

**Результат выполнения (консоль):**

```
запуск производственной линии
Начали делать деталь A...
Начали делать деталь B...
Начали делать деталь C...
Деталь A готова (1 сек)
Деталь B готова (2 сек)
>>> модуль собран из деталей A и B
Деталь C готова (3 сек)
>>> винтик собран из модуля и детали C
=== Винтик готов! ===

--------------------------------
Process exited after 3.023 seconds with return value 0
Press any key to continue . . .
```

---
