

extern void func1();
extern void func2() __attribute__((weak_import));
extern void func3();
extern void func4() __attribute__((weak_import));

extern int data1;
extern int data2 __attribute__((weak_import));
extern int data3;
extern int data4 __attribute__((weak_import));
extern int data5;
extern int data6 __attribute__((weak_import));



