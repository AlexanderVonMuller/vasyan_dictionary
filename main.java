import java.util.HashMap;
import java.util.Map;

public class main {
    public static void main(String[] args) {
        long begin = System.currentTimeMillis();
        for(int i = 0; i < 50*1000*1000; i++) {
            Map<String, Integer> dict = new HashMap<String, Integer>();
            dict.put("1", 1);
            dict.put("2", 2);
            dict.put("3", dict.get("1") + dict.get("2"));
        }
        long end = System.currentTimeMillis();
        double diff = (double)(end - begin)/1000;
        System.out.println(diff);
    }
}
