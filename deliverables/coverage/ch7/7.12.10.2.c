/*
 * 测试 C99 7.12.10.2 —— remainder / remainderf / remainderl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数声明（原型）与头文件 <math.h>
 *   [2] 语义：计算 x REM y（IEC 60559 定义）
 *   [3] 返回值：x REM y；y 为 0 时域错误或返回 0 由实现定义
 *   脚注 210：r = x - n*y，n 为 x/y 最近整数，|n - x/y| = 1/2 时取偶；
 *             r 总是精确的；r = 0 时符号与 x 相同。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* 用于比较浮点结果（remainder 结果总是精确的，可用 == 比较，
 * 但为稳妥起见仍用容差比较） */
static int feq(double a, double b)
{
    return fabs(a - b) <= 1e-12 * (fabs(a) + fabs(b) + 1.0);
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型存在性：取函数地址，验证三个函数均已声明 */
    {
        double (*pd)(double, double) = remainder;
        float  (*pf)(float, float)   = remainderf;
        long double (*pl)(long double, long double) = remainderl;
        assert(pd != NULL && pf != NULL && pl != NULL);
    }

    /* [2][3] 基本语义：x REM y = x - n*y，n 为 x/y 最近整数 */
    {
        /* 5 REM 3 = 5 - 2*3 = -1  (5/3 = 1.666..., 最近整数 2) */
        assert(feq(remainder(5.0, 3.0), -1.0));
        /* -5 REM 3 = -5 - (-2)*3 = 1  (-5/3 = -1.666..., 最近整数 -2) */
        assert(feq(remainder(-5.0, 3.0), 1.0));
        /* 5 REM -3 = 5 - (-2)*(-3) = -1 */
        assert(feq(remainder(5.0, -3.0), -1.0));
        /* -5 REM -3 = -5 - 2*(-3) = 1 */
        assert(feq(remainder(-5.0, -3.0), 1.0));
    }

    /* 脚注 210：|n - x/y| = 1/2 时 n 取偶（round-to-even） */
    {
        /* 3/2 = 1.5，最近整数取偶 -> 2，故 3 REM 2 = 3 - 2*2 = -1 */
        assert(feq(remainder(3.0, 2.0), -1.0));
        /* 5/2 = 2.5，最近整数取偶 -> 2，故 5 REM 2 = 5 - 2*2 = 1 */
        assert(feq(remainder(5.0, 2.0), 1.0));
        /* 1/2 = 0.5，最近整数取偶 -> 0，故 1 REM 2 = 1 - 0*2 = 1 */
        assert(feq(remainder(1.0, 2.0), 1.0));
        /* -1/2 = -0.5，最近整数取偶 -> 0，故 -1 REM 2 = -1 */
        assert(feq(remainder(-1.0, 2.0), -1.0));
    }

    /* 脚注 210：r = 0 时符号与 x 相同 */
    {
        double r1 = remainder(4.0, 2.0);   /* 4 REM 2 = 0，符号 + */
        double r2 = remainder(-4.0, 2.0);  /* -4 REM 2 = 0，符号 - */
        assert(r1 == 0.0);
        assert(r2 == 0.0);
        assert(signbit(r1) == 0);          /* +0 */
        assert(signbit(r2) != 0);          /* -0 */
    }

    /* 脚注 210：remainder 结果总是精确的（可精确表示） */
    {
        /* 大整数情形，结果应为精确整数 */
        double x = 1e15 + 1.0;
        double y = 1e15;
        double r = remainder(x, y);        /* x/y 接近 1，n = 1，r = 1 */
        assert(feq(r, 1.0));
    }

    /* [3] y 为 0：实现定义（域错误或返回 0），此处只验证不崩溃 */
    {
        double r = remainder(1.0, 0.0);
        /* 实现定义：可能是 NaN 或 0，不做强断言，仅确保调用成功 */
        (void)r;
    }

    /* [1][2][3] float 版本 remainderf */
    {
        float r = remainderf(5.0f, 3.0f);
        assert(feq((double)r, -1.0));
        r = remainderf(3.0f, 2.0f);
        assert(feq((double)r, -1.0));
    }

    /* [1][2][3] long double 版本 remainderl */
    {
        long double r = remainderl(5.0L, 3.0L);
        assert(feq((double)r, -1.0));
        r = remainderl(3.0L, 2.0L);
        assert(feq((double)r, -1.0));
    }

    /* [2] 与 fmod 的区别：remainder 使用最近整数，fmod 使用截断 */
    {
        /* fmod(5,3) = 2，remainder(5,3) = -1 */
        assert(feq(fmod(5.0, 3.0), 2.0));
        assert(feq(remainder(5.0, 3.0), -1.0));
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「函数参数类型必须与原型匹配」：
     * remainder 的原型为 double remainder(double, double)，
     * 传入结构体类型参数，gcc -std=c99 应报错。
     */
    struct S { int x; } s;
    remainder(s, 1.0);

    /*
     * 违反约束「实参个数必须与原型一致」：
     * remainder 需要 2 个参数，只传 1 个，应报错。
     */
    remainder(1.0);

    /*
     * 违反约束「实参个数必须与原型一致」：
     * remainder 需要 2 个参数，传 3 个，应报错。
     */
    remainder(1.0, 2.0, 3.0);

    /*
     * 违反约束「函数返回值不可作为左值赋值」：
     * remainder 返回非左值，对其赋值应报错。
     */
    remainder(1.0, 2.0) = 3.0;

    /*
     * 违反约束「取地址操作数必须是左值」：
     * 函数返回值不是左值，取地址应报错。
     */
    double *p = &remainder(1.0, 2.0);

    /*
     * 违反约束「remainderf 参数类型为 float」：
     * 传入结构体，应报错。
     */
    struct T { int y; } t;
    remainderf(t, 1.0f);

    /*
     * 违反约束「remainderl 参数类型为 long double」：
     * 传入结构体，应报错。
     */
    struct U { int z; } u;
    remainderl(u, 1.0L);
#endif

    return 0;
}