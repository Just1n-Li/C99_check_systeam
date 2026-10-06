/*
 * 测试 C99 7.12.6.3 —— expm1 函数族
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 expm1 / expm1f / expm1l，
 *             验证返回值等于 e^x - 1（在浮点容差内），
 *             并验证小量 x 时 expm1 比 exp(x)-1 更精确（脚注 208）。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），
 *             期望编译器在 -std=c99 下报错。
 *
 * 覆盖段落：[1] 原型/头文件、[2] 语义（e^x - 1，x 过大时范围错误）、
 *           [3] 返回值、脚注 208。
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/* 相对容差比较辅助 */
static int close_enough(double a, double b, double tol)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= tol * (1.0 + (b < 0 ? -b : b));
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 头文件 <math.h> 提供三个原型：double/float/long double 版本。
     *     通过取函数指针验证原型存在且类型正确。 */
    {
        double (*pd)(double)        = expm1;
        float  (*pf)(float)         = expm1f;
        long double (*pl)(long double) = expm1l;
        assert(pd != NULL && pf != NULL && pl != NULL);
    }

    /* [2][3] 语义：expm1(x) 返回 e^x - 1。
     *        用已知点验证：x=0 -> 0；x=1 -> e-1；x=ln2 -> 1。 */
    {
        double e = exp(1.0);
        assert(close_enough(expm1(0.0), 0.0, 1e-15));
        assert(close_enough(expm1(1.0), e - 1.0, 1e-12));
        assert(close_enough(expm1(log(2.0)), 1.0, 1e-12));
        assert(close_enough(expm1(-1.0), 1.0/e - 1.0, 1e-12));
    }

    /* [2][3] float 版本：expm1f(x) ≈ e^x - 1（float 精度） */
    {
        float ef = expf(1.0f);
        assert(fabsf(expm1f(1.0f) - (ef - 1.0f)) <= 1e-5f);
        assert(expm1f(0.0f) == 0.0f);
    }

    /* [2][3] long double 版本：expm1l(x) ≈ e^x - 1 */
    {
        long double el = expl(1.0L);
        long double r  = expm1l(1.0L);
        long double d  = r - (el - 1.0L);
        if (d < 0) d = -d;
        assert(d <= 1e-15L);
        assert(expm1l(0.0L) == 0.0L);
    }

    /* 脚注 208：小量 x 时 expm1(x) 比 exp(x)-1 更精确。
     * 取 x = 1e-10，exp(x)-1 因灾难性抵消损失有效位，
     * 而 expm1(x) 应接近 x（相对误差很小）。 */
    {
        double x = 1e-10;
        double naive = exp(x) - 1.0;   /* 抵消严重 */
        double good  = expm1(x);
        /* expm1 的结果应非常接近 x */
        assert(close_enough(good, x, 1e-6));
        /* 朴素算法误差明显更大（相对误差远大于 expm1） */
        double err_naive = fabs(naive - x) / x;
        double err_good  = fabs(good  - x) / x;
        assert(err_good <= err_naive);
    }

    /* [2] 范围错误：x 过大时发生范围错误（返回 HUGE_VAL，errno=ERANGE）。
     *     这里只验证返回值语义，不强制检查 errno（实现相关）。 */
    {
        double big = expm1(1e300);   /* e^(1e300) 溢出 */
        assert(isinf(big) || big == HUGE_VAL);
    }

    /* [3] 返回值类型检查：expm1 返回 double，expm1f 返回 float，
     *     expm1l 返回 long double。用 sizeof 验证。 */
    {
        assert(sizeof(expm1(0.0))  == sizeof(double));
        assert(sizeof(expm1f(0.0f)) == sizeof(float));
        assert(sizeof(expm1l(0.0L)) == sizeof(long double));
    }

    printf("expm1 tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 违反约束「expm1 的原型为 double expm1(double)」：
     * 用不兼容的实参类型（结构体）调用，gcc -std=c99 应报错。 */
    struct S { int x; } s;
    expm1(s);                 /* error: incompatible type for argument 1 */

    /* 违反约束「expm1 返回 double，不可作为赋值目标」：
     * 函数调用结果不是左值，对它赋值应报错。 */
    expm1(1.0) = 2.0;         /* error: lvalue required as left operand of assignment */

    /* 违反约束「expm1f 参数为 float」：传入结构体指针，类型不兼容。 */
    expm1f(&s);               /* error: incompatible type for argument 1 */

    /* 违反约束「expm1l 参数为 long double」：传入字符串字面量。 */
    expm1l("hello");          /* error: incompatible type for argument 1 */

    /* 违反约束「expm1 需要 1 个实参」：实参个数不符。 */
    expm1();                  /* error: too few arguments to function 'expm1' */
    expm1(1.0, 2.0);          /* error: too many arguments to function 'expm1' */
#endif
}