/*
 * 测试 C99 7.3.9.2 —— cimag / cimagf / cimagl 函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 cimag/cimagf/cimagl 计算复数虚部，
 *             结果应等于构造时给定的虚部值，且返回类型为对应的实数类型。
 *             验证脚注 170：z == creal(z) + cimag(z)*I。
 *   负向测试：违反约束的代码应编译报错（见文件末尾 #if 0 块）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：三个函数均可用，参数为对应精度的 complex 类型 */
    double complex        zd = 3.0 + 4.0 * I;
    float  complex        zf = 1.5f + 2.5f * I;
    long double complex   zl = 7.25L + 8.75L * I;

    /* [2] cimag 计算 z 的虚部 */
    double        id = cimag(zd);
    float         if_ = cimagf(zf);
    long double   il = cimagl(zl);

    /* [3] 返回值是虚部值（作为实数） */
    assert(id == 4.0);
    assert(if_ == 2.5f);
    assert(il == 8.75L);

    /* [3] 返回类型检查：应为对应的实数类型 */
    {
        double        *pd = &id;   /* cimag 返回 double */
        float         *pf = &if_;  /* cimagf 返回 float */
        long double   *pl = &il;   /* cimagl 返回 long double */
        (void)pd; (void)pf; (void)pl;
    }

    /* [2] 纯实数的虚部为 0 */
    assert(cimag(5.0 + 0.0 * I) == 0.0);
    assert(cimagf(5.0f + 0.0f * I) == 0.0f);
    assert(cimagl(5.0L + 0.0L * I) == 0.0L);

    /* [2] 纯虚数的虚部为其系数 */
    assert(cimag(0.0 + 9.0 * I) == 9.0);

    /* [2] 负虚部 */
    assert(cimag(1.0 - 6.0 * I) == -6.0);

    /* 脚注 170：z == creal(z) + cimag(z)*I */
    assert(zd == creal(zd) + cimag(zd) * I);
    assert(zf == crealf(zf) + cimagf(zf) * I);
    assert(zl == creall(zl) + cimagl(zl) * I);

    /* 脚注 170 的数值形式再验证一次 */
    {
        double complex z = -2.5 + 3.75 * I;
        assert(creal(z) == -2.5);
        assert(cimag(z) == 3.75);
        assert(z == creal(z) + cimag(z) * I);
    }

    /* [2] 与 I 相乘的语义：cimag(z)*I 得到纯虚部 */
    {
        double complex z = 1.0 + 2.0 * I;
        double complex pure_imag = cimag(z) * I;
        assert(creal(pure_imag) == 0.0);
        assert(cimag(pure_imag) == 2.0);
    }

    printf("All positive tests for C99 7.3.9.2 (cimag) passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「cimag 的参数类型为 double complex」：
     * 传入结构体类型，gcc -std=c99 应报错（参数类型不兼容）。 */
    struct S { int x; } s;
    cimag(s);

    /* 违反约束「cimag 的参数类型为 double complex」：
     * 传入指针类型，应报错。 */
    double *p = 0;
    cimag(p);

    /* 违反约束「cimagf 的参数类型为 float complex」：
     * 传入 double complex，应报错（类型不匹配，无隐式转换）。 */
    double complex zd2 = 1.0 + 2.0 * I;
    cimagf(zd2);

    /* 违反约束「cimagl 的参数类型为 long double complex」：
     * 传入 double complex，应报错。 */
    cimagl(zd2);

    /* 违反约束「cimag 返回 double，不可作为左值赋值」：
     * 函数调用结果不是左值，赋值应报错。 */
    cimag(zd2) = 5.0;

    /* 违反约束「cimag 返回 double，不可取地址」：
     * 对非左值取地址应报错。 */
    double *bad = &cimag(zd2);

    /* 违反约束「cimag 需要 1 个参数」：
     * 参数个数不匹配，应报错。 */
    cimag();

    /* 违反约束「cimag 需要 1 个参数」：
     * 参数过多，应报错。 */
    cimag(zd2, zd2);

#endif
}