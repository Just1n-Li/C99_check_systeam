/*
 * 测试 C99 7.3.9.1 —— carg / cargf / cargl 函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 carg/cargf/cargl，验证返回值为
 *             z 的辐角（相位角），落在 [-pi, +pi] 区间内，且分支切割
 *             沿负实轴。
 *   负向测试：违反约束的代码应导致编译报错（见 #if 0 块）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：三个函数均可用，返回类型分别为 double/float/long double */
    double complex       zd = 1.0 + 1.0 * I;
    float  complex       zf = 1.0f + 1.0f * I;
    long double complex  zl = 1.0L + 1.0L * I;

    double       ad = carg(zd);
    float        af = cargf(zf);
    long double  al = cargl(zl);

    /* [2][3] 第一象限：arg(1+i) = pi/4 */
    assert(fabs(ad - M_PI / 4.0) < 1e-12);
    assert(fabs((double)af - M_PI / 4.0) < 1e-6);
    assert(fabsl(al - (long double)(M_PI / 4.0)) < 1e-15L);

    /* [3] 正实轴：arg(1) = 0 */
    assert(carg(1.0 + 0.0 * I) == 0.0);

    /* [3] 正虚轴：arg(i) = pi/2 */
    assert(fabs(carg(0.0 + 1.0 * I) - M_PI / 2.0) < 1e-12);

    /* [3] 负虚轴：arg(-i) = -pi/2 */
    assert(fabs(carg(0.0 - 1.0 * I) + M_PI / 2.0) < 1e-12);

    /* [2] 分支切割沿负实轴：从上方趋近负实轴 -> +pi */
    assert(fabs(carg(-1.0 + 1e-300 * I) - M_PI) < 1e-12);

    /* [2] 分支切割沿负实轴：从下方趋近负实轴 -> -pi */
    assert(fabs(carg(-1.0 - 1e-300 * I) + M_PI) < 1e-12);

    /* [3] 返回值必须落在 [-pi, +pi] 区间内（遍历若干点验证） */
    {
        double re, im;
        for (re = -2.0; re <= 2.0; re += 0.5) {
            for (im = -2.0; im <= 2.0; im += 0.5) {
                double a = carg(re + im * I);
                assert(a >= -M_PI - 1e-12 && a <= M_PI + 1e-12);
            }
        }
    }

    /* [3] 与 atan2 的一致性（非分支切割处） */
    assert(fabs(carg(3.0 + 4.0 * I) - atan2(4.0, 3.0)) < 1e-12);
    assert(fabs(carg(-3.0 + 4.0 * I) - atan2(4.0, -3.0)) < 1e-12);
    assert(fabs(carg(-3.0 - 4.0 * I) - atan2(-4.0, -3.0)) < 1e-12);
    assert(fabs(carg(3.0 - 4.0 * I) - atan2(-4.0, 3.0)) < 1e-12);

    /* [1] 函数可被取地址，说明是真正的函数而非宏 */
    {
        double (*pd)(double complex) = carg;
        float  (*pf)(float complex)  = cargf;
        long double (*pl)(long double complex) = cargl;
        assert(fabs(pd(zd) - ad) < 1e-12);
        assert(fabs((double)pf(zf) - (double)af) < 1e-6);
        assert(fabsl(pl(zl) - al) < 1e-15L);
    }

    printf("carg  = %.15f\n", ad);
    printf("cargf = %.15f\n", (double)af);
    printf("cargl = %.15Lf\n", al);
    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「实参类型必须与原型参数类型兼容」：
     * carg 的参数是 double complex，传入 struct 类型不兼容，
     * gcc -std=c99 应报错（incompatible type for argument 1）。 */
    struct S { int x; } s;
    carg(s);

    /* 违反约束「实参个数必须与原型一致」：
     * carg 只接受 1 个参数，传 2 个应报错（too many arguments）。 */
    carg(1.0 + 1.0 * I, 2.0);

    /* 违反约束「实参个数必须与原型一致」：
     * cargf 需要 1 个参数，传 0 个应报错（too few arguments）。 */
    cargf();

    /* 违反约束「函数必须已声明」：
     * 未包含 <complex.h> 且未声明 carg 时调用，C99 下隐式声明
     * 返回 int，与 double 赋值不兼容；若开启 -Werror=implicit-function-declaration
     * 应报错。此处演示对未声明标识符的调用。 */
    /* carg_undeclared(1.0 + 1.0 * I); */

    /* 违反约束「赋值目标必须是可修改左值」：
     * carg 的返回值是右值，不能赋值，应报错（lvalue required）。 */
    carg(1.0 + 1.0 * I) = 0.0;

    /* 违反约束「取地址操作数必须是左值/函数」：
     * 对函数调用结果取地址应报错（lvalue required as unary '&' operand）。 */
    double *p = &carg(1.0 + 1.0 * I);

#endif

    return 0;
}