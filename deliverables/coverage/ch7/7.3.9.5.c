/*
 * 测试 C99 7.3.9.5 —— creal / crealf / creall 函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 creal/crealf/creall 计算复数的实部，
 *             结果应等于复数的实部值；并验证脚注 171 的恒等式
 *             z == creal(z) + cimag(z)*I。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：三个函数均可用，返回类型分别为 double/float/long double */
    double complex        z  = 3.0 + 4.0 * I;
    float  complex        zf = 1.5f + 2.5f * I;
    long double complex   zl = 7.25L + 8.75L * I;

    /* [2] creal 计算 z 的实部 */
    double      r  = creal(z);
    float       rf = crealf(zf);
    long double rl = creall(zl);

    /* [3] 返回值即实部数值 */
    assert(r  == 3.0);
    assert(rf == 1.5f);
    assert(rl == 7.25L);

    /* [3] 纯实数复数：实部就是该实数，虚部为 0 */
    double complex pure = 5.0 + 0.0 * I;
    assert(creal(pure) == 5.0);

    /* [3] 纯虚数复数：实部为 0 */
    double complex imag_only = 0.0 + 9.0 * I;
    assert(creal(imag_only) == 0.0);

    /* [3] 负实部 */
    double complex neg = -2.5 + 1.0 * I;
    assert(creal(neg) == -2.5);

    /* [3] 实部为 0 的零复数 */
    double complex zero = 0.0 + 0.0 * I;
    assert(creal(zero) == 0.0);

    /* 脚注 171：z == creal(z) + cimag(z)*I */
    assert(z  == creal(z)  + cimag(z)  * I);
    assert(zf == crealf(zf) + cimagf(zf) * I);
    assert(zl == creall(zl) + cimagl(zl) * I);

    /* 脚注 171 对负实部/纯虚数同样成立 */
    assert(neg       == creal(neg)       + cimag(neg)       * I);
    assert(imag_only == creal(imag_only) + cimag(imag_only) * I);

    /* 类型检查：creal 返回 double，crealf 返回 float，creall 返回 long double */
    assert(sizeof(creal(z))  == sizeof(double));
    assert(sizeof(crealf(zf)) == sizeof(float));
    assert(sizeof(creall(zl)) == sizeof(long double));

    printf("All positive tests for C99 7.3.9.5 (creal/crealf/creall) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「creal 的参数类型为 double complex」：
       传入不兼容的指针类型，gcc -std=c99 应报错（参数类型不匹配）。 */
    {
        double *p = 0;
        double bad = creal(p);   /* 期望：编译错误，实参类型不兼容 */
        (void)bad;
    }

    /* 违反约束「creal 的参数类型为 double complex」：
       传入结构体类型，无法隐式转换为复数类型，应报错。 */
    {
        struct S { int x; } s;
        double bad = creal(s);   /* 期望：编译错误，实参类型不兼容 */
        (void)bad;
    }

    /* 违反约束「crealf 的参数类型为 float complex」：
       传入 double complex 会丢失精度，C99 不允许隐式转换，应报错。 */
    {
        double complex dz = 1.0 + 2.0 * I;
        float bad = crealf(dz);  /* 期望：编译错误，实参类型不兼容 */
        (void)bad;
    }

    /* 违反约束「creall 的参数类型为 long double complex」：
       传入 double complex，应报错。 */
    {
        double complex dz = 1.0 + 2.0 * I;
        long double bad = creall(dz);  /* 期望：编译错误，实参类型不兼容 */
        (void)bad;
    }

    /* 违反约束「creal 返回 double，不可作为左值赋值」：
       函数调用结果不是左值，对其赋值应报错。 */
    {
        double complex dz = 1.0 + 2.0 * I;
        creal(dz) = 5.0;   /* 期望：编译错误，赋值目标不是左值 */
    }

    /* 违反约束「creal 返回 double，不可取地址」：
       对函数返回值取地址应报错。 */
    {
        double complex dz = 1.0 + 2.0 * I;
        double *p = &creal(dz);  /* 期望：编译错误，不能取右值地址 */
        (void)p;
    }

    /* 违反约束「creal 需要 <complex.h> 中的原型」：
       若未包含头文件，隐式声明返回 int，与 double 赋值冲突，
       在 C99 中隐式函数声明本身即为约束违反，应报错。 */
    {
        /* 假设此处未包含 <complex.h> */
        double bad = creal_undeclared(1.0);  /* 期望：编译错误，未声明标识符 */
        (void)bad;
    }

#endif /* 负向测试结束 */

    return 0;
}