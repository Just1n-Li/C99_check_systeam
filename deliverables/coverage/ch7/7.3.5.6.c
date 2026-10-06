/*
 * 测试 C99 7.3.5.6 —— ctan / ctanf / ctanl 复数正切函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 ctan/ctanf/ctanl，验证返回复数正切值，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应被编译器拒绝。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数声明、参数类型、返回类型
 *   [2] Description：计算复数 z 的正切
 *   [3] Returns：返回复数正切值
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：验证三个函数可被调用，参数/返回类型符合声明 */
    {
        double complex        zd = 0.5 + 0.25 * I;
        float complex         zf = 0.5f + 0.25f * I;
        long double complex   zl = 0.5L + 0.25L * I;

        double complex        rd = ctan(zd);
        float complex         rf = ctanf(zf);
        long double complex   rl = ctanl(zl);

        /* 仅验证调用成功、结果有限（非 NaN/Inf） */
        assert(isfinite(creal(rd)) && isfinite(cimag(rd)));
        assert(isfinite(crealf(rf)) && isfinite(cimagf(rf)));
        assert(isfinite(creall(rl)) && isfinite(cimagl(rl)));
    }

    /* [2][3] Description/Returns：ctan(z) = sin(z)/cos(z)，验证数值正确性 */
    {
        double complex z = 0.3 + 0.4 * I;
        double complex r = ctan(z);

        /* 用 sin/cos 计算参考值 */
        double complex ref = csin(z) / ccos(z);

        double eps = 1e-12;
        assert(fabs(creal(r) - creal(ref)) < eps);
        assert(fabs(cimag(r) - cimag(ref)) < eps);
    }

    /* [2][3] 实数轴上的特例：ctan(x + 0i) 应等于 tan(x) */
    {
        double x = 0.7;
        double complex z = x + 0.0 * I;
        double complex r = ctan(z);

        double eps = 1e-12;
        assert(fabs(creal(r) - tan(x)) < eps);
        assert(fabs(cimag(r) - 0.0) < eps);
    }

    /* [2][3] 纯虚数轴：ctan(0 + yi) = i * tanh(y) */
    {
        double y = 0.6;
        double complex z = 0.0 + y * I;
        double complex r = ctan(z);

        double eps = 1e-12;
        assert(fabs(creal(r) - 0.0) < eps);
        assert(fabs(cimag(r) - tanh(y)) < eps);
    }

    /* [2][3] 奇偶性：ctan(-z) = -ctan(z) */
    {
        double complex z = 0.2 + 0.3 * I;
        double complex a = ctan(-z);
        double complex b = -ctan(z);

        double eps = 1e-12;
        assert(fabs(creal(a) - creal(b)) < eps);
        assert(fabs(cimag(a) - cimag(b)) < eps);
    }

    /* [1] 返回类型为复数：结果可参与复数运算 */
    {
        double complex z = 0.1 + 0.2 * I;
        double complex r = ctan(z);
        double complex sq = r * r;   /* 复数乘法，验证返回类型确为复数 */
        assert(isfinite(creal(sq)) && isfinite(cimag(sq)));
    }

    printf("All positive tests for C99 7.3.5.6 (ctan/ctanf/ctanl) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数参数类型必须为 double complex」：
 * 传入结构体类型，gcc -std=c99 应报错（incompatible type for argument）。 */
struct NotComplex { double re, im; } nc;
double complex bad1 = ctan(nc);

/* 违反约束「函数参数个数必须为 1」：
 * 传入两个实参，gcc -std=c99 应报错（too many arguments to function）。 */
double complex bad2 = ctan(1.0 + 0.0 * I, 2.0 + 0.0 * I);

/* 违反约束「函数参数个数必须为 1」：
 * 不传实参，gcc -std=c99 应报错（too few arguments to function）。 */
double complex bad3 = ctan();

/* 违反约束「函数必须已声明」：
 * 未包含 <complex.h> 且未声明 ctan 时调用，gcc -std=c99 应报错
 * （implicit declaration of function 'ctan'）。 */
double complex bad4 = ctan(1.0 + 0.0 * I);

/* 违反约束「返回值类型为 double complex，不能赋给不兼容的标量类型」：
 * 将复数结果赋给 double 标量，gcc -std=c99 应报错
 * （incompatible types when initializing type 'double' using type 'double complex'）。 */
double bad5 = ctan(1.0 + 0.0 * I);

/* 违反约束「ctanf 参数类型必须为 float complex」：
 * 传入 double complex，gcc -std=c99 应报错（incompatible type for argument）。 */
float complex bad6 = ctanf(1.0 + 0.0 * I);

/* 违反约束「ctanl 参数类型必须为 long double complex」：
 * 传入 double complex，gcc -std=c99 应报错（incompatible type for argument）。 */
long double complex bad7 = ctanl(1.0 + 0.0 * I);

#endif