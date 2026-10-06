/*
 * 测试 C99 7.12.10.1 —— fmod 函数族 (fmod / fmodf / fmodl)
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             这些片段放在 #if 0 中，保证本文件仍可正常编译。
 *
 * 覆盖段落：
 *   [1] 头文件 <math.h> 与三个函数原型（double/float/long double 版本）
 *   [2] 计算 x/y 的浮点余数
 *   [3] 返回值 = x - n*y，n 为整数；y 非零时结果符号与 x 相同，
 *       且绝对值小于 |y|；y 为零时行为由实现定义。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证三个版本均已声明且类型正确 */
static double (*p_fmod)(double, double)   = fmod;
static float  (*p_fmodf)(float, float)    = fmodf;
static long double (*p_fmodl)(long double, long double) = fmodl;

/* 辅助：判断浮点值是否近似相等 */
static int approx(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* [1] 三个函数指针非空，说明原型可用 */
    assert(p_fmod  != NULL);
    assert(p_fmodf != NULL);
    assert(p_fmodl != NULL);

    /* [2][3] 基本余数计算：fmod(5.0, 3.0) == 2.0 */
    {
        double r = fmod(5.0, 3.0);
        assert(approx(r, 2.0, 1e-12));
    }

    /* [3] 结果符号与 x 相同：x 为负时结果非正 */
    {
        double r = fmod(-5.0, 3.0);
        assert(approx(r, -2.0, 1e-12));
        assert(r <= 0.0);            /* 符号与 x 相同 */
    }

    /* [3] 结果符号与 x 相同：x 为正时结果非负 */
    {
        double r = fmod(5.0, -3.0);
        assert(approx(r, 2.0, 1e-12));
        assert(r >= 0.0);            /* 符号与 x 相同，而非与 y 相同 */
    }

    /* [3] 绝对值小于 |y| */
    {
        double x = 7.5, y = 2.0;
        double r = fmod(x, y);
        assert(fabs(r) < fabs(y));
        /* 且存在整数 n 使 r == x - n*y */
        double n = (x - r) / y;
        assert(approx(n, floor(n + 0.5), 1e-9)); /* n 为整数 */
    }

    /* [3] |x| < |y| 时结果为 x 本身 */
    {
        double r = fmod(1.5, 4.0);
        assert(approx(r, 1.5, 1e-12));
    }

    /* [3] x 为 0 时结果为 0 */
    {
        double r = fmod(0.0, 3.0);
        assert(r == 0.0);
    }

    /* [3] 精确整除时结果为 0 */
    {
        double r = fmod(9.0, 3.0);
        assert(r == 0.0);
    }

    /* [3] 大数/小数组合，验证 x - n*y 关系 */
    {
        double x = 123.456, y = 10.0;
        double r = fmod(x, y);
        assert(fabs(r) < fabs(y));
        assert(r >= 0.0);            /* x 为正 */
        double n = (x - r) / y;
        assert(approx(n, floor(n + 0.5), 1e-9));
    }

    /* [1][2][3] float 版本 fmodf */
    {
        float r = fmodf(5.5f, 2.0f);
        assert(fabsf(r - 1.5f) < 1e-6f);
        assert(fabsf(r) < 2.0f);
        assert(r >= 0.0f);
    }

    /* [1][2][3] long double 版本 fmodl */
    {
        long double r = fmodl(5.5L, 2.0L);
        assert(fabsl(r - 1.5L) < 1e-18L);
        assert(fabsl(r) < 2.0L);
        assert(r >= 0.0L);
    }

    /* [3] y 为零：实现定义（可能域错误或返回零）。
     *     此处只验证「可调用且不崩溃」，不强制具体结果。 */
    {
        volatile double zero = 0.0;
        double r = fmod(3.0, zero);
        (void)r;   /* 结果由实现定义，不做断言 */
    }

    printf("C99 7.12.10.1 fmod tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fmod 的参数必须为算术类型（可转换为 double）」：
 * 传入结构体类型，gcc -std=c99 应报错（incompatible type / 无法转换）。 */
struct S { int x; } s;
double bad1 = fmod(s, 2.0);

/* 违反约束「fmod 需要两个参数」：
 * 参数个数不匹配，应报错。 */
double bad2 = fmod(1.0);

/* 违反约束「fmod 需要两个参数」：
 * 参数过多，应报错。 */
double bad3 = fmod(1.0, 2.0, 3.0);

/* 违反约束「fmodf 的参数必须可转换为 float」：
 * 传入指针类型，应报错。 */
double bad4 = fmodf((void *)0, 1.0f);

/* 违反约束「fmodl 的参数必须可转换为 long double」：
 * 传入结构体，应报错。 */
double bad5 = fmodl(s, 1.0L);

/* 违反约束「fmod 返回 double，不能作为左值被赋值」：
 * 函数调用结果不是左值，赋值应报错。 */
fmod(1.0, 2.0) = 3.0;

/* 违反约束「fmod 返回非左值，不能取地址」：
 * 对函数返回值取地址应报错。 */
double *bad6 = &fmod(1.0, 2.0);

#endif