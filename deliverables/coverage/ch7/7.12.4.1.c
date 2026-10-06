/*
 * 测试条款：C99 7.12.4.1 —— acos 函数族
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明：double acos(double); float acosf(float); long double acosl(long double);
 *   [2] 语义：计算主值反余弦；参数不在 [-1, +1] 时发生定义域错误。
 *   [3] 返回值：返回 [0, pi] 弧度区间内的 arccos x。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型可用性：三个函数均声明于 <math.h>，且返回类型分别为
 *     double / float / long double，参数类型分别为 double / float / long double。
 *     通过取函数指针并检查其类型来验证原型签名。 */
static double (*p_acos)(double)          = acos;
static float  (*p_acosf)(float)          = acosf;
static long double (*p_acosl)(long double) = acosl;

/* [3] 返回值区间 [0, pi] 的辅助检查 */
static int in_range_0_pi(double v)
{
    return v >= 0.0 && v <= acos(-1.0) + 1e-12;
}

int main(void)
{
    /* ---------- [1] 原型签名检查 ---------- */
    assert(p_acos  == acos);
    assert(p_acosf == acosf);
    assert(p_acosl == acosl);

    /* ---------- [2][3] 典型值：acos(1) == 0 ---------- */
    {
        double r = acos(1.0);
        assert(fabs(r - 0.0) < 1e-12);
        assert(in_range_0_pi(r));
    }

    /* ---------- [2][3] 典型值：acos(-1) == pi ---------- */
    {
        double pi = acos(-1.0);
        assert(fabs(pi - 3.14159265358979323846) < 1e-12);
        assert(in_range_0_pi(pi));
    }

    /* ---------- [2][3] 典型值：acos(0) == pi/2 ---------- */
    {
        double r = acos(0.0);
        assert(fabs(r - acos(-1.0) / 2.0) < 1e-12);
        assert(in_range_0_pi(r));
    }

    /* ---------- [2][3] 区间内一般值：acos(0.5) == pi/3 ---------- */
    {
        double r = acos(0.5);
        assert(fabs(r - acos(-1.0) / 3.0) < 1e-12);
        assert(in_range_0_pi(r));
    }

    /* ---------- [2][3] 区间内一般值：acos(-0.5) == 2*pi/3 ---------- */
    {
        double r = acos(-0.5);
        assert(fabs(r - 2.0 * acos(-1.0) / 3.0) < 1e-12);
        assert(in_range_0_pi(r));
    }

    /* ---------- [2][3] 单调性：x 增大，acos(x) 减小 ---------- */
    {
        double a = acos(-0.9);
        double b = acos( 0.0);
        double c = acos( 0.9);
        assert(a > b && b > c);
        assert(in_range_0_pi(a) && in_range_0_pi(b) && in_range_0_pi(c));
    }

    /* ---------- [2][3] 恒等式：acos(x) + acos(-x) == pi ---------- */
    {
        double x = 0.3;
        double s = acos(x) + acos(-x);
        assert(fabs(s - acos(-1.0)) < 1e-12);
    }

    /* ---------- [1][2][3] acosf：float 版本 ---------- */
    {
        float r = acosf(1.0f);
        assert(fabsf(r - 0.0f) < 1e-6f);

        float r2 = acosf(-1.0f);
        assert(fabsf(r2 - (float)acos(-1.0)) < 1e-6f);
        assert(r2 >= 0.0f);

        float r3 = acosf(0.0f);
        assert(fabsf(r3 - (float)(acos(-1.0) / 2.0)) < 1e-6f);
    }

    /* ---------- [1][2][3] acosl：long double 版本 ---------- */
    {
        long double r = acosl(1.0L);
        assert(fabsl(r - 0.0L) < 1e-15L);

        long double r2 = acosl(-1.0L);
        assert(fabsl(r2 - acosl(-1.0L)) < 1e-15L);
        assert(r2 >= 0.0L);

        long double r3 = acosl(0.0L);
        assert(fabsl(r3 - acosl(-1.0L) / 2.0L) < 1e-15L);
    }

    /* ---------- [2] 边界值：x = +1 与 x = -1 属于定义域 ---------- */
    {
        double rp = acos( 1.0);
        double rn = acos(-1.0);
        assert(rp == 0.0 || fabs(rp) < 1e-12);
        assert(in_range_0_pi(rp));
        assert(in_range_0_pi(rn));
    }

    /* ---------- [2] 定义域错误：参数超出 [-1, +1] 时发生 domain error。
     *      C99 7.12.1 规定：定义域错误时函数返回实现定义的值，并可能设置 errno 为 EDOM。
     *      这里只验证「调用不会崩溃、返回值仍落在 [0, pi] 或为 NaN」这一可移植行为，
     *      不把 errno 的具体取值作为硬性断言（实现定义）。 ---------- */
    {
        double r = acos(2.0);   /* 超出定义域 */
        assert(isnan(r) || in_range_0_pi(r));

        double r2 = acos(-2.0); /* 超出定义域 */
        assert(isnan(r2) || in_range_0_pi(r2));
    }

    printf("C99 7.12.4.1 acos/acosf/acosl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「acos 的参数必须为算术类型（实浮点类型）」：
 * 传入结构体类型，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'acos' */
struct S { int x; } s;
double bad1 = acos(s);

/* 违反约束「acos 的参数必须为算术类型」：
 * 传入指针类型，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'acos' */
int *p = 0;
double bad2 = acos(p);

/* 违反约束「acos 的参数必须为算术类型」：
 * 传入 void 表达式（函数无返回值），gcc -std=c99 应报错。 */
void f(void);
double bad3 = acos(f());

/* 违反约束「acos 的参数必须为算术类型」：
 * 传入数组类型，gcc -std=c99 应报错。 */
int arr[3];
double bad4 = acos(arr);

/* 违反约束「acos 的参数必须为算术类型」：
 * 传入函数指针类型，gcc -std=c99 应报错。 */
double (*fp)(double) = acos;
double bad5 = acos(fp);

/* 违反约束「acos 的参数必须为算术类型」：
 * 传入联合体类型，gcc -std=c99 应报错。 */
union U { int i; double d; } u;
double bad6 = acos(u);

/* 违反约束「acos 的参数必须为算术类型」：
 * 传入字符串字面量（char*），gcc -std=c99 应报错。 */
double bad7 = acos("hello");

/* 违反约束「acos 的参数必须为算术类型」：
 * 传入复数类型（C99 中 _Complex 不是实浮点类型，acos 不接受），
 * gcc -std=c99 应报错。 */
double _Complex z = 1.0 + 0.0 * _Complex_I;
double bad8 = acos(z);

#endif /* 负向测试结束 */