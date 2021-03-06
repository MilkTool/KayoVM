package reflection;

public class ArrayClassTest {
    
    public static void main(String[] args) {
        ArrayClassTest test = new ArrayClassTest();
        test.test();
    }

    public void test() {
        testArrayClass(boolean[].class, "[Z");
        testArrayClass(byte[].class,    "[B");
        testArrayClass(char[].class,    "[C");
        testArrayClass(short[].class,   "[S");
        testArrayClass(int[].class,     "[I");
        testArrayClass(long[].class,    "[J");
        testArrayClass(float[].class,   "[F");
        testArrayClass(double[].class,  "[D");
        testArrayClass(int[][].class,   "[[I");
        testArrayClass(Object[].class,  "[Ljava.lang.Object;");
        testArrayClass(Object[][].class,"[[Ljava.lang.Object;");
    }
    
    private void testArrayClass(Class<?> c, String name) {
//        assertEquals(name, c.getName());
//        assertEquals(Object.class, c.getSuperclass());
//        assertArrayEquals(new Class<?>[]{Cloneable.class, Serializable.class}, c.getInterfaces());
//        assertEquals(0, c.getFields().length);
//        assertEquals(0, c.getDeclaredFields().length);
//        assertEquals(9, c.getMethods().length);
//        assertEquals(0, c.getDeclaredMethods().length);
    }
    
}
