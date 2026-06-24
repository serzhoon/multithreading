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