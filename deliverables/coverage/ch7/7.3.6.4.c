/*
 * 测试条款：C99 7.3.6.4 —— The ccosh functions
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 ccosh / ccoshf / ccoshl，
 *             验证其计算复数双曲余弦 cosh(z) = (e^z + e^-z)/2 的语义，
 *             并检查返回类型为对应的复数类型。程序应能编译并运行通过。
 *   负向测试：违反约束的代码片段（如参数类型错误、缺少头文件声明、
 *             对返回的非左值赋值等），期望编译器报错。
 *             这些片段放在 #if 0 中，保证本文件仍可正常编译运行。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型与 <complex.h>
 *   [2] Description：计算复数双曲余弦
 *   [3] Returns：返回复数双曲余弦值
 */

#include <complex.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

/* 辅助：比较两个 double complex 是否近似相等 */
static int dc_eq(double complex a, double complex b, double eps)
{
    return cabs(a - b) <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：验证三个函数原型可用，且返回类型为对应复数类型。
     * 通过赋值给正确类型的变量来静态验证返回类型。 */
    {
        double complex        z  = 1.0 + 2.0 * I;
        float complex         zf = 1.0f + 2.0f * I;
        long double complex   zl = 1.0L + 2.0L * I;

        double complex        r  = ccosh(z);    /* [1] double complex ccosh(double complex) */
        float complex         rf = ccoshf(zf);  /* [1] float complex ccoshf(float complex) */
        long double complex   rl = ccoshl(zl);  /* [1] long double complex ccoshl(long double complex) */

        /* 使用返回值，避免未使用警告 */
        assert(cabs(r)  >= 0.0);
        assert(cabsf(rf) >= 0.0f);
        assert(cabsl(rl) >= 0.0L);
    }

    /* [2][3] 语义：ccosh(0) = cosh(0) = 1，虚部为 0 */
    {
        double complex z = 0.0 + 0.0 * I;
        double complex r = ccosh(z);
        assert(dc_eq(r, 1.0 + 0.0 * I, 1e-12));
    }

    /* [2][3] 语义：ccosh 为偶函数，ccosh(-z) == ccosh(z) */
    {
        double complex z  = 0.7 - 1.3 * I;
        double complex r1 = ccosh(z);
        double complex r2 = ccosh(-z);
        assert(dc_eq(r1, r2, 1e-12));
    }

    /* [2][3] 语义：纯虚数输入 z = i*y 时，cosh(i*y) = cos(y)（实部），虚部为 0 */
    {
        double y = 0.9;
        double complex z = 0.0 + y * I;
        double complex r = ccosh(z);
        assert(dc_eq(r, cos(y) + 0.0 * I, 1e-12));
    }

    /* [2][3] 语义：与定义 cosh(z) = (e^z + e^-z)/2 一致 */
    {
        double complex z = 0.3 + 0.4 * I;
        double complex expected = (cexp(z) + cexp(-z)) / 2.0;
        double complex r = ccosh(z);
        assert(dc_eq(r, expected, 1e-12));
    }

    /* [2][3] 语义：ccoshf 与 ccosh 在 float 精度下一致 */
    {
        float complex zf = 0.5f + 0.25f * I;
        float complex rf = ccoshf(zf);
        double complex zd = 0.5 + 0.25 * I;
        double complex rd = ccosh(zd);
        assert(cabsf(rf - (float complex)rd) <= 1e-5f);
    }

    /* [2][3] 语义：ccoshl 与 ccosh 在 long double 精度下一致 */
    {
        long double complex zl = 0.5L + 0.25L * I;
        long double complex rl = ccoshl(zl);
        double complex zd = 0.5 + 0.25 * I;
        double complex rd = ccosh(zd);
        assert(cabsl(rl - (long double complex)rd) <= 1e-12L);
    }

    /* [2][3] 语义：cosh 的加法公式 cosh(a+b) = cosh(a)cosh(b) + sinh(a)sinh(b) */
    {
        double complex a = 0.2 + 0.1 * I;
        double complex b = 0.3 - 0.4 * I;
        double complex lhs = ccosh(a + b);
        double complex rhs = ccosh(a) * ccosh(b) + csinh(a) * csinh(b);
        assert(dc_eq(lhs, rhs, 1e-12));
    }

    printf("All positive tests for C99 7.3.6.4 (ccosh) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「实参类型必须与原型参数类型兼容」：
     * ccosh 的参数为 double complex，传入 int 无法隐式转换为复数类型，
     * gcc -std=c99 应报错（incompatible type for argument）。 */
    {
        int i = 1;
        double complex r = ccosh(i);   /* 期望：编译错误 */
        (void)r;
    }

    /* 违反约束「实参类型必须与原型参数类型兼容」：
     * ccoshf 的参数为 float complex，传入 double complex 不兼容，
     * gcc -std=c99 应报错。 */
    {
        double complex z = 1.0 + 1.0 * I;
        float complex r = ccoshf(z);   /* 期望：编译错误 */
        (void)r;
    }

    /* 违反约束「实参类型必须与原型参数类型兼容」：
     * ccoshl 的参数为 long double complex，传入 double complex 不兼容，
     * gcc -std=c99 应报错。 */
    {
        double complex z = 1.0 + 1.0 * I;
        long double complex r = ccoshl(z);  /* 期望：编译错误 */
        (void)r;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * ccosh 原型只接受 1 个参数，传 2 个应报错。 */
    {
        double complex z = 1.0 + 1.0 * I;
        double complex r = ccosh(z, z);   /* 期望：编译错误 */
        (void)r;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * ccosh 原型需要 1 个参数，传 0 个应报错。 */
    {
        double complex r = ccosh();   /* 期望：编译错误 */
        (void)r;
    }

    /* 违反约束「赋值运算符左操作数必须是可修改左值」：
     * 函数调用结果不是左值，不能赋值。 */
    {
        double complex z = 1.0 + 1.0 * I;
        ccosh(z) = z;   /* 期望：编译错误（lvalue required as left operand of assignment） */
    }

    /* 违反约束「赋值运算符左操作数必须是可修改左值」：
     * 对函数返回的复数取实部后赋值，实部不是左值。 */
    {
        double complex z = 1.0 + 1.0 * I;
        creal(ccosh(z)) = 0.0;   /* 期望：编译错误 */
    }

    /* 违反约束「赋值运算符左操作数必须是可修改左值」：
     * 强制转换的结果不是左值，不能赋值。 */
    {
        double complex z = 1.0 + 1.0 * I;
        (double complex)ccosh(z) = z;   /* 期望：编译错误 */
    }

    /* 违反约束「赋值运算符左操作数必须是可修改左值」：
     * 条件表达式结果不是左值，不能赋值。 */
    {
        double complex z = 1.0 + 1.0 * I;
        (1 ? ccosh(z) : ccosh(z)) = z;   /* 期望：编译错误 */
    }

    /* 违反约束「赋值运算符左操作数必须是可修改左值」：
     * 逗号表达式结果不是左值，不能赋值。 */
    {
        double complex z = 1.0 + 1.0 * I;
        (ccosh(z), ccosh(z)) = z;   /* 期望：编译错误 */
    }

    /* 违反约束「const 限定对象不可修改」：
     * 对 const 复数调用 ccosh 后，其成员/实部不可赋值。 */
    {
        const double complex z = 1.0 + 1.0 * I;
        creal(z) = 0.0;   /* 期望：编译错误（read-only） */
    }

    /* 违反约束「函数声明必须与使用一致」：
     * 若未包含 <complex.h>，ccosh 无声明，C99 下隐式声明被禁止，
     * 应报错（implicit declaration of function）。此处通过取消注释
     * 头文件来演示（实际测试时需移除 #include <complex.h>）。 */
    /* #include <complex.h> 被移除时：
       double complex r = ccosh(1.0 + 1.0 * I);   期望：编译错误 */

#endif /* 负向测试结束 */

    return 0;
}