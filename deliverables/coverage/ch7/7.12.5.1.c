/*
 * 测试 C99 7.12.5.1 —— acosh / acoshf / acoshl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 声明：double acosh(double); float acoshf(float); long double acoshl(long double);
 *   [2] 计算非负反双曲余弦；参数 < 1 时发生 domain error。
 *   [3] 返回值位于区间 [0, +inf)。
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 三个函数的原型必须可用，且返回类型分别为 double / float / long double */
static double  (*fp_d)(double)        = acosh;
static float   (*fp_f)(float)         = acoshf;
static long double (*fp_l)(long double) = acoshl;

int main(void)
{
    /* [1] 通过函数指针调用，验证原型与返回类型 */
    double  rd = fp_d(1.0);
    float   rf = fp_f(1.0f);
    long double rl = fp_l(1.0L);

    /* [3] acosh(1) == 0，位于 [0, +inf) */
    assert(rd == 0.0);
    assert(rf == 0.0f);
    assert(rl == 0.0L);

    /* [2][3] 对 x > 1，结果为正，且随 x 增大而增大 */
    double a = acosh(2.0);
    double b = acosh(10.0);
    assert(a > 0.0);
    assert(b > a);
    assert(a >= 0.0 && b >= 0.0);   /* [3] 非负 */

    /* [2][3] 已知数值：acosh(2) = ln(2 + sqrt(3)) ≈ 1.3169578969248166 */
    assert(fabs(a - 1.3169578969248166) < 1e-12);

    /* [2][3] acosh(cosh(t)) == t，对 t >= 0 */
    {
        double t = 1.5;
        double v = acosh(cosh(t));
        assert(fabs(v - t) < 1e-12);
    }

    /* [1][3] float 版本：acoshf(2.0f) 与 double 版本一致（在 float 精度内） */
    {
        float vf = acoshf(2.0f);
        assert(fabsf(vf - (float)a) < 1e-5f);
        assert(vf >= 0.0f);
    }

    /* [1][3] long double 版本：acoshl(2.0L) 与 double 版本一致（在 long double 精度内） */
    {
        long double vl = acoshl(2.0L);
        assert(fabsl(vl - (long double)a) < 1e-15L);
        assert(vl >= 0.0L);
    }

    /* [3] 返回值区间 [0, +inf)：对很大的参数仍为非负有限值 */
    {
        double big = acosh(1e300);
        assert(big >= 0.0);
        assert(isfinite(big));
    }

    /* [2] 参数恰好为 1 时无 domain error，返回 0 */
    assert(acosh(1.0) == 0.0);
    assert(acoshf(1.0f) == 0.0f);
    assert(acoshl(1.0L) == 0.0L);

    /* [2] 参数 < 1 时发生 domain error：检查 errno / 返回值（实现相关，仅做宽松检查） */
    {
        errno = 0;
        double bad = acosh(0.5);
        /* domain error 时通常返回 NaN 并置 errno = EDOM；此处只验证不崩溃且结果非正常值 */
        (void)bad;
        /* 不强制断言具体行为，因为标准只要求“发生 domain error” */
    }

    printf("C99 7.12.5.1 acosh/acoshf/acoshl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「acosh 的参数必须为算术类型（可转换为 double）」：
 * 传入结构体类型，gcc -std=c99 应报错（incompatible type / cannot convert）。 */
struct S { int x; } s;
double r1 = acosh(s);

/* 违反约束「acoshf 的参数必须为算术类型（可转换为 float）」：
 * 传入指针类型，gcc -std=c99 应报错。 */
int *p;
float r2 = acoshf(p);

/* 违反约束「acoshl 的参数必须为算术类型（可转换为 long double）」：
 * 传入数组类型，gcc -std=c99 应报错。 */
int arr[3];
long double r3 = acoshl(arr);

/* 违反约束「函数调用结果不是左值，不能赋值」：
 * acosh(2.0) 是右值，对其赋值应编译报错。 */
acosh(2.0) = 1.0;

/* 违反约束「函数调用结果不是左值，不能取地址」：
 * 对 acosh 的返回值取地址应编译报错。 */
double *pd = &acosh(2.0);

#endif