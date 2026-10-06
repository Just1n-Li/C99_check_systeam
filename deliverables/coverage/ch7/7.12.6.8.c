/*
 * 测试条款：C99 7.12.6.8  The log10 functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 log10 / log10f / log10l，
 *             验证其返回以 10 为底的对数；验证负参数产生定义域错误
 *             （返回 NaN 并置 errno == EDOM）；验证零参数可能产生
 *             范围错误（返回 -HUGE_VAL 并可能置 errno == ERANGE）。
 *   负向测试：违反约束的代码（如参数个数错误、对非算术类型调用等）
 *             应导致编译报错，统一放在 #if 0 中。
 *
 * 编译：gcc -std=c99 -Wall -lm test.c
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <float.h>
#include <assert.h>

/* 判断 double 是否为 NaN（不依赖 C99 的 isnan 宏，避免额外依赖） */
static int is_nan_d(double x) { return x != x; }

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：三个函数均可用，返回类型分别为 double/float/long double */
    {
        double      (*pd)(double)          = log10;
        float       (*pf)(float)           = log10f;
        long double (*pl)(long double)     = log10l;
        assert(pd != NULL && pf != NULL && pl != NULL);
    }

    /* [2][3] log10 计算以 10 为底的对数：log10(1)=0, log10(10)=1, log10(100)=2 */
    {
        double r1 = log10(1.0);
        double r2 = log10(10.0);
        double r3 = log10(100.0);
        assert(fabs(r1 - 0.0) < 1e-12);
        assert(fabs(r2 - 1.0) < 1e-12);
        assert(fabs(r3 - 2.0) < 1e-12);
    }

    /* [2][3] log10f 计算 float 版本 */
    {
        float r = log10f(1000.0f);
        assert(fabsf(r - 3.0f) < 1e-5f);
    }

    /* [2][3] log10l 计算 long double 版本 */
    {
        long double r = log10l(10000.0L);
        assert(fabsl(r - 4.0L) < 1e-15L);
    }

    /* [2] 定义域错误：参数为负 -> 返回 NaN 并置 errno == EDOM */
    {
        errno = 0;
        double r = log10(-1.0);
        assert(is_nan_d(r));          /* 返回 NaN */
        assert(errno == EDOM);        /* 定义域错误 */
    }

    /* [2] 定义域错误（float 版本） */
    {
        errno = 0;
        float r = log10f(-2.5f);
        assert(r != r);               /* NaN */
        assert(errno == EDOM);
    }

    /* [2] 定义域错误（long double 版本） */
    {
        errno = 0;
        long double r = log10l(-3.0L);
        assert(r != r);               /* NaN */
        assert(errno == EDOM);
    }

    /* [2] 范围错误：参数为 0 -> 返回 -HUGE_VAL，可能置 errno == ERANGE */
    {
        errno = 0;
        double r = log10(0.0);
        assert(r == -HUGE_VAL);       /* 返回 -HUGE_VAL */
        /* 范围错误“可能”发生，因此 errno 可能为 ERANGE 也可能不变，
           这里只验证返回值，不强制断言 errno。 */
    }

    /* [2] 范围错误（float 版本）：返回 -HUGE_VALF */
    {
        errno = 0;
        float r = log10f(0.0f);
        assert(r == -HUGE_VALF);
    }

    /* [2] 范围错误（long double 版本）：返回 -HUGE_VALL */
    {
        errno = 0;
        long double r = log10l(0.0L);
        assert(r == -HUGE_VALL);
    }

    /* [3] 返回值语义：log10(x) 与 log(x)/log(10) 一致（数值一致性检查） */
    {
        double x = 42.0;
        double a = log10(x);
        double b = log(x) / log(10.0);
        assert(fabs(a - b) < 1e-12);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 违反约束「函数调用实参个数必须与原型一致」：
       log10 原型只接受 1 个参数，传 2 个参数，gcc -std=c99 应报错
       （error: too many arguments to function 'log10'）。 */
    double bad1 = log10(1.0, 2.0);

    /* 违反约束「函数调用实参个数必须与原型一致」：
       log10 原型要求 1 个参数，传 0 个参数，应报错
       （error: too few arguments to function 'log10'）。 */
    double bad2 = log10();

    /* 违反约束「实参类型必须可转换为形参类型」：
       结构体类型无法隐式转换为 double，应报错
       （error: incompatible type for argument 1 of 'log10'）。 */
    struct S { int x; } s;
    double bad3 = log10(s);

    /* 违反约束「函数返回类型为算术类型，不能作为赋值目标」：
       log10 的返回值不是左值，对其赋值应报错
       （error: lvalue required as left operand of assignment）。 */
    log10(10.0) = 1.0;

    /* 违反约束「不能对函数返回值取地址」：
       log10 的返回值不是左值，取地址应报错
       （error: lvalue required as unary '&' operand）。 */
    double *bad4 = &log10(10.0);
#endif

    return 0;
}