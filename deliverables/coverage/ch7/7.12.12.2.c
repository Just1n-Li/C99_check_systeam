/*
 * 测试 C99 7.12.12.2 —— fmax / fmaxf / fmaxl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明（double fmax(double,double); float fmaxf(float,float);
 *       long double fmaxl(long double,long double);）—— 通过取函数指针类型验证。
 *   [2] 语义：确定参数的最大数值。
 *   [3] 返回值：返回参数的最大数值。
 *   脚注 213：若一个参数为 NaN、另一个为数值，则选择数值参数。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型完全一致。
 *     若原型不匹配（例如返回 float 或参数类型错误），此处赋值会编译报错。 */
static double (*p_fmax)(double, double) = fmax;
static float  (*p_fmaxf)(float, float)  = fmaxf;
static long double (*p_fmaxl)(long double, long double) = fmaxl;

int main(void)
{
    /* [1] 通过函数指针调用，确认原型可用 */
    assert(p_fmax(1.0, 2.0) == 2.0);
    assert(p_fmaxf(1.0f, 2.0f) == 2.0f);
    assert(p_fmaxl(1.0L, 2.0L) == 2.0L);

    /* [2][3] 基本语义：返回两个参数中的最大数值 */
    assert(fmax(3.0, 7.0) == 7.0);
    assert(fmax(7.0, 3.0) == 7.0);
    assert(fmax(-1.0, -5.0) == -1.0);
    assert(fmax(-5.0, -1.0) == -1.0);

    /* [2][3] 相等参数：返回该值 */
    assert(fmax(4.5, 4.5) == 4.5);

    /* [2][3] 含 0 与符号：数值比较，-0.0 与 +0.0 数值相等 */
    assert(fmax(0.0, -0.0) == 0.0);
    assert(fmax(-0.0, 0.0) == 0.0);

    /* [2][3] 极大/极小值 */
    assert(fmax(DBL_MAX, -DBL_MAX) == DBL_MAX);
    assert(fmax(-DBL_MAX, DBL_MAX) == DBL_MAX);

    /* [2][3] fmaxf 版本 */
    assert(fmaxf(1.5f, 2.5f) == 2.5f);
    assert(fmaxf(2.5f, 1.5f) == 2.5f);
    assert(fmaxf(-2.5f, -1.5f) == -1.5f);

    /* [2][3] fmaxl 版本 */
    assert(fmaxl(1.5L, 2.5L) == 2.5L);
    assert(fmaxl(2.5L, 1.5L) == 2.5L);
    assert(fmaxl(-2.5L, -1.5L) == -1.5L);

    /* 脚注 213：一个参数为 NaN，另一个为数值 —— 选择数值参数 */
    {
        double nan_val = NAN;
        assert(fmax(nan_val, 5.0) == 5.0);   /* NaN 在左 */
        assert(fmax(5.0, nan_val) == 5.0);   /* NaN 在右 */
        assert(fmax(nan_val, -3.0) == -3.0);
        assert(fmax(-3.0, nan_val) == -3.0);
    }
    {
        float nan_f = NAN;
        assert(fmaxf(nan_f, 5.0f) == 5.0f);
        assert(fmaxf(5.0f, nan_f) == 5.0f);
    }
    {
        long double nan_l = NAN;
        assert(fmaxl(nan_l, 5.0L) == 5.0L);
        assert(fmaxl(5.0L, nan_l) == 5.0L);
    }

    /* 脚注 213：两个参数均为 NaN —— 返回 NaN（用自比较检测） */
    {
        double r = fmax(NAN, NAN);
        assert(r != r);   /* NaN != NaN 为真 */
    }

    /* [2][3] 结果可赋给 double 变量并参与后续运算 */
    {
        double m = fmax(2.0, 9.0);
        assert(m == 9.0);
        assert(m + 1.0 == 10.0);
    }

    printf("C99 7.12.12.2 fmax/fmaxf/fmaxl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fmax 的参数必须为 double 类型（算术类型转换后）」：
 * 传入结构体，无法转换为 double，gcc -std=c99 应报错。 */
struct S { int x; } s1, s2;
double bad1 = fmax(s1, s2);

/* 违反约束「fmax 参数个数必须为 2」：
 * 只传一个参数，gcc -std=c99 应报错（too few arguments）。 */
double bad2 = fmax(1.0);

/* 违反约束「fmax 参数个数必须为 2」：
 * 传三个参数，gcc -std=c99 应报错（too many arguments）。 */
double bad3 = fmax(1.0, 2.0, 3.0);

/* 违反约束「fmaxf 的参数必须为 float 类型」：
 * 传入结构体，无法转换为 float，gcc -std=c99 应报错。 */
float bad4 = fmaxf(s1, s2);

/* 违反约束「fmaxl 的参数必须为 long double 类型」：
 * 传入结构体，无法转换为 long double，gcc -std=c99 应报错。 */
long double bad5 = fmaxl(s1, s2);

/* 违反约束「fmax 的返回值类型为 double，不能作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
fmax(1.0, 2.0) = 3.0;

/* 违反约束「fmaxf 的返回值类型为 float，不能作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
fmaxf(1.0f, 2.0f) = 3.0f;

/* 违反约束「fmaxl 的返回值类型为 long double，不能作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
fmaxl(1.0L, 2.0L) = 3.0L;

#endif