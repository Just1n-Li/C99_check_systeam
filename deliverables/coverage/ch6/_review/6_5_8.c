/*
 * 验证 C99 6.5.8 Relational operators（关系运算符）
 * 预期行为：
 * - 正向测试：能编译并运行通过，所有 assert 成立。
 * - 负向测试：违反约束，应编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：测试 <, >, <=, >= 四个运算符 */
void test_syntax(void) {
    int a = 5, b = 10;
    assert((a < b) == 1);
    assert((a > b) == 0);
    assert((a <= b) == 1);
    assert((a >= b) == 0);
}

/* [2] 约束：测试指向限定/非限定兼容对象类型的指针比较 */
void test_pointer_constraints(void) {
    int x = 0;
    int *p = &x;
    const int *cp = &x;
    volatile int *vp = &x;
    /* 指向兼容对象类型的限定与非限定指针可以比较 */
    assert((p <= cp) == 1);
    assert((p >= vp) == 1);
    assert((p < cp) == 0);
}

/* [2] 约束：测试指向兼容不完整类型的指针比较 */
void test_incomplete_type_pointers(void) {
    struct Incomplete;
    struct Incomplete *ip1 = 0;
    struct Incomplete *ip2 = 0;
    /* 两个指向兼容不完整类型的指针比较，结果相等 */
    assert((ip1 <= ip2) == 1);
    assert((ip1 >= ip2) == 1);
    assert((ip1 > ip2) == 0);
}

/* [3] 语义：算术类型执行常规算术转换 */
void test_usual_arithmetic_conversions(void) {
    int i = 5;
    double d = 5.5;
    char c = 'A'; /* ASCII 65 */
    /* int 与 double 比较，int 转换为 double */
    assert((i < d) == 1);
    assert((i > d) == 0);
    /* char 与 int 比较，char 提升为 int */
    assert((c > i) == 1);
    assert((c >= i) == 1);
}

/* [4] 语义：指向非数组元素的对象的指针，等同于指向长度为1的数组首元素的指针 */
void test_single_object_as_array(void) {
    int x = 0;
    int *p = &x;
    /* p 指向对象 x，等同于指向长度为1的数组首元素 */
    assert((p <= &x) == 1);
    assert((p >= &x) == 1);
    assert((p < &x) == 0);
    assert((p > &x) == 0);
}

/* [5] 语义：指针比较的相对位置规则 */
void test_pointer_semantics(void) {
    /* 同一对象比较 */
    int x = 0;
    int *p = &x;
    assert((p <= p) == 1);
    assert((p >= p) == 1);

    /* 结构体成员声明顺序 */
    struct S { int a; int b; } s;
    assert((&s.a < &s.b) == 1);
    assert((&s.b > &s.a) == 1);
    assert((&s.a <= &s.a) == 1);

    /* 数组元素下标顺序 */
    int arr[5] = {0};
    assert((&arr[1] < &arr[2]) == 1);
    assert((&arr[4] > &arr[0]) == 1);
    assert((&arr[0] <= &arr[0]) == 1);

    /* 联合体成员比较 */
    union U { int a; double b; } u;
    assert((&u.a <= &u.b) == 1);
    assert((&u.a >= &u.b) == 1);
    assert((&u.a > &u.b) == 0);

    /* Q+1 > P */
    int *P = &arr[0];
    int *Q = &arr[4];
    assert((Q + 1 > P) == 1);
    assert((Q + 1 >= P) == 1);
}

/* [6] 语义：结果为 1 或 0，类型为 int */
void test_result_type_and_value(void) {
    assert(sizeof(1 < 2) == sizeof(int));
    assert((1 < 2) == 1);
    assert((1 > 2) == 0);
    assert((1 <= 2) == 1);
    assert((1 >= 2) == 0);
}

/* 脚注 92：a<b<c 的结合性，解释为 (a<b)<c */
void test_chaining(void) {
    int a = 1, b = 2, c = 0;
    /* a<b 为真(1)，然后 1<c 为假(0) */
    assert((a < b < c) == 0);
    /* (a<b) 为 1，1 < 2 为真 */
    assert((a < b < 2) == 1);
}

int main(void) {
    test_syntax();
    test_pointer_constraints();
    test_incomplete_type_pointers();
    test_usual_arithmetic_conversions();
    test_single_object_as_array();
    test_pointer_semantics();
    test_result_type_and_value();
    test_chaining();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [2] 违反约束「操作数必须为实数类型」：复数类型不能比较，gcc -std=c99 应报错 */
void test_complex_comparison(void) {
    _Complex double z1 = 1.0 + 2.0i;
    _Complex double z2 = 3.0 + 4.0i;
    z1 < z2;
}

/* [2] 违反约束「指针必须指向兼容对象类型」：int* 与 double* 不兼容，gcc -std=c99 应报错 */
void test_incompatible_pointer_types(void) {
    int *pi = 0;
    double *pd = 0;
    pi < pd;
}

/* [2] 违反约束「指针必须指向兼容不完整类型」：struct A* 与 struct B* 不兼容，gcc -std=c99 应报错 */
void test_incompatible_incomplete_types(void) {
    struct A { int x; } *pa = 0;
    struct B { int x; } *pb = 0;
    pa < pb;
}
#endif