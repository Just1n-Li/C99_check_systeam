/*
 * 测试 C99 7.12.6.10 —— log2 函数族 (log2 / log2f / log2l)
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 log2/log2f/log2l，验证返回 log2(x)，
 *             并验证定义域错误（x<0）与值域错误（x==0）的 errno 行为。
 *             整个程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 说明：log2 的“约束”极少（仅要求 <math.h> 声明、参数为实数类型），
 *       因此负向测试主要针对“未声明函数/错误参数类型”等约束。
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <float.h>

/* 允许浮点比较的容差 */
static int close_enough(double a, double b)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= 1e-12 * (1.0 + (b < 0 ? -b : b));
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可见：<math.h> 提供 double log2(double)、
     *     float log2f(float)、long double log2l(long double) 的声明。
     *     通过取函数指针验证原型存在且类型正确。 */
    {
        double (*p_d)(double)          = log2;
        float  (*p_f)(float)           = log2f;
        long double (*p_l)(long double)= log2l;
        assert(p_d != NULL && p_f != NULL && p_l != NULL);
    }

    /* [2][3] 计算以 2 为底的对数：log2(2^k) == k */
    {
        assert(close_enough(log2(1.0),  0.0));   /* log2(1)  = 0 */
        assert(close_enough(log2(2.0),  1.0));   /* log2(2)  = 1 */
        assert(close_enough(log2(4.0),  2.0));   /* log2(4)  = 2 */
        assert(close_enough(log2(8.0),  3.0));   /* log2(8)  = 3 */
        assert(close_enough(log2(1024.0), 10.0));/* log2(2^10)=10 */
        assert(close_enough(log2(0.5), -1.0));   /* log2(1/2) = -1 */
        assert(close_enough(log2(0.25),-2.0));   /* log2(1/4) = -2 */
    }

    /* [2][3] log2f 版本：float 参数、float 返回 */
    {
        float r = log2f(8.0f);
        assert(close_enough((double)r, 3.0));
        r = log2f(1.0f);
        assert(close_enough((double)r, 0.0));
    }

    /* [2][3] log2l 版本：long double 参数、long double 返回 */
    {
        long double r = log2l(16.0L);
        assert(close_enough((double)r, 4.0));
        r = log2l(1.0L);
        assert(close_enough((double)r, 0.0));
    }

    /* [2] 定义域错误：参数小于零时发生 domain error。
     *     标准要求返回 NaN（实现定义的具体值），并置 errno 为 EDOM。 */
    {
        errno = 0;
        double r = log2(-1.0);
        assert(isnan(r));          /* 定义域错误返回 NaN */
        assert(errno == EDOM);     /* 且 errno 被置为 EDOM */
    }

    /* [2] 值域错误：参数为零时可能发生 range error。
     *     标准允许返回 -HUGE_VAL（负无穷）并置 errno 为 ERANGE。
     *     由于是“may occur”，这里只做宽松检查：结果应为 -inf 或 NaN。 */
    {
        errno = 0;
        double r = log2(0.0);
        assert(isinf(r) || isnan(r));
        /* 若实现报告了值域错误，则 errno 应为 ERANGE */
        if (errno != 0) {
            assert(errno == ERANGE);
        }
    }

    /* [2][3] 一般非整数参数：log2(3) 与 log(3)/log(2) 一致 */
    {
        double a = log2(3.0);
        double b = log(3.0) / log(2.0);
        assert(close_enough(a, b));
    }

    /* [3] 返回值类型检查：log2 返回 double，log2f 返回 float，
     *     log2l 返回 long double。用 sizeof 验证返回类型宽度。 */
    {
        assert(sizeof(log2(1.0))  == sizeof(double));
        assert(sizeof(log2f(1.0f))== sizeof(float));
        assert(sizeof(log2l(1.0L))== sizeof(long double));
    }

    printf("C99 7.12.6.10 log2 functions: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「调用函数前必须有可见声明（C99 6.5.2.2）」：
     * 若未包含 <math.h>，log2 无声明，gcc -std=c99 应报
     * "implicit declaration of function 'log2'"（C99 中隐式声明为约束违反）。 */
    {
        double y = log2(4.0);   /* 无原型：应报错 */
        (void)y;
    }

    /* 违反约束「实参类型必须与形参兼容（C99 6.5.2.2）」：
     * log2 的形参为 double，传入结构体类型不兼容，应报错。 */
    {
        struct S { int x; } s;
        double y = log2(s);     /* 结构体不能转换为 double：应报错 */
        (void)y;
    }

    /* 违反约束「实参个数必须与形参个数一致（C99 6.5.2.2）」：
     * log2 只接受 1 个参数，多传参数应报错。 */
    {
        double y = log2(1.0, 2.0);  /* 参数过多：应报错 */
        (void)y;
    }

    /* 违反约束「log2 的返回值不是左值，不能赋值（C99 6.5.16）」：
     * 函数调用结果不是左值，对其赋值应报错。 */
    {
        log2(4.0) = 1.0;        /* 对非左值赋值：应报错 */
    }

    /* 违反约束「不能对函数名取地址后赋值（C99 6.5.16）」：
     * 函数指示符不是可修改左值。 */
    {
        log2 = 0;               /* 对函数名赋值：应报错 */
    }

#endif /* 负向测试结束 */

    return 0;
}