/*
 * 测试 C99 7.12.6.1 —— exp 函数族（exp / expf / expl）
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 exp/expf/expl，验证返回值等于 e^x，
 *             并验证 float/long double 版本的存在与精度。
 *   负向测试：违反约束的代码（如参数个数错误、未声明就调用等）应编译报错。
 *
 * 说明：本条款只规定“计算以 e 为底的指数”，未规定具体精度误差，
 *       因此正向测试使用容差比较，不要求逐位相等。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* 容差比较辅助函数 */
static int close_enough(double a, double b, double tol)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= tol;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 头文件 <math.h> 提供 exp / expf / expl 的声明。
     *     能取到函数地址即证明声明存在且类型正确。 */
    double (*pd)(double)          = exp;
    float  (*pf)(float)           = expf;
    long double (*pl)(long double)= expl;
    assert(pd != NULL && pf != NULL && pl != NULL);

    /* [2][3] exp(x) 计算 e^x，返回 e 的 x 次幂。
     *        e^0 = 1 */
    assert(close_enough(exp(0.0), 1.0, 1e-12));

    /* e^1 = e ≈ 2.718281828459045 */
    assert(close_enough(exp(1.0), 2.718281828459045, 1e-12));

    /* e^2 ≈ 7.389056098930650 */
    assert(close_enough(exp(2.0), 7.389056098930650, 1e-12));

    /* e^(-1) ≈ 0.367879441171442 */
    assert(close_enough(exp(-1.0), 0.367879441171442, 1e-12));

    /* 与 log 互逆：exp(log(5)) ≈ 5 */
    assert(close_enough(exp(log(5.0)), 5.0, 1e-12));

    /* [1][3] float 版本 expf 返回 float，值应接近 e^x */
    {
        float r = expf(1.0f);
        assert(close_enough((double)r, 2.718281828459045, 1e-5));
        assert(close_enough((double)expf(0.0f), 1.0, 1e-6));
    }

    /* [1][3] long double 版本 expl 返回 long double，值应接近 e^x */
    {
        long double r = expl(1.0L);
        assert(close_enough((double)r, 2.718281828459045, 1e-12));
        assert(close_enough((double)expl(0.0L), 1.0, 1e-12));
    }

    /* [2] 大参数：e^10 ≈ 22026.4657948067 */
    assert(close_enough(exp(10.0), 22026.4657948067, 1e-6));

    /* [2] 负大参数：e^(-10) ≈ 4.539992976248485e-5 */
    assert(close_enough(exp(-10.0), 4.539992976248485e-5, 1e-15));

    /* [2] 参数为整数常量时，隐式转换为 double（原型在作用域内） */
    assert(close_enough(exp(3), 20.085536923187668, 1e-12));

    /* [3] 返回值类型为 double（sizeof 验证） */
    assert(sizeof(exp(1.0)) == sizeof(double));
    assert(sizeof(expf(1.0f)) == sizeof(float));
    assert(sizeof(expl(1.0L)) == sizeof(long double));

    printf("正向测试全部通过：exp/expf/expl 语义符合 C99 7.12.6.1\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「exp 原型为 double exp(double)」：参数个数错误，
     * gcc -std=c99 应报 "too few arguments to function 'exp'" */
    exp();

    /* 违反约束「exp 原型为 double exp(double)」：参数过多，
     * gcc -std=c99 应报 "too many arguments to function 'exp'" */
    exp(1.0, 2.0);

    /* 违反约束「expf 原型为 float expf(float)」：参数个数错误，
     * gcc -std=c99 应报参数个数错误 */
    expf();

    /* 违反约束「expl 原型为 long double expl(long double)」：参数个数错误，
     * gcc -std=c99 应报参数个数错误 */
    expl();

    /* 违反约束「函数调用必须使用函数或函数指针」：对非函数对象调用，
     * gcc -std=c99 应报 "called object is not a function" */
    {
        int not_a_function = 0;
        not_a_function(1.0);
    }

    /* 违反约束「exp 返回 double，不能作为左值赋值」：
     * 函数调用结果不是左值，gcc -std=c99 应报 "lvalue required as left operand of assignment" */
    exp(1.0) = 2.0;

    /* 违反约束「expf 返回 float，不能取地址赋值」：
     * 函数调用结果不是左值，gcc -std=c99 应报 "lvalue required" */
    &expf(1.0f);

#endif

    return 0;
}