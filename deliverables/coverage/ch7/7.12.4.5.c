/*
 * 测试目标：C99 7.12.4.5 —— The cos functions
 *
 * 条款要点：
 *   [1] 头文件 <math.h>，三个函数原型：
 *         double      cos (double x);
 *         float       cosf(float x);
 *         long double cosl(long double x);
 *   [2] cos 函数计算 x（以弧度为单位）的余弦。
 *   [3] cos 函数返回 cos x。
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可正常编译运行）。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，若原型不匹配则编译失败 */
static double      (*p_cos )(double)      = cos;
static float       (*p_cosf)(float)       = cosf;
static long double (*p_cosl)(long double) = cosl;

/* 辅助：浮点近似比较 */
static int nearly_equal(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* [1] 三个函数均可调用，返回类型分别为 double / float / long double */
    double      d = cos(0.0);
    float       f = cosf(0.0f);
    long double l = cosl(0.0L);

    /* [3] cos(0) == 1 */
    assert(nearly_equal(d, 1.0, 1e-12));
    assert(nearly_equal((double)f, 1.0, 1e-6));
    assert(nearly_equal((double)l, 1.0, 1e-12));

    /* [2][3] cos(pi) == -1（pi 以弧度计） */
    {
        double pi = 3.14159265358979323846;
        assert(nearly_equal(cos(pi), -1.0, 1e-12));
        assert(nearly_equal((double)cosf((float)pi), -1.0, 1e-5));
        assert(nearly_equal((double)cosl((long double)pi), -1.0, 1e-12));
    }

    /* [2][3] cos(pi/2) == 0 */
    {
        double half_pi = 1.57079632679489661923;
        assert(nearly_equal(cos(half_pi), 0.0, 1e-12));
    }

    /* [2][3] cos(pi/3) == 0.5 */
    {
        double pi_over_3 = 1.04719755119659774615;
        assert(nearly_equal(cos(pi_over_3), 0.5, 1e-12));
    }

    /* [2] 偶函数性质：cos(-x) == cos(x) */
    {
        double x = 0.7;
        assert(nearly_equal(cos(-x), cos(x), 1e-12));
    }

    /* [1] 参数类型：整型实参经默认实参提升/转换后仍可调用 */
    assert(nearly_equal(cos(0), 1.0, 1e-12));

    /* [1] 函数指针调用 */
    assert(nearly_equal(p_cos(0.0), 1.0, 1e-12));
    assert(nearly_equal((double)p_cosf(0.0f), 1.0, 1e-6));
    assert(nearly_equal((double)p_cosl(0.0L), 1.0, 1e-12));

    /* [3] 返回值可参与算术运算 */
    {
        double s = cos(0.0) + cos(0.0);
        assert(nearly_equal(s, 2.0, 1e-12));
    }

    printf("C99 7.12.4.5 cos functions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「cos 的参数必须可转换为 double」：
 * 结构体类型无法隐式转换为 double，gcc -std=c99 应报错。 */
struct S { int x; } s;
double bad1 = cos(s);

/* 违反约束「cos 的参数必须可转换为 double」：
 * 指针类型无法隐式转换为 double，应报错。 */
double bad2 = cos((int *)0);

/* 违反约束「cos 的参数个数必须为 1」：
 * 参数个数不匹配，应报错。 */
double bad3 = cos(1.0, 2.0);

/* 违反约束「cos 的参数个数必须为 1」：
 * 缺少参数，应报错。 */
double bad4 = cos();

/* 违反约束「cos 的返回类型为 double，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
cos(0.0) = 1.0;

/* 违反约束「cosf 的返回类型为 float，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
cosf(0.0f) = 1.0f;

/* 违反约束「cosl 的返回类型为 long double，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
cosl(0.0L) = 1.0L;

/* 违反约束「cos 的返回类型为 double，不可取地址」：
 * 函数调用结果不是左值，& 操作数必须是左值，应报错。 */
double *bad5 = &cos(0.0);

#endif