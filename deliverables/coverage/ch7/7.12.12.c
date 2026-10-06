/*
 * C99 7.12.12 —— Maximum, minimum, and positive difference functions
 * 头文件 <math.h> 声明：
 *   double fmax(double x, double y);
 *   float  fmaxf(float x, float y);
 *   long double fmaxl(long double x, long double y);
 *   double fmin(double x, double y);
 *   float  fminf(float x, float y);
 *   long double fminl(long double x, long double y);
 *   double fdim(double x, double y);
 *   float  fdimf(float x, float y);
 *   long double fdiml(long double x, long double y);
 *
 * 语义（C99 7.12.12）：
 *   fmax(x,y) 返回 x、y 中较大的值；若一者为 NaN，返回另一者。
 *   fmin(x,y) 返回 x、y 中较小的值；若一者为 NaN，返回另一者。
 *   fdim(x,y) 返回 x - y（当 x > y），否则返回 +0。
 *
 * 预期行为：
 *   正向测试：编译通过、运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* 辅助：判断是否为 NaN（不依赖 C99 的 isnan 宏，避免额外条款依赖） */
static int my_isnan(double v)
{
    return v != v;
}

int main(void)
{
    /* ---------- [1] fmax / fmaxf / fmaxl：返回较大值 ---------- */

    /* fmax：普通数值 */
    assert(fmax(1.0, 2.0) == 2.0);
    assert(fmax(2.0, 1.0) == 2.0);
    assert(fmax(-1.0, -2.0) == -1.0);
    assert(fmax(-2.0, -1.0) == -1.0);
    assert(fmax(0.0, 0.0) == 0.0);

    /* fmax：相等时返回该值 */
    assert(fmax(3.5, 3.5) == 3.5);

    /* fmax：含 NaN —— 返回非 NaN 的那个操作数 */
    {
        double nan_val = 0.0 / 0.0;   /* 产生 NaN（实现支持 IEEE 时） */
        if (my_isnan(nan_val)) {
            assert(fmax(nan_val, 5.0) == 5.0);
            assert(fmax(5.0, nan_val) == 5.0);
            assert(fmax(nan_val, -5.0) == -5.0);
            assert(fmax(-5.0, nan_val) == -5.0);
            /* 两个都是 NaN 时返回 NaN */
            assert(my_isnan(fmax(nan_val, nan_val)));
        }
    }

    /* fmaxf：float 版本 */
    {
        float r = fmaxf(1.5f, 2.5f);
        assert(r == 2.5f);
        r = fmaxf(-1.5f, -2.5f);
        assert(r == -1.5f);
    }

    /* fmaxl：long double 版本 */
    {
        long double r = fmaxl(1.5L, 2.5L);
        assert(r == 2.5L);
        r = fmaxl(-1.5L, -2.5L);
        assert(r == -1.5L);
    }

    /* ---------- [1] fmin / fminf / fminl：返回较小值 ---------- */

    /* fmin：普通数值 */
    assert(fmin(1.0, 2.0) == 1.0);
    assert(fmin(2.0, 1.0) == 1.0);
    assert(fmin(-1.0, -2.0) == -2.0);
    assert(fmin(-2.0, -1.0) == -2.0);
    assert(fmin(0.0, 0.0) == 0.0);

    /* fmin：相等时返回该值 */
    assert(fmin(3.5, 3.5) == 3.5);

    /* fmin：含 NaN —— 返回非 NaN 的那个操作数 */
    {
        double nan_val = 0.0 / 0.0;
        if (my_isnan(nan_val)) {
            assert(fmin(nan_val, 5.0) == 5.0);
            assert(fmin(5.0, nan_val) == 5.0);
            assert(fmin(nan_val, -5.0) == -5.0);
            assert(fmin(-5.0, nan_val) == -5.0);
            assert(my_isnan(fmin(nan_val, nan_val)));
        }
    }

    /* fminf：float 版本 */
    {
        float r = fminf(1.5f, 2.5f);
        assert(r == 1.5f);
        r = fminf(-1.5f, -2.5f);
        assert(r == -2.5f);
    }

    /* fminl：long double 版本 */
    {
        long double r = fminl(1.5L, 2.5L);
        assert(r == 1.5L);
        r = fminl(-1.5L, -2.5L);
        assert(r == -2.5L);
    }

    /* ---------- [1] fdim / fdimf / fdiml：正差 ---------- */

    /* fdim：x > y 时返回 x - y */
    assert(fdim(5.0, 3.0) == 2.0);
    assert(fdim(3.0, 5.0) == 0.0);   /* x <= y 时返回 +0 */
    assert(fdim(3.0, 3.0) == 0.0);   /* 相等时返回 +0 */
    assert(fdim(-1.0, -3.0) == 2.0);
    assert(fdim(-3.0, -1.0) == 0.0);

    /* fdim：返回 +0 时符号为正 */
    {
        double z = fdim(1.0, 2.0);
        assert(z == 0.0);
        /* +0 与 -0 比较相等，但用 1.0/z 可区分符号（IEEE 下） */
        if (1.0 / z > 0.0) {
            /* 确认是 +0 */
            assert(1.0 / z > 0.0);
        }
    }

    /* fdim：含 NaN —— 返回 NaN */
    {
        double nan_val = 0.0 / 0.0;
        if (my_isnan(nan_val)) {
            assert(my_isnan(fdim(nan_val, 1.0)));
            assert(my_isnan(fdim(1.0, nan_val)));
            assert(my_isnan(fdim(nan_val, nan_val)));
        }
    }

    /* fdimf：float 版本 */
    {
        float r = fdimf(5.0f, 3.0f);
        assert(r == 2.0f);
        r = fdimf(3.0f, 5.0f);
        assert(r == 0.0f);
    }

    /* fdiml：long double 版本 */
    {
        long double r = fdiml(5.0L, 3.0L);
        assert(r == 2.0L);
        r = fdiml(3.0L, 5.0L);
        assert(r == 0.0L);
    }

    /* ---------- 返回值类型检查（通过赋值给对应类型变量） ---------- */

    {
        double   d1 = fmax(1.0, 2.0);
        double   d2 = fmin(1.0, 2.0);
        double   d3 = fdim(1.0, 2.0);
        float    f1 = fmaxf(1.0f, 2.0f);
        float    f2 = fminf(1.0f, 2.0f);
        float    f3 = fdimf(1.0f, 2.0f);
        long double l1 = fmaxl(1.0L, 2.0L);
        long double l2 = fminl(1.0L, 2.0L);
        long double l3 = fdiml(1.0L, 2.0L);
        (void)d1; (void)d2; (void)d3;
        (void)f1; (void)f2; (void)f3;
        (void)l1; (void)l2; (void)l3;
    }

    printf("C99 7.12.12 positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「函数参数个数必须与原型一致」：
 * fmax 原型为 double fmax(double, double)，只传 1 个实参，
 * gcc -std=c99 应报错：too few arguments to function 'fmax'。 */
double bad1 = fmax(1.0);

/* 违反约束「函数参数个数必须与原型一致」：
 * fmin 传 3 个实参，gcc -std=c99 应报错：too many arguments。 */
double bad2 = fmin(1.0, 2.0, 3.0);

/* 违反约束「函数参数个数必须与原型一致」：
 * fdim 传 0 个实参，gcc -std=c99 应报错。 */
double bad3 = fdim();

/* 违反约束「实参类型必须可转换为形参类型」：
 * 传入结构体类型，无法转换为 double，gcc -std=c99 应报错。 */
struct S { int x; };
struct S s;
double bad4 = fmax(s, 1.0);

/* 违反约束「实参类型必须可转换为形参类型」：
 * 传入指针类型，无法隐式转换为 double，gcc -std=c99 应报错。 */
double bad5 = fmin((void *)0, 1.0);

/* 违反约束「函数调用结果不是左值，不能赋值」：
 * fmax 返回非左值，对其赋值应编译报错：lvalue required as left operand of assignment。 */
void bad6(void) { fmax(1.0, 2.0) = 3.0; }

/* 违反约束「函数调用结果不是左值，不能取地址」：
 * 对函数返回值取地址应编译报错：lvalue required as unary '&' operand。 */
void bad7(void) { double *p = &fmin(1.0, 2.0); (void)p; }

/* 违反约束「函数调用结果不是左值，不能自增」：
 * fdim 返回非左值，自增应编译报错。 */
void bad8(void) { fdim(1.0, 2.0)++; }

#endif