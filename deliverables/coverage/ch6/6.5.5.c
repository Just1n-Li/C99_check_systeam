/*
 * 验证 C99 条款 6.5.5 Multiplicative operators
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 语法：测试 *, /, % 三种乘法运算符的基本语法 */
    int a = 10, b = 3;
    assert(a * b == 30);
    assert(a / b == 3);
    assert(a % b == 1);

    /* [2] 约束：算术类型操作数（整型、浮点型均可用于 * 和 /） */
    float f1 = 2.5f, f2 = 4.0f;
    double d1 = 2.5, d2 = 4.0;
    assert(f1 * f2 == 10.0f);
    assert(d1 / d2 == 0.625);

    /* [3] 语义：常规算术转换（int 与 double 运算时，int 提升为 double） */
    int i = 3;
    double d = 2.5;
    /* i 被转换为 double 3.0，结果为 7.5 */
    assert(i * d == 7.5);
    /* i 被转换为 double 3.0，结果为 1.2 */
    assert(i / d == 1.2);

    /* [4] 语义：* 运算符的结果是操作数的乘积 */
    assert(5 * 6 == 30);
    assert(2.5 * 4.0 == 10.0);
    assert(-3 * 7 == -21);

    /* [5] 语义：/ 的结果是商，% 的结果是余数 */
    assert(17 / 5 == 3);
    assert(17 % 5 == 2);
    assert(17.0 / 5.0 == 3.4);
    /* 注：第二操作数为零是未定义行为(UB)，不属于约束，故不测试 */

    /* [6] 语义：整数除法向零截断，且 (a/b)*b + a%b == a 恒等式成立 */
    /* 正数 / 正数 */
    assert(7 / 2 == 3);
    assert(7 % 2 == 1);
    assert((7 / 2) * 2 + 7 % 2 == 7);

    /* 负数 / 正数：商向零截断为 -3 */
    assert(-7 / 2 == -3);
    assert(-7 % 2 == -1);
    assert((-7 / 2) * 2 + (-7) % 2 == -7);

    /* 正数 / 负数：商向零截断为 -3 */
    assert(7 / -2 == -3);
    assert(7 % -2 == 1);
    assert((7 / -2) * (-2) + 7 % (-2) == 7);

    /* 负数 / 负数：商向零截断为 3 */
    assert(-7 / -2 == 3);
    assert(-7 % -2 == -1);
    assert((-7 / -2) * (-2) + (-7) % (-2) == -7);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* [2] 违反约束「操作数必须为算术类型」：结构体不能用于 *, /, % */
struct S { int x; } s1, s2;
s1 * s2;  /* 期望报错：invalid operands to binary * */
s1 / s2;  /* 期望报错：invalid operands to binary / */
s1 % s2;  /* 期望报错：invalid operands to binary % */

/* [2] 违反约束「操作数必须为算术类型」：指针不能用于 *, /, % */
int *p1, *p2;
p1 * p2;  /* 期望报错：invalid operands to binary * */
p1 / p2;  /* 期望报错：invalid operands to binary / */
p1 % p2;  /* 期望报错：invalid operands to binary % */

/* [2] 违反约束「% 的操作数必须为整数类型」：浮点数不能用于 % */
float fa = 1.5f, fb = 2.5f;
fa % fb;  /* 期望报错：invalid operands to binary % */

double da = 1.5, db = 2.5;
da % db;  /* 期望报错：invalid operands to binary % */

/* [2] 违反约束「% 的操作数必须为整数类型」：浮点数与整数混合也不能用 % */
int xi = 3;
double dy = 2.0;
xi % dy;  /* 期望报错：invalid operands to binary % */

#endif