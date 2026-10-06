/*
 * 验证 C99 条款 6.2.7 (Compatible type and composite type)
 * 预期行为：
 * - 正向测试：能编译并运行通过，assert 验证语义。
 * - 负向测试：违反约束，编译器应报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 兼容类型：相同类型兼容，以及不同作用域内结构体兼容性测试 */
struct CompatStruct {
    int a;
    char b;
};

struct CompatStruct g_struct = {10, 'x'};

void test_compatible_type_1(void) {
    /* [1] 块作用域内声明的结构体，如果标签和成员与文件作用域一致，则类型兼容 */
    struct CompatStruct {
        int a;
        char b;
    };
    struct CompatStruct local = g_struct; /* 兼容类型之间可以赋值 */
    assert(local.a == 10 && local.b == 'x');
}

/* [2] 引用同一对象或函数的所有声明应具有兼容类型 */
extern int g_val;
int g_val = 100; /* 兼容的声明 */

int compatible_func(int x);
int compatible_func(int x) { /* 兼容的函数声明 */
    return x + 1;
}

void test_compatible_type_2(void) {
    assert(g_val == 100);
    assert(compatible_func(5) == 6);
}

/* [3] 复合类型：数组复合类型（已知大小与未知大小复合为已知大小） */
extern int arr[];
int arr[3] = {1, 2, 3}; /* 复合类型为 int[3] */

void test_composite_type_array(void) {
    assert(sizeof(arr) == 3 * sizeof(int));
    assert(arr[2] == 3);
}

/* [3] 复合类型：函数原型复合（无原型与有原型复合为有原型） */
int composite_func(int); /* 有原型 */
int composite_func(int x) { /* 复合类型为 int(int) */
    return x * 2;
}

void test_composite_type_func(void) {
    assert(composite_func(10) == 20);
}

/* [4] 内部链接标识符的复合类型 */
static int internal_func(int);
static int internal_func(int x) { /* 复合类型为 int(int) */
    return x - 1;
}

void test_composite_type_linkage(void) {
    assert(internal_func(10) == 9);
}

/* [5] EXAMPLE 复合类型构造 */
int f(int (*)(char *), double (*)[]); /* 声明1 */
int f(int (*)(char *), double (*)[3]); /* 声明2 */
/* 复合类型为: int f(int (*)(char *), double (*)[3]); */

int dummy_func(char *c) { return (int)*c; }
int f(int (*fp)(char *), double (*dp)[3]) {
    if (dp) return (int)(*dp)[0];
    if (fp) {
        char c = 'A';
        return fp(&c);
    }
    return 0;
}

void test_example_composite(void) {
    double d[3] = {42.0, 0.0, 0.0};
    int (*fp)(char *) = dummy_func;
    double (*dp)[3] = &d;
    assert(f(fp, dp) == 42);
}

int main(void) {
    test_compatible_type_1();
    test_compatible_type_2();
    test_composite_type_array();
    test_composite_type_func();
    test_composite_type_linkage();
    test_example_composite();
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束「同一作用域内声明同一对象/函数必须具有兼容类型」(6.7p4, 6.2.7[2]) */
int bad_func(int);
int bad_func(double); /* 参数类型不兼容，gcc -std=c99 应报错 */

/* 违反约束「同一作用域内结构体定义必须兼容」(6.7.2.1, 6.2.7[1]) */
struct BadStruct { int a; };
struct BadStruct { double b; }; /* 成员不同，应报错 */

/* 违反约束「同一作用域内数组声明不兼容」(6.7, 6.2.7[1]) */
extern int bad_arr[];
extern int bad_arr[3]; /* 这个在 C99 中是允许的（复合类型），不报错 */

/* 违反约束「同一作用域内变量声明不兼容」(6.7p4, 6.2.7[2]) */
extern int bad_val;
extern double bad_val; /* 类型不兼容，应报错 */
#endif