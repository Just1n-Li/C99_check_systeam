/*
 * 测试 C99 7.12.10.3 —— remquo / remquof / remquol 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明（double/float/long double 三个版本，第三个参数为 int *）
 *   [2] 语义：余数与 remainder 相同；*quo 的符号为 x/y 的符号；
 *       *quo 的绝对值与 x/y 的整数商的绝对值模 2^n 同余（n >= 3，实现定义）
 *   [3] 返回值：x REM y；y 为 0 时 *quo 未指定，是否域错误/返回零由实现定义
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* 辅助：判断两个 double 是否近似相等 */
static int close_enough(double a, double b)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d < 1e-9;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：三个函数都能被调用，返回类型分别为 double/float/long double */
    {
        int q = 0;
        double  rd = remquo(7.0, 3.0, &q);
        float   rf = remquof(7.0f, 3.0f, &q);
        long double rl = remquol(7.0L, 3.0L, &q);
        (void)rd; (void)rf; (void)rl;
        /* 三个函数都存在且可调用，编译通过即验证了 [1] 的原型 */
        assert(1);
    }

    /* [2] 余数与 remainder 相同：remquo(x,y,&q) 的返回值 == remainder(x,y) */
    {
        int q1 = 0, q2 = 0;
        double r1 = remquo(7.0, 3.0, &q1);
        double r2 = remainder(7.0, 3.0);
        (void)q2;
        assert(close_enough(r1, r2));

        r1 = remquo(-7.0, 3.0, &q1);
        r2 = remainder(-7.0, 3.0);
        assert(close_enough(r1, r2));

        r1 = remquo(7.0, -3.0, &q1);
        r2 = remainder(7.0, -3.0);
        assert(close_enough(r1, r2));

        r1 = remquo(-7.0, -3.0, &q1);
        r2 = remainder(-7.0, -3.0);
        assert(close_enough(r1, r2));
    }

    /* [2] *quo 的符号 == x/y 的符号 */
    {
        int q = 0;
        (void)remquo(7.0, 3.0, &q);      /* x/y > 0 -> q >= 0 */
        assert(q >= 0);

        (void)remquo(-7.0, 3.0, &q);     /* x/y < 0 -> q <= 0 */
        assert(q <= 0);

        (void)remquo(7.0, -3.0, &q);     /* x/y < 0 -> q <= 0 */
        assert(q <= 0);

        (void)remquo(-7.0, -3.0, &q);    /* x/y > 0 -> q >= 0 */
        assert(q >= 0);
    }

    /* [2] *quo 的绝对值与整数商的绝对值模 2^n 同余（n >= 3，实现定义）
     *     取 n = 3 时，|q| 应满足 |q| ≡ |trunc(x/y)| (mod 8)。
     *     这里用几个已知整数商来验证低 3 位一致。 */
    {
        int q = 0;
        /* 7/3 的整数商为 2，2 mod 8 == 2 */
        (void)remquo(7.0, 3.0, &q);
        assert((q < 0 ? -q : q) % 8 == 2 % 8);

        /* 17/5 的整数商为 3，3 mod 8 == 3 */
        (void)remquo(17.0, 5.0, &q);
        assert((q < 0 ? -q : q) % 8 == 3 % 8);

        /* 100/7 的整数商为 14，14 mod 8 == 6 */
        (void)remquo(100.0, 7.0, &q);
        assert((q < 0 ? -q : q) % 8 == 14 % 8);

        /* -100/7 的整数商为 -14，|q| mod 8 == 6，且符号为负 */
        (void)remquo(-100.0, 7.0, &q);
        assert(q <= 0);
        assert((q < 0 ? -q : q) % 8 == 14 % 8);
    }

    /* [2] 大整数商：验证模 2^n 同余（n >= 3），取 n = 3 检查低 3 位 */
    {
        int q = 0;
        /* 1000/3 的整数商为 333，333 mod 8 == 5 */
        (void)remquo(1000.0, 3.0, &q);
        assert((q < 0 ? -q : q) % 8 == 333 % 8);
    }

    /* [3] 返回值 x REM y：验证几个具体数值 */
    {
        int q = 0;
        double r = remquo(7.0, 3.0, &q);
        /* 7 = 2*3 + 1，余数 1 */
        assert(close_enough(r, 1.0));

        r = remquo(-7.0, 3.0, &q);
        /* -7 = -2*3 - 1，余数 -1 */
        assert(close_enough(r, -1.0));

        r = remquo(5.0, 2.0, &q);
        /* 5 = 2*2 + 1，余数 1 */
        assert(close_enough(r, 1.0));

        r = remquo(4.0, 2.0, &q);
        /* 4 = 2*2 + 0，余数 0 */
        assert(close_enough(r, 0.0));
    }

    /* [3] y 为 0 时：*quo 未指定，是否域错误/返回零由实现定义。
     *     这里只验证「调用不会导致编译错误」，运行行为不做断言（实现定义）。 */
    {
        int q = 0;
        volatile double zero = 0.0;
        double r = remquo(1.0, zero, &q);
        (void)r; (void)q;
        /* 实现定义：可能返回 NaN、可能触发域错误、可能返回 0，均符合标准 */
        assert(1);
    }

    /* [1] float 版本 remquof 的返回值类型为 float */
    {
        int q = 0;
        float rf = remquof(7.0f, 3.0f, &q);
        assert(rf == 1.0f);
        assert(q >= 0);
    }

    /* [1] long double 版本 remquol 的返回值类型为 long double */
    {
        int q = 0;
        long double rl = remquol(7.0L, 3.0L, &q);
        assert(rl == 1.0L);
        assert(q >= 0);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「remquo 的第三个参数类型必须为 int *」：
     * 传入 double * 应导致编译错误（参数类型不兼容）。
     * 期望：gcc -std=c99 报 "incompatible pointer type" 或类似错误。 */
    {
        double qd = 0.0;
        double r = remquo(7.0, 3.0, &qd);   /* 错误：应为 int *，实参为 double * */
        (void)r;
    }

    /* 违反约束「remquo 的第三个参数类型必须为 int *」：
     * 传入 int 值（而非指针）应导致编译错误。
     * 期望：gcc -std=c99 报 "passing argument ... makes pointer from integer" 错误。 */
    {
        int q = 0;
        double r = remquo(7.0, 3.0, q);     /* 错误：应为 int *，实参为 int */
        (void)r;
    }

    /* 违反约束「remquo 需要 3 个参数」：
     * 参数个数不足应导致编译错误。
     * 期望：gcc -std=c99 报 "too few arguments to function 'remquo'" 错误。 */
    {
        double r = remquo(7.0, 3.0);        /* 错误：缺少第三个参数 */
        (void)r;
    }

    /* 违反约束「remquo 需要 3 个参数」：
     * 参数个数过多应导致编译错误。
     * 期望：gcc -std=c99 报 "too many arguments to function 'remquo'" 错误。 */
    {
        int q = 0;
        double r = remquo(7.0, 3.0, &q, 1); /* 错误：多了一个参数 */
        (void)r;
    }

    /* 违反约束「remquo 的第一个参数必须为算术类型」：
     * 传入结构体应导致编译错误。
     * 期望：gcc -std=c99 报 "incompatible type for argument 1" 错误。 */
    {
        struct S { int x; } s;
        int q = 0;
        double r = remquo(s, 3.0, &q);      /* 错误：结构体不能作为算术参数 */
        (void)r;
    }

    /* 违反约束「remquo 的返回值不能作为左值被赋值」：
     * 函数调用结果不是左值，对其赋值应导致编译错误。
     * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment" 错误。 */
    {
        int q = 0;
        remquo(7.0, 3.0, &q) = 1.0;         /* 错误：函数返回值不是左值 */
    }

    /* 违反约束「remquof 的第三个参数类型必须为 int *」：
     * 传入 float * 应导致编译错误。
     * 期望：gcc -std=c99 报 "incompatible pointer type" 错误。 */
    {
        float qf = 0.0f;
        float r = remquof(7.0f, 3.0f, &qf); /* 错误：应为 int *，实参为 float * */
        (void)r;
    }

    /* 违反约束「remquol 的第三个参数类型必须为 int *」：
     * 传入 long double * 应导致编译错误。
     * 期望：gcc -std=c99 报 "incompatible pointer type" 错误。 */
    {
        long double ql = 0.0L;
        long double r = remquol(7.0L, 3.0L, &ql); /* 错误：应为 int *，实参为 long double * */
        (void)r;
    }

#endif /* 负向测试结束 */

    return 0;
}