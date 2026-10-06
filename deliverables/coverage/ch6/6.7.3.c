/*
 * 验证 C99 条款 6.7.3 Type qualifiers
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：const, restrict, volatile 的基本使用 */
static void test_syntax(void) {
    const int c = 10;
    volatile int v = 20;
    int * restrict p = 0; /* restrict 只能用于指针 */
    assert(c == 10);
    assert(v == 20);
    (void)p;
}

/* [3] 语义：限定类型的属性仅对左值有意义。
   const 修饰的左值不可修改，体现了限定符对左值的约束作用。 */

/* [4] 重复限定符：同一限定符出现多次与出现一次行为相同 */
static void test_duplicate_qualifiers(void) {
    const const int cc = 5; /* 直接重复 */
    typedef const int CI;
    CI const cci = 6;       /* 通过 typedef 重复 */
    assert(cc == 5);
    assert(cci == 6);
}

/* [6] 语义：volatile 对象的读写应严格按抽象机器规则评估 */
static void test_volatile_access(void) {
    volatile int vol = 0;
    vol = 10;
    int x = vol; /* 读取 volatile 对象 */
    assert(x == 10);
}

/* [7] 语义：restrict 限定指针用于促进优化，删除不影响可观察行为 */
static void test_restrict_pointer(int * restrict p, int * restrict q) {
    *p = 10;
    *q = 20;
    assert(*p + *q == 30);
}

/* [8] 语义：数组类型包含限定符时，限定的是元素类型 */
static void test_array_qualifiers(void) {
    typedef int A[3];
    const A arr = {1, 2, 3}; /* array of const int */
    const int *pi = arr;     /* valid, arr 衰减为 const int* */
    assert(pi[0] == 1);
    assert(pi[1] == 2);
    assert(pi[2] == 3);
}

/* [9] 语义：限定符顺序不影响类型，兼容类型需有相同限定版本 */
static void test_qualifier_order(void) {
    const volatile int cv1 = 0;
    volatile const int vc1 = 0; /* 顺序不同，类型相同 */
    const volatile int *pcv1 = &cv1;
    const volatile int *pvc1 = &vc1;
    pcv1 = pvc1; /* 类型兼容，可赋值 */
    assert(pcv1 == pvc1);
}

/* [10] EXAMPLE 1: const volatile 对象可被硬件修改，但不能被程序赋值 */
static void test_example1(void) {
    extern const volatile int real_time_clock;
    int val = real_time_clock; /* 读取合法 */
    (void)val;
}

/* [11] EXAMPLE 2: 限定符修饰聚合类型时的合法行为 */
static void test_aggregate_qualifiers(void) {
    const struct s { int mem; } cs = { 1 };
    struct s ncs;                /* ncs 可修改 */
    typedef int A[2][3];
    const A a = {{4, 5, 6}, {7, 8, 9}}; /* array of array of const int */
    const int *pci;
    int *pi;

    ncs = cs;          /* valid: ncs 不是 const */
    assert(ncs.mem == 1);
    pi = &ncs.mem;     /* valid: ncs.mem 不是 const */
    assert(*pi == 1);
    pci = &cs.mem;     /* valid: pci 是 const int* */
    assert(*pci == 1);
    pci = a[0];        /* valid: a[0] 类型为 const int* */
    assert(pci[0] == 4);
}

int main(void) {
    test_syntax();
    test_duplicate_qualifiers();
    test_volatile_access();
    {
        int a = 0, b = 0;
        test_restrict_pointer(&a, &b);
    }
    test_array_qualifiers();
    test_qualifier_order();
    test_example1();
    test_aggregate_qualifiers();
    
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [2] 违反约束「Types other than pointer types derived from object or incomplete types shall not be restrict-qualified」：
   非指针类型不能被 restrict 限定，gcc -std=c99 应报错 */
restrict int x;
int restrict y;

/* [10] EXAMPLE 1 违反约束「modifiable lvalue」：
   const volatile int 变量不能被赋值、自增、自减，gcc -std=c99 应报错 */
extern const volatile int real_time_clock;
void test_example1_negative(void) {
    real_time_clock = 1; /* violates modifiable lvalue constraint */
    real_time_clock++;   /* violates constraint */
    real_time_clock--;   /* violates constraint */
}

/* [11] EXAMPLE 2 违反约束：
   cs = ncs 违反可修改左值约束；pi = &cs.mem 和 pi = a[0] 违反类型约束，gcc -std=c99 应报错 */
void test_example2_constraints(void) {
    const struct s { int mem; } cs = { 1 };
    struct s ncs;
    typedef int A[2][3];
    const A a = {{4, 5, 6}, {7, 8, 9}};
    int *pi;

    cs = ncs;        /* violates modifiable lvalue constraint for = */
    pi = &cs.mem;    /* violates type constraints for = (const int* -> int*) */
    pi = a[0];       /* invalid: a[0] has type const int* */
}
#endif