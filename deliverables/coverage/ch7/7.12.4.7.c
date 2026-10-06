/*
 * 测试目标：C99 7.12.4.7 —— tan 函数族 (tan / tanf / tanl)
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，tan/tanf/tanl 三个函数可被调用，
 *             返回 x（弧度）的正切值，类型分别为 double/float/long double。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 说明：本条款本身约束极少（仅要求 <math.h> 中声明这三个函数），
 *       负向测试主要针对「调用未声明函数」「参数个数/类型不匹配」等
 *       由函数原型约束推导出的编译期错误。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* 辅助：判断两个 double 是否近似相等 */
static int dbl_close(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：<math.h> 提供 double tan(double);
     *                float tanf(float);
     *                long double tanl(long double);
     *     验证三个函数均可被调用，且返回类型正确。 */
    {
        double      xd = 0.0;
        float       xf = 0.0f;
        long double xl = 0.0L;

        double      rd = tan(xd);
        float       rf = tanf(xf);
        long double rl = tanl(xl);

        /* 返回类型检查：赋值给对应类型不应产生截断警告（此处仅验证可编译） */
        assert(rd == 0.0);
        assert(rf == 0.0f);
        assert(rl == 0.0L);
    }

    /* [2] Description：返回 x（弧度）的正切值。
     *     验证几个已知点的正切值。 */
    {
        /* tan(0) = 0 */
        assert(dbl_close(tan(0.0), 0.0, 1e-15));

        /* tan(pi/4) = 1 */
        double pi = 3.14159265358979323846;
        assert(dbl_close(tan(pi / 4.0), 1.0, 1e-12));

        /* tan(-pi/4) = -1 （奇函数性质） */
        assert(dbl_close(tan(-pi / 4.0), -1.0, 1e-12));

        /* tan(pi) ≈ 0 */
        assert(dbl_close(tan(pi), 0.0, 1e-12));
    }

    /* [2] 弧度语义：tan 的参数以弧度计。
     *     验证 tan(pi/6) ≈ 1/sqrt(3) ≈ 0.5773502691896258 */
    {
        double pi = 3.14159265358979323846;
        double expected = 1.0 / sqrt(3.0);
        assert(dbl_close(tan(pi / 6.0), expected, 1e-12));
    }

    /* [3] Returns：tanf 与 tanl 的返回值应与 tan 在相同输入下一致（精度范围内）。 */
    {
        double pi = 3.14159265358979323846;
        double x  = pi / 3.0;   /* tan(pi/3) = sqrt(3) */

        double      rd = tan(x);
        float       rf = tanf((float)x);
        long double rl = tanl((long double)x);

        double expected = sqrt(3.0);

        assert(dbl_close(rd, expected, 1e-12));
        assert(dbl_close((double)rf, expected, 1e-5));   /* float 精度较低 */
        assert(dbl_close((double)rl, expected, 1e-12));
    }

    /* [3] 奇函数性质：tan(-x) == -tan(x) */
    {
        double x = 0.7;
        assert(dbl_close(tan(-x), -tan(x), 1e-15));
    }

    /* [3] 周期性：tan(x + pi) == tan(x) */
    {
        double pi = 3.14159265358979323846;
        double x  = 0.3;
        assert(dbl_close(tan(x + pi), tan(x), 1e-12));
    }

    /* [1] 函数指针类型检查：验证原型声明的返回/参数类型。 */
    {
        double      (*pd)(double)            = tan;
        float       (*pf)(float)             = tanf;
        long double (*pl)(long double)       = tanl;

        assert(dbl_close(pd(0.0), 0.0, 1e-15));
        assert(pf(0.0f) == 0.0f);
        assert(pl(0.0L) == 0.0L);
    }

    printf("All positive tests for C99 7.12.4.7 (tan functions) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「函数调用参数个数必须与原型一致」：
     * tan 原型为 double tan(double)，调用时传 0 个参数，
     * gcc -std=c99 应报错：too few arguments to function 'tan'。 */
    tan();

    /* 违反约束「函数调用参数个数必须与原型一致」：
     * 传 2 个参数，gcc -std=c99 应报错：too many arguments to function 'tan'。 */
    tan(1.0, 2.0);

    /* 违反约束「函数调用参数类型必须可转换为原型参数类型」：
     * 传入结构体，无法隐式转换为 double，
     * gcc -std=c99 应报错：incompatible type for argument 1 of 'tan'。 */
    struct S { int x; } s;
    tan(s);

    /* 违反约束「函数调用参数类型必须可转换为原型参数类型」：
     * 传入指针，无法隐式转换为 double，
     * gcc -std=c99 应报错：incompatible type for argument 1 of 'tan'。 */
    int *p = 0;
    tan(p);

    /* 违反约束「函数返回值不可作为左值赋值」：
     * tan 返回非左值，对其赋值应报错：
     * gcc -std=c99 应报错：lvalue required as left operand of assignment。 */
    tan(1.0) = 2.0;

    /* 违反约束「函数返回值不可取地址」：
     * tan 返回非左值，&tan(1.0) 应报错：
     * gcc -std=c99 应报错：lvalue required as unary '&' operand。 */
    double *q = &tan(1.0);

    /* 违反约束「函数名不可作为赋值目标」：
     * tan 是函数指示符，不可赋值，
     * gcc -std=c99 应报错：lvalue required as left operand of assignment。 */
    tan = 0;

    /* 违反约束「tanf 参数类型为 float，传入结构体不可转换」：
     * gcc -std=c99 应报错：incompatible type for argument 1 of 'tanf'。 */
    tanf(s);

    /* 违反约束「tanl 参数类型为 long double，传入结构体不可转换」：
     * gcc -std=c99 应报错：incompatible type for argument 1 of 'tanl'。 */
    tanl(s);

#endif

    return 0;
}