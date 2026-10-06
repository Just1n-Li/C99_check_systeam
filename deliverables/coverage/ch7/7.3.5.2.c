/*
 * 测试 C99 条款 7.3.5.2 —— casin 函数族
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 覆盖段落：
 *   [1] 声明与原型：casin / casinf / casinl，参数与返回类型。
 *   [2] 语义：计算复数反正弦，分支割线位于实轴 [-1,+1] 之外。
 *   [3] 返回值：实部落在 [-pi/2, +pi/2]，虚部无界。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

/* 用于比较浮点数的容差 */
#define EPS 1e-9

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：三种类型的函数都能被调用，返回类型正确 */
    {
        double complex        z  = 0.5 + 0.5 * I;
        float complex         zf = 0.5f + 0.5f * I;
        long double complex   zl = 0.5L + 0.5L * I;

        double complex        rd = casin(z);
        float complex         rf = casinf(zf);
        long double complex   rl = casinl(zl);

        /* 三种精度结果应彼此接近 */
        assert(fabs(creal(rd) - (double)crealf(rf)) < 1e-5);
        assert(fabs(creal(rd) - (double)creall(rl)) < 1e-5);
        assert(fabs(cimag(rd) - (double)cimagf(rf)) < 1e-5);
        assert(fabs(cimag(rd) - (double)cimagl(rl)) < 1e-5);
    }

    /* [2][3] 基本恒等式：sin(casin(z)) == z （在分支割线之外） */
    {
        double complex z = 0.3 + 0.4 * I;
        double complex w = casin(z);
        double complex back = csin(w);
        assert(fabs(creal(back) - creal(z)) < EPS);
        assert(fabs(cimag(back) - cimag(z)) < EPS);
    }

    /* [3] 实部范围：对任意 z，creal(casin(z)) 应落在 [-pi/2, +pi/2] */
    {
        double complex samples[] = {
            0.0 + 0.0 * I,
            0.5 + 0.0 * I,
           -0.5 + 0.0 * I,
            0.0 + 1.0 * I,
            0.0 - 1.0 * I,
            2.0 + 3.0 * I,
           -2.0 - 3.0 * I,
            1.0 + 1.0 * I,
           -1.0 + 1.0 * I
        };
        int n = (int)(sizeof(samples) / sizeof(samples[0]));
        int i;
        for (i = 0; i < n; ++i) {
            double complex w = casin(samples[i]);
            double re = creal(w);
            assert(re >= -M_PI / 2.0 - EPS);
            assert(re <=  M_PI / 2.0 + EPS);
        }
    }

    /* [2] 分支割线之外：实轴上的点 |x| > 1 时，结果应为纯虚数（实部为 0） */
    {
        double complex z = 2.0 + 0.0 * I;
        double complex w = casin(z);
        assert(fabs(creal(w)) < EPS);
        /* asin(2) = pi/2 - i*ln(2+sqrt(3))，虚部为负 */
        assert(cimag(w) < 0.0);
    }

    /* [2] 实轴上 |x| <= 1 时，结果应为实数（虚部为 0） */
    {
        double complex z = 0.5 + 0.0 * I;
        double complex w = casin(z);
        assert(fabs(cimag(w)) < EPS);
        assert(fabs(creal(w) - asin(0.5)) < EPS);
    }

    /* [3] 特殊值：casin(0) == 0 */
    {
        double complex w = casin(0.0 + 0.0 * I);
        assert(fabs(creal(w)) < EPS);
        assert(fabs(cimag(w)) < EPS);
    }

    /* [3] 特殊值：casin(1) == pi/2 （实部），虚部为 0 */
    {
        double complex w = casin(1.0 + 0.0 * I);
        assert(fabs(creal(w) - M_PI / 2.0) < EPS);
        assert(fabs(cimag(w)) < EPS);
    }

    /* [3] 特殊值：casin(-1) == -pi/2 （实部），虚部为 0 */
    {
        double complex w = casin(-1.0 + 0.0 * I);
        assert(fabs(creal(w) + M_PI / 2.0) < EPS);
        assert(fabs(cimag(w)) < EPS);
    }

    /* [2] 奇函数性质：casin(-z) == -casin(z) */
    {
        double complex z = 0.7 + 0.2 * I;
        double complex a = casin(-z);
        double complex b = casin(z);
        assert(fabs(creal(a) + creal(b)) < EPS);
        assert(fabs(cimag(a) + cimag(b)) < EPS);
    }

    /* [1] 参数类型为 double complex，可接受隐式转换的实参 */
    {
        double complex w = casin(0.25);   /* 实数隐式转换为复数 */
        assert(fabs(creal(w) - asin(0.25)) < EPS);
        assert(fabs(cimag(w)) < EPS);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「函数调用实参个数必须与原型一致」：
       casin 原型只接受 1 个参数，传 2 个应报错。
       期望：gcc -std=c99 报 "too many arguments to function 'casin'" */
    {
        double complex z = 0.0 + 0.0 * I;
        double complex w = casin(z, z);
        (void)w;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
       casin 原型需要 1 个参数，传 0 个应报错。
       期望：gcc -std=c99 报 "too few arguments to function 'casin'" */
    {
        double complex w = casin();
        (void)w;
    }

    /* 违反约束「赋值运算符左操作数必须是可修改左值」：
       casin 的返回值是右值，不能赋值。
       期望：gcc -std=c99 报 "lvalue required as left operand of assignment" */
    {
        double complex z = 0.0 + 0.0 * I;
        casin(z) = z;
    }

    /* 违反约束「取地址运算符 & 的操作数必须是左值或函数指示符」：
       casin 的返回值是右值，不能取地址。
       期望：gcc -std=c99 报 "lvalue required as unary '&' operand" */
    {
        double complex z = 0.0 + 0.0 * I;
        double complex *p = &casin(z);
        (void)p;
    }

    /* 违反约束「函数指示符不能作为赋值目标」：
       试图给函数名 casin 赋值。
       期望：gcc -std=c99 报 "lvalue required as left operand of assignment" */
    {
        casin = 0;
    }

    /* 违反约束「函数指示符不能自增/自减」：
       期望：gcc -std=c99 报 "lvalue required as increment operand" */
    {
        casin++;
    }

#endif /* 负向测试结束 */

    return 0;
}