/*
 * 测试条款：C99 7.12.5.6 —— The tanh functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 tanh / tanhf / tanhl，
 *             验证其返回双曲正切值，程序应能编译并运行通过。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型，参数与返回类型
 *   [2] Description：计算 x 的双曲正切
 *   [3] Returns：返回 tanh x
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证三个函数原型存在且类型正确。
 *     通过取函数指针并检查其类型来确认签名。 */
static double  (*p_tanh)(double)        = tanh;
static float   (*p_tanhf)(float)        = tanhf;
static long double (*p_tanhl)(long double) = tanhl;

/* 辅助：判断两个 double 是否近似相等 */
static int dclose(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* [1] 原型可用性：能取地址即说明声明存在且类型匹配 */
    assert(p_tanh  != NULL);
    assert(p_tanhf != NULL);
    assert(p_tanhl != NULL);

    /* [2][3] tanh(0) == 0 */
    assert(tanh(0.0) == 0.0);

    /* [2][3] tanh 是奇函数：tanh(-x) == -tanh(x) */
    {
        double x = 0.7;
        assert(dclose(tanh(-x), -tanh(x), 1e-15));
    }

    /* [2][3] 已知值：tanh(1) ≈ 0.7615941559557649 */
    assert(dclose(tanh(1.0), 0.7615941559557649, 1e-15));

    /* [2][3] 大正数趋近 1 */
    assert(dclose(tanh(20.0), 1.0, 1e-12));

    /* [2][3] 大负数趋近 -1 */
    assert(dclose(tanh(-20.0), -1.0, 1e-12));

    /* [2][3] 恒等式：tanh(x) = sinh(x)/cosh(x) */
    {
        double x = 0.3;
        assert(dclose(tanh(x), sinh(x) / cosh(x), 1e-15));
    }

    /* [2][3] 恒等式：1 - tanh(x)^2 = 1/cosh(x)^2 */
    {
        double x = 0.9;
        double t = tanh(x);
        assert(dclose(1.0 - t * t, 1.0 / (cosh(x) * cosh(x)), 1e-14));
    }

    /* [1][2][3] float 版本 tanhf */
    assert(tanhf(0.0f) == 0.0f);
    {
        float x = 1.0f;
        float r = tanhf(x);
        assert(r > 0.7615941f && r < 0.7615943f);
    }

    /* [1][2][3] long double 版本 tanhl */
    assert(tanhl(0.0L) == 0.0L);
    {
        long double x = 1.0L;
        long double r = tanhl(x);
        long double diff = r - 0.7615941559557649L;
        if (diff < 0) diff = -diff;
        assert(diff <= 1e-15L);
    }

    /* [2][3] 三个版本对同一输入应给出一致结果（在各自精度内） */
    {
        double xd = 0.5;
        float  xf = 0.5f;
        long double xl = 0.5L;
        double rd = tanh(xd);
        double rf = (double)tanhf(xf);
        double rl = (double)tanhl(xl);
        assert(dclose(rd, rf, 1e-6));
        assert(dclose(rd, rl, 1e-15));
    }

    /* [2][3] 定义域为全体实数：任意有限输入均返回有限值 */
    {
        double vals[] = { -100.0, -1.0, -0.001, 0.0, 0.001, 1.0, 100.0 };
        int i;
        for (i = 0; i < 7; i++) {
            double r = tanh(vals[i]);
            assert(r >= -1.0 && r <= 1.0);
        }
    }

    printf("All positive tests for C99 7.12.5.6 (tanh) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参个数必须与原型一致」：
 * tanh 原型只接受 1 个参数，传 2 个参数，gcc -std=c99 应报错
 *   error: too many arguments to function 'tanh' */
double bad1 = tanh(1.0, 2.0);

/* 违反约束「实参类型必须可转换为形参类型」：
 * 结构体类型无法隐式转换为 double，gcc -std=c99 应报错
 *   error: incompatible type for argument 1 of 'tanh' */
struct S { int x; };
double bad2(struct S s) { return tanh(s); }

/* 违反约束「函数调用必须使用已声明的标识符」：
 * tanh 未声明（未包含 <math.h> 且无自身声明），
 * 在 C99 中隐式函数声明已被移除，gcc -std=c99 应报错
 *   error: implicit declaration of function 'tanh' */
double bad3(void) { return tanh(1.0); }

/* 违反约束「赋值目标必须是可修改左值」：
 * 函数调用结果不是左值，不能赋值，gcc -std=c99 应报错
 *   error: lvalue required as left operand of assignment */
void bad4(void) { tanh(1.0) = 0.5; }

/* 违反约束「取地址操作数必须是左值或函数指示符」：
 * 函数返回的 double 不是左值，不能取地址，gcc -std=c99 应报错
 *   error: lvalue required as unary '&' operand */
double *bad5(void) { return &tanh(1.0); }

#endif