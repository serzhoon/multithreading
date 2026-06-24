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
