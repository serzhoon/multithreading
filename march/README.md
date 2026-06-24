# Март — Потоки: алгоритмы и синхронизация

Лабораторная работа: создание потоков, передача параметров, сбор результатов, устранение взаимоблокировки.

**Студент:** Гадиян Сергей, группа ПИН-Б-З-22-1
**Язык:** Java
**Среда:** IntelliJ IDEA (OpenJDK 26)

[« Назад к списку месяцев](../README.md)

---

## Задание 6. Sleepsort

Реализуйте уникальный алгоритм сортировки sleepsort с асимптотикой O(N) (по процессорному времени).
На стандартный вход программы подается не более 100 строк различной длины. Вам необходимо вывести эти строки, отсортированные по длине. Строки одинаковой длины могут выводиться в произвольном порядке.
Для каждой входной строки, создайте поток и передайте ей эту строку в качестве параметра. Поток должна вызвать sleep(2) или usleep(2) с параметром, пропорциональным длине этой строки. Затем поток выводит строку в стандартный поток вывода и завершается. Не следует выбирать коэффициент пропорциональности слишком маленьким, вы рискуете получить некорректную сортировку.



**Файл с кодом:** [SleepSort.java](SleepSort.java)

<details>
<summary>Показать код</summary>

```java
public class SleepSort {

    public static void main(String[] args) throws InterruptedException {
        // числа которые хотим отсортировать
        int[] numbers = {5, 2, 8, 1, 9, 3, 7};

        System.out.println("Исходный массив:");
        for (int n : numbers) {
            System.out.print(n + " ");
        }
        System.out.println();
        System.out.println("Результат sleepsort (числа печатают сами потоки):");

        // для каждого числа создаём и запускаем свой поток
        for (int n : numbers) {
            final int value = n;   // значение для этого потока

            Thread t = new Thread(() -> {
                try {
                    // спим время, пропорциональное числу
                    // множитель 100 мс чтобы разница была заметна
                    Thread.sleep(value * 100L);
                } catch (InterruptedException e) {
                    // если поток прервут во время сна - ничего не делаем
                }
                // проснулись  печатаем своё число.
                System.out.println(value);
            });

            t.start();   // запуск потока
        }

        // главный поток ждёт, пока самый долгий поток проснётся
        // максимальное число * 100 мс + небольшой запас.
        Thread.sleep(1000);

        System.out.println("Готово.");
    }
}
```

</details>

**Результат выполнения (консоль):**

```
Исходный массив:
5 2 8 1 9 3 7 
Результат sleepsort (числа печатают сами потоки):
1
2
3
5
7
8
9
Готово.
```

---
## Задание 7. Многопоточный cp -R

Реализуйте многопоточную программу рекурсивного копирования дерева подкаталогов, функциональный аналог команды cp(1) с ключом -R. Программа должна принимать два параметра – полное путевое имя корневого каталога исходного дерева и полное путевое имя целевого дерева. Программа должна обходить исходное дерево каталогов при помощи opendir(3c)/readdir_r(3с) и определять тип каждого найденного файла при помощи stat(2). Для определения размера буфера для readdir_r используйте pathconf(2) (sizeof (struct dirent) + pathconf(directory)+1).
Для каждого подкаталога должен создаваться одноименный каталог в целевом дереве и запускаться отдельная поток, обходящая этот подкаталог. Для каждого регулярного файла должна запускаться поток, копирующая этот файл в одноименный файл целевого дерева при помощи open(2)/read(2)/write(2). Файлы других типов (символические связи, именованные трубы и др.) следует игнорировать.
При копировании больших деревьев каталогов возможны проблемы с исчерпанием лимита открытых файлов. Очень важно закрывать дескрипторы обработанных файлов и каталогов при помощи close(2)/closedir(3c). Тем не менее, для очень больших деревьев этого может оказаться недостаточно. Допускается обход этой проблемы при помощи холостого цикла с ожиданием (если open(2) или readdir(3c) завершается с ошибкой EMFILE, то допускается сделать sleep(3c) и повторить попытку открытия через некоторое время).
Обратите также внимание, что значения дескрипторов открытых файлов могут переиспользоваться, т.Е. В разные моменты времени один и тот же дескриптор может указывать на разные файлы. Чтобы избежать связанных с этим проблем, избегайте передачи дескрипторов между нитями. Вся работа с дескриптором от создания до закрытия должна происходить в одной потоки.
Дополнительное упражнение: при помощи команды time(1) сравните ресурсы, потребляемые вашей программой и командой cp -R при копировании одного и того же дерева каталогов. Объясните наблюдаемые различия. Каким образом их можно устранить? Следует ли вообще реализовать копирование файлов таким способом и если да, то в каких условиях?

**Файл с кодом:** [CopyTree.java](CopyTree.java)

<details>
<summary>Показать код</summary>

```java
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.util.ArrayList;

public class CopyTree {

    // копируем одну папку файлы - сразу вложенные папки - в своих потоках.
    static void copyFolder(File from, File to) {
        to.mkdirs();  // создаём папку-приёмник

        File[] items = from.listFiles();
        if (items == null) return;

        ArrayList<Thread> threads = new ArrayList<>();

        for (File item : items) {
            File target = new File(to, item.getName());

            if (item.isDirectory()) {
                // на каждую вложенную папку отдельный поток
                Thread t = new Thread(() -> copyFolder(item, target));
                t.start();
                threads.add(t);
            } else {
                // файл копируем  здесь
                copyFile(item, target);
                System.out.println("Скопирован файл: " + target.getPath());
            }
        }

        // ждём пока все потоки вложенных папок закончат
        for (Thread t : threads) {
            try { t.join(); } catch (InterruptedException e) {}
        }
    }

    // простое копирование одного файла побайтово
    static void copyFile(File from, File to) {
        try {
            FileInputStream in = new FileInputStream(from);
            FileOutputStream out = new FileOutputStream(to);
            byte[] buffer = new byte[4096];
            int len;
            while ((len = in.read(buffer)) > 0) {
                out.write(buffer, 0, len);
            }
            in.close();
            out.close();
        } catch (Exception e) {
            System.out.println("Ошибка с файлом " + from.getName());
        }
    }

    // создаём тестовую папку с файлами чтобы было что копировать.
    static void makeTestData(String root) {
        try {
            new File(root + "/subfolder").mkdirs();
            FileWriter f1 = new FileWriter(root + "/file1.txt");
            f1.write("Привет, это файл 1"); f1.close();
            FileWriter f2 = new FileWriter(root + "/file2.txt");
            f2.write("Это файл 2"); f2.close();
            FileWriter f3 = new FileWriter(root + "/subfolder/file3.txt");
            f3.write("Файл во вложенной папке"); f3.close();
        } catch (Exception e) {
            System.out.println("Не удалось создать тестовые данные");
        }
    }

    public static void main(String[] args) {
        // откуда и куда копировать
        String source = "test_src";
        String dest = "test_copy";

        // сначала создаём тестовую папку с файлами
        makeTestData(source);

        System.out.println("Копирую папку '" + source + "' в '" + dest + "'");
        copyFolder(new File(source), new File(dest));
        System.out.println("Готово!");
    }
}
```

</details>

**Результат выполнения (консоль):**

```
C:\Users\serzh\.jdks\openjdk-26.0.1\bin\java.exe ... CopyTree
Копирую папку 'test_src' в 'test_copy'
Скопирован файл: test_copy\file1.txt
Скопирован файл: test_copy\file2.txt
Скопирован файл: test_copy\subfolder\file3.txt
Готово!
Process finished with exit code 0
```

---
## Задание 8. Вычисление Пи

Напишите программу, которая вычисляет число Пи при помощи ряда Лейбница.
Однопоточная версия такой программы доступна в файле pi_serial.C. Количество потоков программы должно определяться параметром командной строки. Количество итераций может определяться во время компиляции. Для передачи частичных сумм ряда, подсчитанных потоками, используйте pthread_exit(3c)/pthread_join(3c).
Обратите внимание, что на 32 разрядных платформах sizeof(double)>sizeof(void *), поэтому частичную сумму ряда нельзя преобразовывать к указателю, для нее надо выделять собственную память.


**Файл с кодом:** [CalcPi.java](CalcPi.java)

<details>
<summary>Показать код</summary>

```java
public class CalcPi {

    static int threadsCount = 4;        // сколько потоков 4
    static int steps = 100000000;       // сколько слагаемых всего
    static double[] partialSums;        // частичная сумма каждого потока

    public static void main(String[] args) throws InterruptedException {
        partialSums = new double[threadsCount];
        Thread[] threads = new Thread[threadsCount];

        System.out.println("Считаю Пи в " + threadsCount + " потока(ов)...");

        // Запускаем потоки. Каждому даём свой номер.
        for (int i = 0; i < threadsCount; i++) {
            final int id = i;
            threads[i] = new Thread(() -> {
                double sum = 0;
                // Поток id считает слагаемые id
                for (int k = id; k < steps; k += threadsCount) {
                    // знак: + для чётных k - для нечётных
                    if (k % 2 == 0) {
                        sum += 1.0 / (2 * k + 1);
                    } else {
                        sum -= 1.0 / (2 * k + 1);
                    }
                }
                partialSums[id] = sum;
            });
            threads[i].start();
        }

        // ждем все потоки и складываем их результаты
        double total = 0;
        for (int i = 0; i < threadsCount; i++) {
            threads[i].join();
            total += partialSums[i];
        }

        double pi = total * 4.0;
        System.out.println("Получилось Пи = " + pi);
    }
}
```

</details>

**Результат выполнения (консоль):**

```
C:\Users\serzh\.jdks\openjdk-26.0.1\bin\java.exe ... CalcPi
Считаю Пи в 4 потока(ов)...
Получилось Пи = 3.1415926435966117
Process finished with exit code 0
```

---
## Обедающие философы

Возьмите за основу программу din_phil.C. Эта программа симулирует известную задачу про обедающих философов. 
Пять философов сидят за круглым столом и едят спагетти. Спагетти едят при помощи двух вилок. 
Каждые двое философов, сидящих рядом, пользуются общей вилкой. 
Философ некоторое время размышляет, потом пытается взять вилки и принимается за еду. 
Съев некоторое количество спагетти, философ освобождает вилки и снова начинает размышлять.
 Еще через некоторое время он снова принимается за еду, и т.Д., пока спагетти не кончатся. 
Если одну из вилок взять не получается, философ ждет, пока она освободится. 
В программе din_phil.C философы симулируются при помощи потоков, периоды размышлений и еды – при помощи usleep(3c), а вилки – при помощи мутексов. Философы всегда берут сначала левую вилку, а потом правую. При некоторых обстоятельствах это может приводить к мертвой блокировке. Измените протокол взаимодействия философов с вилками таким образом, чтобы мертвых блокировок не происходило.


**Файл с кодом:** [Philosophers.java](Philosophers.java)

<details>
<summary>Показать код</summary>

```java
public class Philosophers {

    static final int N = 5;                 // количество философов и вилок
    static Object[] forks = new Object[N];  // вилки

    public static void main(String[] args) {
        // Создаём вилки.
        for (int i = 0; i < N; i++) {
            forks[i] = new Object();
        }

        // сздаём и запускаем философов
        for (int i = 0; i < N; i++) {
            final int id = i;
            new Thread(() -> philosopher(id)).start();
        }
    }

    static void philosopher(int id) {
        int left = id;              // левая вилка
        int right = (id + 1) % N;   // правая вилка

        // каждый философ поест 3 раза
        for (int meal = 1; meal <= 3; meal++) {
            System.out.println("Философ " + id + " думает...");
            sleep(100);

            // чтобы не было взаимоблокировки: последний философ
            // берёт вилки в обратном порядке.
            int first = left;
            int second = right;
            if (id == N - 1) {
                first = right;
                second = left;
            }

            synchronized (forks[first]) {
                System.out.println("Философ " + id + " взял вилку " + first);
                synchronized (forks[second]) {
                    System.out.println("Философ " + id + " взял вилку " + second);
                    System.out.println("Философ " + id + " ЕСТ (приём " + meal + ")");
                    sleep(100);
                }
            }
            System.out.println("Философ " + id + " поел и положил вилки");
        }
        System.out.println("Философ " + id + " наелся и уходит.");
    }

    static void sleep(int ms) {
        try { Thread.sleep(ms); } catch (InterruptedException e) {}
    }
}
```

</details>

**Результат выполнения (консоль):**

```
C:\Users\serzh\.jdks\openjdk-26.0.1\bin\java.exe ... Philosophers
Философ 0 думает...
Философ 3 думает...
Философ 1 думает...
Философ 4 думает...
Философ 2 думает...
Философ 2 взял вилку 2
Философ 0 взял вилку 0
Философ 1 взял вилку 1
Философ 3 взял вилку 3
Философ 3 взял вилку 4
Философ 3 ЕСТ (приём 1)
...
(философы едят по очереди, взаимоблокировки нет)
...
Философ 3 наелся и уходит.
Философ 2 наелся и уходит.
Философ 1 наелся и уходит.
Философ 0 наелся и уходит.
Философ 4 наелся и уходит.
Process finished with exit code 0
```

---