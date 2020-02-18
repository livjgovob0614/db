package smalldemos.ifx;

import static jbse.meta.Analysis.ass3rt;

public class IfExample {
    boolean a, b;
    public void m(int x) {
        if (x > 0) {
            a = true;
        } else {
            a = false;
        }
        if (x > 0) {
            b = true;
        } else {
            b = false;
        }
        ass3rt(a == b);
    }
}
