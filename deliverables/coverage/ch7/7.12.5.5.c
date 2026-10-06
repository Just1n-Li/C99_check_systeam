/*
 * 测试 C99 7.12.5.5 —— The sinh functions
 *
 * 预期行为：
 *   正向测试：<math.h> 提供 sinh / sinhf / sinhl 三个函数，
 *             分别接受 double / float / long double 参数，
 *             返回对应类型的双曲正弦值 sinh(x)。
 *             编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（参数个数错误、参数类型不可转换等）
 *             应导致编译报错；这些片段放在 #if 0 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数声明与原型
 *   [2] Description：计算双曲正弦；|x| 过大时发生 range error
 *   [3] Returns：返回 sinh x
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/* 判断浮点近似相等 */
static int close_enough(double a, double b)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= 1e-12 * (1.0 + (b < 0 ? -b : b));
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：三个函数均可用，且返回类型分别为 double/float/long double。
     *     通过赋值给对应类型变量并检查 sizeof 来验证原型存在。 */
    double      (*pd)(double)      = sinh;
    float       (*pf)(float)       = sinhf;
    long double (*pl)(long double) = sinhl;
    assert(pd != NULL && pf != NULL && pl != NULL);

    /* [1] 用显式类型调用，验证返回类型宽度 */
    {
        double      rd = sinh(0.0);
        float       rf = sinhf(0.0f);
        long double rl = sinhl(0.0L);
        assert(sizeof rd == sizeof(double));
        assert(sizeof rf == sizeof(float));
        assert(sizeof rl == sizeof(long double));
    }

    /* [3] Returns：sinh(0) == 0 */
    assert(sinh(0.0) == 0.0);
    assert(sinhf(0.0f) == 0.0f);
    assert(sinhl(0.0L) == 0.0L);

    /* [2][3] 基本数值：sinh 是奇函数，sinh(-x) == -sinh(x) */
    {
        double x = 1.25;
        assert(close_enough(sinh(-x), -sinh(x)));
        assert(close_enough(sinhf(-1.25f), -sinhf(1.25f)));
        assert(close_enough((double)sinhl(-1.25L), -(double)sinhl(1.25L)));
    }

    /* [2][3] 与定义 sinh x = (e^x - e^-x)/2 对照 */
    {
        double x = 0.75;
        double expected = (exp(x) - exp(-x)) / 2.0;
        assert(close_enough(sinh(x), expected));
    }

    /* [2][3] 与恒等式 cosh^2 - sinh^2 == 1 对照 */
    {
        double x = 1.5;
        double s = sinh(x);
        double c = cosh(x);
        assert(close_enough(c * c - s * s, 1.0));
    }

    /* [2][3] 大参数：sinh 单调递增且为正 */
    {
        double x = 5.0;
        assert(sinh(x) > 0.0);
        assert(sinh(x) > sinh(4.0));
    }

    /* [2] Description：|x| 过大时发生 range error。
     *     这里只验证“极大值”调用不会崩溃，并检查 errno/返回值行为
     *     在实现允许的范围内（HUGE_VAL 或有限值皆可，取决于实现）。
     *     注意：是否置 errno 由实现定义，故不做强制断言。 */
    {
        volatile double big = DBL_MAX;
        double r = sinh(big);
        /* 结果应为正（溢出时返回 HUGE_VAL，仍为正） */
        assert(r > 0.0 || r != r); /* r != r 处理 NaN 情形 */
    }

    /* [1] 通过 <math.h> 宏/函数名可被取地址，确认不是宏遮蔽 */
    {
        double (*fp)(double) = &sinh;
        assert(close_enough(fp(0.5), sinh(0.5)));
    }

    printf("C99 7.12.5.5 sinh functions: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * sinh 原型为 double sinh(double)，调用时给 0 个或 2 个实参，
     * gcc -std=c99 应报 "too few/many arguments to function 'sinh'"。 */
    sinh();
    sinh(1.0, 2.0);

    /* 违反约束「实参类型必须可转换为形参类型」：
     * 传入结构体，无法转换为 double，应报 incompatible type。 */
    struct S { int x; } s;
    sinh(s);

    /* 违反约束「实参类型必须可转换为形参类型」：
     * 传入指针，指针不能隐式转换为 double，应报 incompatible type。 */
    int *p = 0;
    sinh(p);

    /* 违反约束「函数返回类型不可作为赋值目标（非左值）」：
     * sinh(1.0) 是右值，对其赋值应报 "lvalue required as left operand of assignment"。 */
    sinh(1.0) = 2.0;

    /* 违反约束「取地址操作数必须是左值/函数」：
     * 对函数调用结果取地址应报错。 */
    &sinh(1.0);

    /* 违反约束「函数名不可被赋值」：
     * sinh 是函数指示符，不是可修改左值，应报错。 */
    sinh = 0;

#endif

    return 0;
}