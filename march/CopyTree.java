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