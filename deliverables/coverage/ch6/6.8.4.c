/*
 * 验证 C99 条款 6.8.4 (Selection statements)
 * 预期行为：
 * 正向测试：能编译并运行通过，断言成功。
 * 负向测试：违反语法和作用域约束，编译器应报错。
 */

#include <stdio.h>
#include <assert.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 语法：if ( expression ) statement */
    int a = 1;
    if (a) a = 2;
    assert(a == 2);

    /* [1] 语法：if ( expression ) statement else statement */
    int b = 0;
    if (b) b = 1; else b = 2;
    assert(b == 2);

    /* [1] 语法：switch ( expression ) statement */
    int c = 2;
    switch (c) {
        case 1: c = 10; break;
        case 2: c = 20; break;
        default: c = 0; break;
    }
    assert(c == 20);

    /* [2] 语义：选择语句根据控制表达式的值在一组语句中进行选择 */
    int d = 5;
    if (d > 3) d = 100; else d = 200;
    assert(d == 100);

    int e = 1;
    switch (e) {
        case 1: e = 10; break;
        case 2: e = 20; break;
    }
    assert(e == 10);

    /* [3] 语义：选择语句是一个块，其作用域是外围块作用域的严格子集。
       在控制表达式中声明的变量，其作用域覆盖整个选择语句及其子语句。 */
    if (int g = 5) {
        assert(g == 5); /* 子语句中可见 */
    }
    /* g 在此处不可见，不使用它以保持正向测试通过 */

    /* [3] 语义：每个关联的子语句也是一个块，其作用域是选择语句作用域的严格子集。 */
    if (int h = 1) {
        int h_inner = 10; /* 子语句块内的变量 */
        assert(h_inner == 10);
    } else {
        int h_inner = 20; /* 另一个子语句块内的同名变量，互不干扰 */
        assert(h_inner == 20);
    }

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 违反语法 [1]：if 语句缺少括号，gcc -std=c99 应报错 */
    int x = 1;
    if x x = 2;

    /* 违反语法 [1]：switch 语句缺少括号，gcc -std=c99 应报错 */
    int y = 1;
    switch y { case 1: y = 2; }

    /* 违反语义 [3]：选择语句是一个块，控制表达式中声明的变量在语句外不可见。
       gcc -std=c99 应报错：'g' undeclared */
    if (int g_err = 5) {
        g_err = 10;
    }
    g_err = 20;

    /* 违反语义 [3]：子语句是一个块，其作用域是严格子集。
       在一个子语句中声明的变量，在另一个子语句中不可见。
       gcc -std=c99 应报错：'h_inner' undeclared */
    if (1) {
        int h_inner = 10;
    } else {
        h_inner = 20;
    }
#endif

    printf("6.8.4 测试通过。\n");
    return 0;
}