/*
 * 测试 C99 7.12.8.2 —— erfc 函数族（erfc / erfcf / erfcl）
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 erfc/erfcf/erfcl，验证返回值语义
 *             （erfc(x) = 1 - erf(x)，erfc(0) = 1，erfc(+inf) = 0，
 *              大参数触发范围错误 range error）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <errno.h>
#include <fenv.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] Synopsis：三个函数声明必须存在于 <math.h> 中，
 *     且返回类型分别为 double / float / long double。
 *     这里通过取函数指针并检查类型来验证原型。 */
static double (*p_erfc)(double)          = erfc;
static float  (*p_erfcf)(float)          = erfcf;
static long double (*p_erfcl)(long double) = erfcl;

/* 辅助：判断两个 double 是否近似相等 */
static int close_enough(double a, double b, double tol)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= tol;
}

int main(void)
{
    /* ---------- [1] 原型/返回类型检查 ---------- */
    assert(p_erfc  != NULL);
    assert(p_erfcf != NULL);
    assert(p_erfcl != NULL);

    /* ---------- [3] 定义式：erfc(x) = 1 - erf(x) ---------- */
    {
        double xs[] = { 0.0, 0.5, 1.0, 2.0, -0.5, -1.0, 3.0 };
        size_t i;
        for (i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
            double x = xs[i];
            double lhs = erfc(x);
            double rhs = 1.0 - erf(x);
            assert(close_enough(lhs, rhs, 1e-12));
        }
    }

    /* ---------- [3] 特殊值：erfc(0) = 1 ---------- */
    assert(close_enough(erfc(0.0), 1.0, 1e-15));
    assert(close_enough((double)erfcf(0.0f), 1.0, 1e-6));
    assert(close_enough((double)erfcl(0.0L), 1.0, 1e-15));

    /* ---------- [3] 对称性：erfc(-x) = 2 - erfc(x) ---------- */
    {
        double x = 1.25;
        assert(close_enough(erfc(-x), 2.0 - erfc(x), 1e-12));
    }

    /* ---------- [3] 极限：erfc(+inf) = 0, erfc(-inf) = 2 ---------- */
    assert(erfc(INFINITY) == 0.0);
    assert(close_enough(erfc(-INFINITY), 2.0, 1e-15));

    /* ---------- [3] 单调递减（x 增大，erfc 减小） ---------- */
    {
        double a = erfc(0.5);
        double b = erfc(1.5);
        double c = erfc(2.5);
        assert(a > b && b > c);
    }

    /* ---------- [2] 范围错误：x 太大时发生 range error ----------
     * 标准只说 "A range error occurs if x is too large"。
     * 实现通常返回 0（下溢）并可能置 errno = ERANGE。
     * 这里只验证返回值非负且有限/为 0，不强制 errno（实现相关）。 */
    {
        double big = 1e300;
        errno = 0;
        double r = erfc(big);
        /* 结果应为 0（下溢到 0），且不应为 NaN */
        assert(!isnan(r));
        assert(r >= 0.0);
        assert(r == 0.0);   /* 对极大参数，erfc 下溢为 0 */
    }

    /* ---------- [2] 大参数下 erfcf / erfcl 同样下溢为 0 ---------- */
    {
        float rf = erfcf(1e30f);
        long double rl = erfcl(1e300L);
        assert(rf == 0.0f);
        assert(rl == 0.0L);
    }

    /* ---------- [3] 与 erf 的互补关系在 float / long double 上也成立 ---------- */
    {
        float xf = 0.75f;
        assert(close_enough((double)erfcf(xf), 1.0 - (double)erf(xf), 1e-5));

        long double xl = 0.75L;
        assert(close_enough((double)erfcl(xl), 1.0 - (double)erf(xl), 1e-12));
    }

    /* ---------- [3] 值域：0 <= erfc(x) <= 2 ---------- */
    {
        double xs[] = { -5.0, -1.0, 0.0, 1.0, 5.0, 10.0 };
        size_t i;
        for (i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
            double v = erfc(xs[i]);
            assert(v >= 0.0 && v <= 2.0);
        }
    }

    printf("All positive tests for C99 7.12.8.2 (erfc) passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「函数调用实参个数/类型必须与原型匹配」：
 * erfc 原型为 double erfc(double)，传入两个实参应报错。
 * 期望：error: too many arguments to function 'erfc' */
double bad1 = erfc(1.0, 2.0);

/* 违反约束「函数调用实参必须可转换为形参类型」：
 * 传入结构体，无法转换为 double，应报错。
 * 期望：error: incompatible type for argument 1 of 'erfc' */
struct S { int x; };
double bad2(struct S s) { return erfc(s); }

/* 违反约束「erfc 返回 double，不能作为函数被调用」：
 * 对非函数类型使用函数调用运算符，应报错。
 * 期望：error: called object is not a function or function pointer */
double bad3 = erfc(1.0)(2.0);

/* 违反约束「赋值目标必须是可修改左值」：
 * erfc 的返回值不是左值，不能赋值。
 * 期望：error: lvalue required as left operand of assignment */
void bad4(void) { erfc(1.0) = 0.5; }

/* 违反约束「取地址运算符 & 的操作数必须是左值或函数指示符」：
 * 对函数调用结果取地址，应报错。
 * 期望：error: lvalue required as unary '&' operand */
double *bad5 = &erfc(1.0);

/* 违反约束「erfcf 形参为 float，实参为结构体无法转换」：
 * 期望：error: incompatible type for argument 1 of 'erfcf' */
float bad6(struct S s) { return erfcf(s); }

/* 违反约束「erfcl 形参为 long double，实参为结构体无法转换」：
 * 期望：error: incompatible type for argument 1 of 'erfcl' */
long double bad7(struct S s) { return erfcl(s); }

#endif