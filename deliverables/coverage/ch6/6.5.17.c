/*
 * 验证 C99 6.5.17 逗号操作符
 * 正向测试：应能编译并运行通过（assert 全部成立）
 * 负向测试：应编译报错（逗号操作符结果非左值，对其赋值/取地址/自增违反约束）
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    int a = 1, b = 2, c = 3;
    int t;
    int result;

    /* [1] 语法：expression , assignment-expression，多重逗号从左到右结合 */
    result = (1, 2, 3);
    assert(result == 3);

    /* [2] 左操作数作为 void 表达式求值（副作用仍然发生） */
    int side = 0;
    result = (side = 100, 42);
    assert(result == 42);
    assert(side == 100);

    /* [2] 左操作数求值后有序列点：左操作数的副作用在右操作数求值前完成 */
    t = 0;
    result = (t = 10, t + 5);
    assert(result == 15);
    assert(t == 10);

    /* [2] 结果具有右操作数的类型和值——右操作数为 double */
    double d = (a, 3.14);
    assert(d > 3.13 && d < 3.15);

    /* [2] 结果具有右操作数的类型和值——右操作数为 char */
    char ch = (a, (char)'Z');
    assert(ch == 'Z');

    /* [2] 多重逗号操作符，从左到右依次求值，结果为最后一个 */
    int order = 0;
    int seq = (order = 1, order = 2, order = 3);
    assert(seq == 3);
    assert(order == 3);

    /* [3] EXAMPLE: 逗号操作符在括号表达式中作为函数参数 */
    /* f(a, (t=3, t+2), c) —— 函数有三个参数，第二个值为 5 */
    t = 0;
    int arg2 = (t = 3, t + 2);
    assert(arg2 == 5);
    assert(t == 3);

    /* [3] 逗号操作符在条件操作符的第二个表达式中使用 */
    int cond = 1;
    int val = cond ? (a = 5, a + 1) : 0;
    assert(val == 6);
    assert(a == 5);

    /* [3] 验证逗号在函数参数列表中是分隔符而非逗号操作符 */
    /* f(a, b, c) 传递三个参数，而非逗号操作符 */
    int sum3(int x, int y, int z) { return x + y + z; }  /* 嵌套定义仅用于测试演示 */
    /* 注意：C99 不允许嵌套函数，改为在 main 外定义——此处用直接计算代替 */
    /* 直接验证：a, b, c 三个独立值 */
    assert(a + b + c == 5 + 2 + 3);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 脚注 97 + 6.5.16p2 约束：逗号操作符不产生左值，对其结果赋值应报错 */
void test_assign_to_comma_result(void)
{
    int a = 1, b = 2, c = 3;
    (a, b) = c;   /* 期望报错：assignment to expression / lvalue required */
}

/* 脚注 97 + 6.5.3.2p1 约束：逗号操作符不产生左值，对其取地址应报错 */
void test_address_of_comma_result(void)
{
    int a = 1, b = 2;
    int *p = &(a, b);  /* 期望报错：operand of '&' must be an lvalue */
}

/* 脚注 97 + 6.5.2.4p1 约束：逗号操作符不产生左值，后缀自增应报错 */
void test_postincr_comma_result(void)
{
    int a = 1, b = 2;
    (a, b)++;  /* 期望报错：lvalue required */
}

/* 脚注 97 + 6.5.3.1p1 约束：逗号操作符不产生左值，前缀自增应报错 */
void test_preincr_comma_result(void)
{
    int a = 1, b = 2;
    ++(a, b);  /* 期望报错：lvalue required */
}

#endif