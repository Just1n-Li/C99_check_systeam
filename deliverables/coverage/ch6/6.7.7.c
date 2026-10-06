/*
 * 验证 C99 条款 6.7.7 (Type definitions)
 * 预期行为：正向测试应能编译并运行通过，负向测试应编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] 语义：typedef 不引入新类型，只是同义词；共享相同命名空间 */
typedef int IntAlias;
void test_typedef_synonym_and_namespace() {
    IntAlias a = 10;
    int b = 20;
    /* 验证类型兼容性：IntAlias 和 int 是兼容的类型，可以互相赋值 */
    a = b;
    b = a;
    assert(a == 20 && b == 20);
}

/* [4] 示例 1：MILES, KLICKSP(), range 的使用 */
typedef int MILES, KLICKSP();
typedef struct { double hi, lo; } range;

int klicksp_func() { return 42; }
void test_example_1() {
    MILES distance = 100;
    extern KLICKSP *metricp; /* pointer to function with no parameter specification returning int */
    metricp = klicksp_func;
    range x = {1.0, 2.0};
    range z, *zp;
    
    assert(distance == 100);
    assert(metricp() == 42);
    z = x;
    zp = &x;
    assert(z.hi == 1.0 && zp->lo == 2.0);
}

/* [5] 示例 2：类型兼容性测试 */
typedef struct s1 { int x; } t1, *tp1;
typedef struct s2 { int x; } t2, *tp2;
void test_example_2() {
    t1 var1 = {1};
    struct s1 var2 = {2};
    /* t1 和 struct s1 是兼容类型 */
    var1 = var2;
    assert(var1.x == 2);
    
    tp1 ptr1 = &var1;
    struct s1 *ptr2 = &var2;
    ptr1 = ptr2; /* tp1 和 struct s1* 是兼容类型 */
    assert(ptr1->x == 2);
}

/* [6] 示例 3：typedef 名字在位域与晦涩构造中的使用 */
typedef signed int t;
typedef int plain;
struct tag {
    unsigned t:4;   /* t is a structure member here, because unsigned is a type specifier */
    const t:5;      /* t is still visible as typedef name, modified by const qualifier */
    plain r:5;       /* plain is visible as typedef name */
};
void test_example_3() {
    struct tag my_tag = {15, .r = 31};
    assert(my_tag.t == 15);
    assert(my_tag.r == 31); /* or implementation-defined range, but 31 fits in [-16, 15] or [0, 31] */
    
    /* 晦涩构造：t f(t (t)); long t; */
    {
        /* t f(t (t)); declares function f returning signed int, with one unnamed parameter 
           of type pointer to function returning signed int with one unnamed parameter of type signed int */
        t f(t (t));
        long t; /* identifier t with type long int, hides typedef t */
        t = 100L;
        assert(t == 100L);
    }
}

/* [7] 示例 4：signal 函数的三种等价声明 */
typedef void fv(int), (*pfv)(int);
/* void (*my_signal1(int, void (*)(int)))(int); */
/* fv *my_signal2(int, fv *); */
/* pfv my_signal3(int, pfv); */
void test_example_4() {
    /* 验证三种声明类型的兼容性，通过函数指针互相赋值 */
    void (*(*fp1)(int, void (*)(int)))(int) = 0;
    fv *(*fp2)(int, fv *) = 0;
    pfv (*fp3)(int, pfv) = 0;
    
    fp1 = fp2;
    fp2 = fp3;
    fp3 = fp1;
    assert(fp1 == fp2 && fp2 == fp3);
}

/* [8] 示例 5：VLA typedef 的求值时机 */
void copyt(int n) {
    typedef int B[n]; // B is n ints, n evaluated now
    n += 1;
    B a; // a is n ints, n without += 1
    int b[n]; // a and b are different sizes
    
    /* 验证 a 的大小是原始 n，b 的大小是 n+1 */
    assert(sizeof(a) == sizeof(int) * (n - 1));
    assert(sizeof(b) == sizeof(int) * n);
    
    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    
    for (int i = 1; i < n; i++)
        a[i-1] = b[i];
}

/* [3] 语义：VLA typedef 的数组大小表达式每次执行到时求值 */
void test_vla_typedef_evaluation() {
    int n = 5;
    copyt(n);
}

int main() {
    test_typedef_synonym_and_namespace();
    test_example_1();
    test_example_2();
    test_example_3();
    test_example_4();
    test_vla_typedef_evaluation();
    
    printf("All positive tests passed!\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「如果 typedef 名字指定了一个可变修改类型，那么它必须具有块作用域」 */
/* 文件作用域的 VLA typedef，gcc -std=c99 应报错 */
int n = 10;
typedef int VLA[n]; /* 错误: variably modified type at file scope */

#endif