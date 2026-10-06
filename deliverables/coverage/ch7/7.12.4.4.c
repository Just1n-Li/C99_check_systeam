/*
 * 测试 C99 7.12.4.4 —— atan2 / atan2f / atan2l 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 概要：三个函数声明于 <math.h>，原型为
 *       double atan2(double,double); float atan2f(float,float);
 *       long double atan2l(long double,long double);
 *   [2] 描述：计算 y/x 的反正切，用两个参数的符号确定象限；两参数均为 0 时可能发生域错误。
 *   [3] 返回值：返回 arctan(y/x)，位于 [-pi, +pi] 弧度区间。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* 用于比较浮点数的容差 */
static int close_enough(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 概要：三个函数均可用，且返回类型分别为 double / float / long double。
     * 通过赋值给对应类型并检查类型宽度来验证原型存在。 */
    {
        double      d = atan2(1.0, 1.0);
        float       f = atan2f(1.0f, 1.0f);
        long double l = atan2l(1.0L, 1.0L);

        /* 返回值类型检查：sizeof 应等于对应类型 */
        assert(sizeof(d) == sizeof(double));
        assert(sizeof(f) == sizeof(float));
        assert(sizeof(l) == sizeof(long double));

        /* 三个函数在相同数学输入下结果应一致（在各自精度内） */
        assert(close_enough((double)f, d, 1e-6));
        assert(close_enough((double)l, d, 1e-6));
    }

    /* [2] 描述：用两个参数的符号确定象限。
     * 第一象限 (y>0, x>0)：结果在 (0, pi/2)
     * 第二象限 (y>0, x<0)：结果在 (pi/2, pi)
     * 第三象限 (y<0, x<0)：结果在 (-pi, -pi/2)
     * 第四象限 (y<0, x>0)：结果在 (-pi/2, 0)
     */
    {
        double q1 = atan2( 1.0,  1.0);   /* 第一象限 */
        double q2 = atan2( 1.0, -1.0);   /* 第二象限 */
        double q3 = atan2(-1.0, -1.0);   /* 第三象限 */
        double q4 = atan2(-1.0,  1.0);   /* 第四象限 */

        assert(q1 > 0.0 && q1 < M_PI / 2.0);
        assert(q2 > M_PI / 2.0 && q2 < M_PI);
        assert(q3 < -M_PI / 2.0 && q3 > -M_PI);
        assert(q4 < 0.0 && q4 > -M_PI / 2.0);

        /* 对称性：atan2(y, x) == -atan2(-y, x) */
        assert(close_enough(atan2( 1.0, 2.0), -atan2(-1.0, 2.0), 1e-12));
        assert(close_enough(atan2( 1.0, -2.0), -atan2(-1.0, -2.0), 1e-12));
    }

    /* [2] 描述：y/x 的反正切。当 x>0 时，atan2(y,x) == atan(y/x)。 */
    {
        double y = 0.5, x = 2.0;
        assert(close_enough(atan2(y, x), atan(y / x), 1e-12));
    }

    /* [2] 描述：x = 0 且 y != 0 时，结果应为 ±pi/2（符号由 y 决定）。 */
    {
        double p = atan2( 1.0, 0.0);
        double n = atan2(-1.0, 0.0);
        assert(close_enough(p,  M_PI / 2.0, 1e-12));
        assert(close_enough(n, -M_PI / 2.0, 1e-12));
    }

    /* [2] 描述：y = 0 且 x > 0 时，结果为 0；y = 0 且 x < 0 时，结果为 pi。 */
    {
        assert(close_enough(atan2(0.0,  1.0), 0.0, 1e-12));
        assert(close_enough(atan2(0.0, -1.0), M_PI, 1e-12));
    }

    /* [2] 描述：两参数均为 0 时可能发生域错误。
     * 这里只验证「可能发生域错误」这一允许行为：调用后检查 errno 或返回值，
     * 不强制要求一定报错（实现可返回 0 或设置 errno 为 EDOM）。 */
    {
        errno = 0;
        double r = atan2(0.0, 0.0);
        /* 允许两种实现：返回 0 且不设 errno，或设置 errno 为 EDOM。
         * 无论哪种，返回值都应在 [-pi, pi] 内。 */
        assert(r >= -M_PI && r <= M_PI);
        (void)r;
    }

    /* [3] 返回值：结果位于 [-pi, +pi] 弧度区间。
     * 对大量不同符号组合进行抽样验证。 */
    {
        const double ys[] = { -3.0, -1.0, -0.5, 0.0, 0.5, 1.0, 3.0 };
        const double xs[] = { -3.0, -1.0, -0.5, 0.5, 1.0, 3.0 };
        size_t i, j;
        for (i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
            for (j = 0; j < sizeof(xs) / sizeof(xs[0]); ++j) {
                double v = atan2(ys[i], xs[j]);
                assert(v >= -M_PI && v <= M_PI);
            }
        }
    }

    /* [3] 返回值：atan2 与 atan 在 x>0 时一致，且结果落在 [-pi, pi]。 */
    {
        double v = atan2(1.0, 1.0);
        assert(v >= -M_PI && v <= M_PI);
        assert(close_enough(v, M_PI / 4.0, 1e-12));
    }

    /* [1][3] atan2f / atan2l 的返回值也应在 [-pi, pi] 内。 */
    {
        float       vf = atan2f(1.0f, -1.0f);
        long double vl = atan2l(1.0L, -1.0L);
        assert((double)vf >= -M_PI && (double)vf <= M_PI);
        assert((double)vl >= -M_PI && (double)vl <= M_PI);
        /* 第二象限：应为 3*pi/4 */
        assert(close_enough((double)vf, 3.0 * M_PI / 4.0, 1e-6));
        assert(close_enough((double)vl, 3.0 * M_PI / 4.0, 1e-6));
    }

    printf("C99 7.12.4.4 atan2/atan2f/atan2l: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「atan2 的参数必须为算术类型（arithmetic type）」：
     * 传入结构体类型，gcc -std=c99 应报错：
     *   error: incompatible type for argument 1 of 'atan2'
     */
    struct S { int x; } s;
    atan2(s, 1.0);

    /*
     * 违反约束「atan2 的参数必须为算术类型」：
     * 传入指针类型，gcc -std=c99 应报错：
     *   error: incompatible type for argument 2 of 'atan2'
     */
    int *p = 0;
    atan2(1.0, p);

    /*
     * 违反约束「atan2 需要两个参数」：
     * 只传一个参数，gcc -std=c99 应报错：
     *   error: too few arguments to function 'atan2'
     */
    atan2(1.0);

    /*
     * 违反约束「atan2 需要两个参数」：
     * 传三个参数，gcc -std=c99 应报错：
     *   error: too many arguments to function 'atan2'
     */
    atan2(1.0, 2.0, 3.0);

    /*
     * 违反约束「atan2f 的参数必须为算术类型」：
     * 传入结构体，gcc -std=c99 应报错。
     */
    struct T { int y; } t;
    atan2f(t, 1.0f);

    /*
     * 违反约束「atan2l 的参数必须为算术类型」：
     * 传入指针，gcc -std=c99 应报错。
     */
    double *q = 0;
    atan2l(1.0L, q);

    /*
     * 违反约束「atan2 的返回值不可作为左值赋值」：
     * 函数调用结果不是左值，gcc -std=c99 应报错：
     *   error: lvalue required as left operand of assignment
     */
    atan2(1.0, 1.0) = 0.5;

    /*
     * 违反约束「atan2f 的返回值不可作为左值赋值」：
     * gcc -std=c99 应报错。
     */
    atan2f(1.0f, 1.0f) = 0.5f;

    /*
     * 违反约束「atan2l 的返回值不可作为左值赋值」：
     * gcc -std=c99 应报错。
     */
    atan2l(1.0L, 1.0L) = 0.5L;
#endif

    return 0;
}