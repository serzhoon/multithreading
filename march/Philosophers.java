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