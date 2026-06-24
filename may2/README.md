# Май (часть 2)

[« Назад к списку месяцев](../README.md)

## Многопоточный сервер
Реализуйте сервер, который принимает TCP соединения и транслирует их. Сервер должен получать из командной строки следующие параметры:  
·	Номер порта P, на котором следует слушать.  
·	Имя или IP-адрес узла N, на который следует транслировать соединения.  
·	Номер порта P', на который следует транслировать соединения.  
Сервер принимает все входящие запросы на установление соединения на порт P. Для каждого такого соединения он открывает соединение с портом P' на сервере N.   
Затем он транслирует все данные, получаемые от клиента, серверу N, а все данные, получаемые от сервера N – клиенту. Если сервер N или клиент разрывают соединение, наш сервер также должен разорвать соединение. Если сервер N отказывает в установлении соединения, следует разорвать клиентское соединение.
Сервер должен обеспечивать трансляцию 510 соединений при лимите количества открытых файлов на процесс 1024. Сервер не должен быть многопоточным и никогда не должен блокироваться при операциях чтения и записи. Не следует использовать неблокирующиеся сокеты. Следует использовать select или poll.



**Файл с кодом:** [tcp.c](tcp.c)

<details>
<summary>Показать код</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

#define MAX_CONN 510          /* сколько пар соединений поддерживаем */
#define BUF_SIZE 4096         /* размер буфера для передачи данных */

/*
 * Одно "соединение" клиента состоит из ДВУХ сокетов:
 *  - client_fd: сокет к клиенту, который к нам подключился
 *  - remote_fd: сокет к удалённому серверу N, куда мы транслируем
 * Данные от клиента пишем в remote, от remote - клиенту.
 */
typedef struct {
    int client_fd;     /* сокет клиента (-1 = свободно) */
    int remote_fd;     /* сокет удалённого сервера */
    int used;          /* занята ли эта запись */
} pair_t;

pair_t pairs[MAX_CONN];        /* все пары соединений */

char *remote_host;             /* узел N (куда транслируем) */
int   remote_port;             /* порт P' */

/* Найти свободную ячейку для новой пары соединений. */
int find_free_slot() {
    for (int i = 0; i < MAX_CONN; i++)
        if (!pairs[i].used)
            return i;
    return -1;
}

/* Закрыть пару соединений (и клиента, и удалённый сервер). */
void close_pair(int i) {
    if (pairs[i].client_fd != -1) close(pairs[i].client_fd);
    if (pairs[i].remote_fd != -1) close(pairs[i].remote_fd);
    pairs[i].client_fd = -1;
    pairs[i].remote_fd = -1;
    pairs[i].used = 0;
}

/* Установить соединение с удалённым сервером N:P'. Вернуть сокет или -1. */
int connect_to_remote() {
    struct sockaddr_in addr;
    struct hostent *he;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    /* превратить имя узла (например "example.com") в IP-адрес */
    he = gethostbyname(remote_host);
    if (he == NULL) { close(fd); return -1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote_port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;     /* сервер N отказал в соединении */
    }
    return fd;
}

int main(int argc, char **argv) {
    /* Проверяем аргументы командной строки. */
    if (argc != 4) {
        printf("Использование: %s <порт P> <узел N> <порт P'>\n", argv[0]);
        return 1;
    }

    int listen_port = atoi(argv[1]);   /* порт P, на котором слушаем */
    remote_host = argv[2];             /* узел N */
    remote_port = atoi(argv[3]);       /* порт P' */

    /* Изначально все пары - свободны. */
    for (int i = 0; i < MAX_CONN; i++) {
        pairs[i].client_fd = -1;
        pairs[i].remote_fd = -1;
        pairs[i].used = 0;
    }

    /* --- Создаём слушающий сокет на порту P --- */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;       /* слушаем на всех адресах */
    serv.sin_port = htons(listen_port);

    if (bind(listen_fd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(listen_fd, 16) < 0) {
        perror("listen"); return 1;
    }

    printf("Сервер слушает порт %d, транслирует на %s:%d\n",
           listen_port, remote_host, remote_port);

    /* --- Главный цикл: ждём события через select --- */
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        int maxfd = listen_fd;

        /* добавляем в набор все активные сокеты */
        for (int i = 0; i < MAX_CONN; i++) {
            if (pairs[i].used) {
                FD_SET(pairs[i].client_fd, &readfds);
                FD_SET(pairs[i].remote_fd, &readfds);
                if (pairs[i].client_fd > maxfd) maxfd = pairs[i].client_fd;
                if (pairs[i].remote_fd > maxfd) maxfd = pairs[i].remote_fd;
            }
        }

        /* select ждёт, пока хоть на одном сокете не появятся данные */
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        /* --- Новое входящее соединение на порту P? --- */
        if (FD_ISSET(listen_fd, &readfds)) {
            int client = accept(listen_fd, NULL, NULL);
            if (client >= 0) {
                int slot = find_free_slot();
                if (slot == -1) {
                    /* мест нет - закрываем клиента */
                    close(client);
                } else {
                    /* открываем соединение к серверу N */
                    int remote = connect_to_remote();
                    if (remote < 0) {
                        /* сервер N отказал - рвём клиента */
                        close(client);
                    } else {
                        pairs[slot].client_fd = client;
                        pairs[slot].remote_fd = remote;
                        pairs[slot].used = 1;
                        printf("Новое соединение (слот %d)\n", slot);
                    }
                }
            }
        }

        /* --- Проверяем данные на всех активных парах --- */
        for (int i = 0; i < MAX_CONN; i++) {
            if (!pairs[i].used) continue;

            char buf[BUF_SIZE];

            /* данные от КЛИЕНТА -> пересылаем удалённому серверу */
            if (FD_ISSET(pairs[i].client_fd, &readfds)) {
                int n = read(pairs[i].client_fd, buf, BUF_SIZE);
                if (n <= 0) {            /* клиент закрыл соединение */
                    printf("Клиент отключился (слот %d)\n", i);
                    close_pair(i);
                    continue;
                }
                write(pairs[i].remote_fd, buf, n);
            }

            /* данные от СЕРВЕРА N -> пересылаем клиенту */
            if (FD_ISSET(pairs[i].remote_fd, &readfds)) {
                int n = read(pairs[i].remote_fd, buf, BUF_SIZE);
                if (n <= 0) {            /* сервер закрыл соединение */
                    printf("Сервер N отключился (слот %d)\n", i);
                    close_pair(i);
                    continue;
                }
                write(pairs[i].client_fd, buf, n);
            }
        }
    }

    close(listen_fd);
    return 0;
}
```

</details>

**Результат выполнения (консоль):**

```
$ ./tcp 9000 127.0.0.1 9001
Сервер слушает порт 9000, транслирует на 127.0.0.1:9001
Новое соединение (слот 0)
Клиент отключился (слот 0)
```

---