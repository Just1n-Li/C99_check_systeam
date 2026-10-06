/*
 * 测试 C99 7.12.4.3 —— atan 函数族 (atan / atanf / atanl)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 声明与原型：double atan(double); float atanf(float); long double atanl(long double);
 *   [2] 语义：计算 x 的反正切主值。
 *   [3] 返回值：返回 arctan x，位于区间 [-pi/2, +pi/2] 弧度。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>

/* 用于比较浮点数的容差 */
#define EPS 1e-9

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型检查：三个函数均可被调用，且返回类型正确。
     *     通过将返回值赋给对应类型的变量来静态验证返回类型。 */
    double  d_ret;
    float   f_ret;
    long double l_ret;

    d_ret = atan(0.0);
    f_ret = atanf(0.0f);
    l_ret = atanl(0.0L);

    /* [2][3] 基本值：atan(0) == 0，位于区间内 */
    assert(fabs(d_ret - 0.0) < EPS);
    assert(fabsf(f_ret - 0.0f) < (float)EPS);
    assert(fabsl(l_ret - 0.0L) < (long double)EPS);

    /* [2][3] 已知精确值：atan(1) == pi/4 */
    {
        double pi = acos(-1.0);
        assert(fabs(atan(1.0) - pi / 4.0) < EPS);
        assert(fabsf(atanf(1.0f) - (float)(pi / 4.0)) < (float)EPS);
        assert(fabsl(atanl(1.0L) - (long double)(pi / 4.0)) < (long double)EPS);
    }

    /* [2][3] 奇函数性质：atan(-x) == -atan(x) */
    {
        double x = 0.5;
        assert(fabs(atan(-x) + atan(x)) < EPS);
        assert(fabsf(atanf(-0.5f) + atanf(0.5f)) < (float)EPS);
        assert(fabsl(atanl(-0.5L) + atanl(0.5L)) < (long double)EPS);
    }

    /* [3] 值域检查：结果必须落在 [-pi/2, +pi/2] 内。
     *     对一系列输入（含极大/极小值）验证。 */
    {
        double pi = acos(-1.0);
        double half_pi = pi / 2.0;
        double inputs[] = { -1e300, -1e10, -1.0, -0.1, 0.0, 0.1, 1.0, 1e10, 1e300 };
        int n = (int)(sizeof(inputs) / sizeof(inputs[0]));
        int i;
        for (i = 0; i < n; i++) {
            double r = atan(inputs[i]);
            assert(r >= -half_pi - EPS);
            assert(r <=  half_pi + EPS);
        }
    }

    /* [3] 极限行为：x -> +inf 时 atan(x) -> pi/2；x -> -inf 时 -> -pi/2 */
    {
        double pi = acos(-1.0);
        double half_pi = pi / 2.0;
        assert(fabs(atan(1e300) - half_pi) < 1e-6);
        assert(fabs(atan(-1e300) + half_pi) < 1e-6);
    }

    /* [2][3] 单调性：atan 在实数域上严格单调递增 */
    {
        double a = atan(-2.0);
        double b = atan(-1.0);
        double c = atan(0.0);
        double d = atan(1.0);
        double e = atan(2.0);
        assert(a < b && b < c && c < d && d < e);
    }

    /* [2][3] 与 tan 的互逆关系（在 (-pi/2, pi/2) 内）：
     *     atan(tan(y)) == y 对 y 在 (-pi/2, pi/2) 内成立。 */
    {
        double ys[] = { -1.0, -0.5, 0.0, 0.5, 1.0 };
        int n = (int)(sizeof(ys) / sizeof(ys[0]));
        int i;
        for (i = 0; i < n; i++) {
            double y = ys[i];
            assert(fabs(atan(tan(y)) - y) < 1e-9);
        }
    }

    /* [1] 三个函数对同一数学输入应给出一致结果（在各自精度内） */
    {
        double x = 0.75;
        double rd = atan(x);
        float  rf = atanf((float)x);
        long double rl = atanl((long double)x);
        assert(fabs((double)rf - rd) < 1e-6);
        assert(fabsl(rl - (long double)rd) < 1e-9L);
    }

    printf("C99 7.12.4.3 atan 正向测试全部通过。\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「函数调用实参个数必须与原型一致」：
     * atan 原型为 double atan(double)，只接受 1 个实参。
     * 传入 2 个实参，gcc -std=c99 应报错：
     *   error: too many arguments to function 'atan'
     */
    double bad1 = atan(1.0, 2.0);

    /*
     * 违反约束「函数调用实参个数必须与原型一致」：
     * atanf 原型为 float atanf(float)，不接受 0 个实参。
     * gcc -std=c99 应报错：
     *   error: too few arguments to function 'atanf'
     */
    float bad2 = atanf();

    /*
     * 违反约束「函数调用实参个数必须与原型一致」：
     * atanl 原型为 long double atanl(long double)，不接受 0 个实参。
     * gcc -std=c99 应报错：
     *   error: too few arguments to function 'atanl'
     */
    long double bad3 = atanl();

    /*
     * 违反约束「函数调用实参类型必须可转换为形参类型」：
     * 形参为 double，实参为结构体类型，无法隐式转换。
     * gcc -std=c99 应报错：
     *   error: incompatible type for argument 1 of 'atan'
     */
    struct S { int x; } s;
    double bad4 = atan(s);

    /*
     * 违反约束「函数调用实参类型必须可转换为形参类型」：
     * 形参为 double，实参为指针类型，指针不能隐式转换为 double。
     * gcc -std=c99 应报错：
     *   error: incompatible type for argument 1 of 'atan'
     */
    int *p = 0;
    double bad5 = atan(p);

    /*
     * 违反约束「函数调用实参类型必须可转换为形参类型」：
     * 形参为 float，实参为结构体类型，无法隐式转换。
     * gcc -std=c99 应报错：
     *   error: incompatible type for argument 1 of 'atanf'
     */
    float bad6 = atanf(s);

    /*
     * 违反约束「函数调用实参类型必须可转换为形参类型」：
     * 形参为 long double，实参为结构体类型，无法隐式转换。
     * gcc -std=c99 应报错：
     *   error: incompatible type for argument 1 of 'atanl'
     */
    long double bad7 = atanl(s);

    /*
     * 违反约束「函数返回值不可作为左值赋值」：
     * atan 返回非左值，不能对其赋值。
     * gcc -std=c99 应报错：
     *   error: lvalue required as left operand of assignment
     */
    atan(1.0) = 0.0;

    /*
     * 违反约束「函数返回值不可作为左值赋值」：
     * atanf 返回非左值，不能对其赋值。
     * gcc -std=c99 应报错：
     *   error: lvalue required as left operand of assignment
     */
    atanf(1.0f) = 0.0f;

    /*
     * 违反约束「函数返回值不可作为左值赋值」：
     * atanl 返回非左值，不能对其赋值。
     * gcc -std=c99 应报错：
     *   error: lvalue required as left operand of assignment
     */
    atanl(1.0L) = 0.0L;

    /*
     * 违反约束「函数返回值不可取地址」：
     * atan 返回非左值，不能对其取地址。
     * gcc -std=c99 应报错：
     *   error: lvalue required as unary '&' operand
     */
    double *bad8 = &atan(1.0);

    /*
     * 违反约束「函数返回值不可自增/自减」：
     * atan 返回非左值，不能对其使用 ++ 运算符。
     * gcc -std=c99 应报错：
     *   error: lvalue required as increment operand
     */
    atan(1.0)++;
#endif

    return 0;
}