/*
 * 验证 C99 条款 6.5.9 (Equality operators)
 * 预期行为：
 * - 正向测试：能编译并运行通过，所有 assert 成立。
 * - 负向测试：违反约束，应编译报错（被放在 #if 0 块中以保证整体可编译）。
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 语法：equality-expression 支持 == 和 != */
    int a = 1, b = 2;
    assert((a == b) == 0);
    assert((a != b) == 1);

    /* [2] 约束：两个操作数均为算术类型 */
    assert(1 == 1.0);
    assert('a' != 'b');

    /* [2] 约束：两个操作数均为指向兼容类型的限定或非限定版本的指针 */
    int x = 10;
    int *p = &x;
    const int *cp = &x;
    assert(p == cp); /* 指向兼容类型的限定/非限定版本指针比较 */

    /* [2] 约束：一个是指向对象或不完整类型的指针，另一个是指向 void 的限定或非限定版本的指针 */
    void *vp = &x;
    assert((void *)p == vp);

    /* [2] 约束：一个是指针，另一个是 null 指针常量 */
    int *np = NULL;
    assert(np == 0);
    assert(np == NULL);

    /* [3] 语义：结果类型为 int，值为 1 或 0，且互斥 */
    assert(sizeof(1 == 1) == sizeof(int));
    assert(((a == 1) == 1) && ((a != 1) == 0));

    /* [4] 语义：算术类型执行常规算术转换 */
    assert(1 == 1.0f); /* int 转 float 再转 double */
    assert(1.0 == 1.0f);

    /* [4] 语义：复数类型相等当且仅当实部和虚部都相等 */
    double _Complex c1 = 1.0 + 2.0 * I;
    double _Complex c2 = 1.0 + 2.0 * I;
    double _Complex c3 = 1.0 + 3.0 * I;
    assert(c1 == c2);
    assert(c1 != c3);

    /* [4] 语义：不同类型域的算术类型转换到结果类型后比较 */
    assert(3 == 3.0 + 0.0 * I); /* int 与 double _Complex 比较，int 提升为 double _Complex */

    /* [5] 语义：指针与 null 指针常量比较，常量转换为指针类型 */
    int *q = NULL;
    assert(q == 0);

    /* [5] 语义：对象指针与 void 指针比较，前者转换为后者类型 */
    int arr[5] = {0};
    int *pa = arr;
    void *vpa = arr;
    assert((void *)pa == vpa);

    /* [6] 语义：两个 null 指针比较相等 */
    int *n1 = NULL, *n2 = 0;
    assert(n1 == n2);

    /* [6] 语义：指向同一对象（含首地址子对象）的指针比较相等 */
    struct S { int y; } s = {0};
    struct S *ps = &s;
    int *py = &s.y;
    assert((void *)ps == (void *)py);

    /* [6] 语义：指向同一数组尾后元素的指针比较相等 */
    int a5[5] = {0};
    assert(&a5[5] == a5 + 5);

    /* [6] 语义：一个指向数组尾后，另一个指向紧随其后的另一个数组首部 */
    /* 注意：标准说 "happens to immediately follow"，依赖实现，这里用结构体数组保证相邻 */
    struct { int first[5]; int second[5]; } wrapper;
    int *end_first = wrapper.first + 5;
    int *begin_second = wrapper.second;
    /* 此比较在内存连续时为真，属于合法语义比较 */
    if (end_first == begin_second) {
        /* 行为符合预期 */
    }

    /* [7] 语义：非数组对象视为长度为1的数组的首元素 */
    int single = 42;
    assert(&single == &single + 0);
    /* 尾后指针比较 */
    int *end_single = &single + 1;
    assert(end_single == &single + 1);

    /* 脚注 93：优先级，a<b == c<d 当 a<b 和 c<d 真值相同时为 1 */
    int i1 = 1, i2 = 2, i3 = 1, i4 = 2;
    assert((i1 < i2) == (i3 < i4)); /* 1 == 1 结果为 1 */
    assert((i2 < i1) == (i4 < i3)); /* 0 == 0 结果为 1 */

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束 [2]：操作数必须为算术类型或指针。结构体不能使用 == 运算符，gcc -std=c99 应报错 */
struct S2 { int x; } s1, s2;
void test_struct_eq(void) {
    s1 == s2;
}

/* 违反约束 [2]：指针必须指向兼容类型或 void。int* 和 double* 不兼容，gcc -std=c99 应报错 */
void test_incompatible_ptr(void) {
    int *pi;
    double *pd;
    pi == pd;
}

/* 违反约束 [2]：一个是指针时，另一个必须是 null 指针常量。整数 1 不是 null 常量，gcc -std=c99 应报错 */
void test_ptr_int(void) {
    int *p;
    p == 1;
}
#endif