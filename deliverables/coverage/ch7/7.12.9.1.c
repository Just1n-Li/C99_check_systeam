/*
 * 测试 C99 7.12.9.1 —— ceil / ceilf / ceill 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 声明与原型（<math.h> 中 double ceil(double); float ceilf(float);
 *       long double ceill(long double);）
 *   [2] 语义：计算不小于 x 的最小整数值
 *   [3] 返回值：以浮点数形式返回 ⌈x⌉
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：三个函数的返回类型与参数类型必须与条款一致。
 *     通过取函数指针并赋值给精确匹配的类型来静态验证原型。 */
static double (*p_ceil)(double)          = ceil;
static float  (*p_ceilf)(float)          = ceilf;
static long double (*p_ceill)(long double) = ceill;

/* [2][3] 辅助：判断浮点值是否为整数（即等于其截断值） */
static int is_integral_value(double v)
{
    return v == floor(v);
}

int main(void)
{
    /* ---------- [1] 原型可用性 ---------- */
    assert(p_ceil  != NULL);
    assert(p_ceilf != NULL);
    assert(p_ceill != NULL);

    /* ---------- [2][3] 正数：非整数 -> 向上取整 ---------- */
    assert(ceil(2.1)  == 3.0);
    assert(ceil(2.9)  == 3.0);
    assert(ceil(0.1)  == 1.0);
    assert(ceil(0.0001) == 1.0);

    /* ---------- [2][3] 正数：已是整数 -> 返回自身 ---------- */
    assert(ceil(3.0)  == 3.0);
    assert(ceil(0.0)  == 0.0);
    assert(ceil(42.0) == 42.0);

    /* ---------- [2][3] 负数：非整数 -> 向零方向（数值更大）取整 ---------- */
    assert(ceil(-2.1) == -2.0);
    assert(ceil(-2.9) == -2.0);
    assert(ceil(-0.1) == -0.0);   /* 结果数值为 0，符号可为负零 */
    assert(ceil(-0.1) == 0.0);    /* -0.0 == 0.0 为真 */

    /* ---------- [2][3] 负数：已是整数 -> 返回自身 ---------- */
    assert(ceil(-3.0) == -3.0);
    assert(ceil(-42.0) == -42.0);

    /* ---------- [3] 返回值必须是浮点类型且为整数值 ---------- */
    {
        double r = ceil(2.5);
        assert(is_integral_value(r));
        assert(r == 3.0);
    }

    /* ---------- [2] 语义核心：ceil(x) 是 >= x 的最小整数 ---------- */
    {
        double xs[] = { 2.1, -2.1, 0.5, -0.5, 7.0, -7.0, 1e-9, -1e-9 };
        size_t i;
        for (i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
            double x = xs[i];
            double c = ceil(x);
            /* 结果 >= x */
            assert(c >= x);
            /* 结果是整数 */
            assert(is_integral_value(c));
            /* 不存在比 c 更小的整数仍 >= x：即 c - 1 < x */
            assert(c - 1.0 < x);
        }
    }

    /* ---------- [1][2][3] ceilf：float 版本 ---------- */
    {
        float rf = ceilf(2.1f);
        assert(rf == 3.0f);
        assert(ceilf(-2.1f) == -2.0f);
        assert(ceilf(5.0f)  == 5.0f);
        assert(ceilf(0.0f)  == 0.0f);
        /* 结果类型为 float */
        assert(sizeof(ceilf(1.5f)) == sizeof(float));
    }

    /* ---------- [1][2][3] ceill：long double 版本 ---------- */
    {
        long double rl = ceill(2.1L);
        assert(rl == 3.0L);
        assert(ceill(-2.1L) == -2.0L);
        assert(ceill(5.0L)  == 5.0L);
        assert(ceill(0.0L)  == 0.0L);
        /* 结果类型为 long double */
        assert(sizeof(ceill(1.5L)) == sizeof(long double));
    }

    /* ---------- [2][3] 大数值：超出精确整数范围时仍返回自身 ---------- */
    {
        double big = 1e300;
        assert(ceil(big) == big);
        assert(ceil(-big) == -big);
    }

    /* ---------- [2][3] 与 floor 的关系：ceil(x) == -floor(-x) ---------- */
    {
        double xs[] = { 2.3, -2.3, 0.7, -0.7, 10.0, -10.0 };
        size_t i;
        for (i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
            assert(ceil(xs[i]) == -floor(-xs[i]));
        }
    }

    printf("C99 7.12.9.1 ceil/ceilf/ceill: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「ceil 的参数必须为 double 类型（算术类型可隐式转换）」：
 * 传入结构体类型，无法转换为 double，gcc -std=c99 应报错。 */
struct S { int x; } s;
double bad1 = ceil(s);   /* error: incompatible type for argument 1 of 'ceil' */

/* 违反约束「ceil 的参数个数必须为 1」：
 * 参数个数不匹配，应报错。 */
double bad2 = ceil(1.0, 2.0);   /* error: too many arguments to function 'ceil' */

/* 违反约束「ceil 的参数个数必须为 1」：
 * 缺少参数，应报错。 */
double bad3 = ceil();   /* error: too few arguments to function 'ceil' */

/* 违反约束「ceilf 的参数必须可转换为 float」：
 * 传入指针类型，无法隐式转换为 float，应报错。 */
int *ip;
float bad4 = ceilf(ip);   /* error: incompatible type for argument 1 of 'ceilf' */

/* 违反约束「ceill 的参数必须可转换为 long double」：
 * 传入结构体，应报错。 */
long double bad5 = ceill(s);   /* error: incompatible type for argument 1 of 'ceill' */

/* 违反约束「ceil 的返回值不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
ceil(1.5) = 2.0;   /* error: lvalue required as left operand of assignment */

/* 违反约束「ceilf 的返回值不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
ceilf(1.5f) = 2.0f;   /* error: lvalue required as left operand of assignment */

/* 违反约束「ceill 的返回值不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
ceill(1.5L) = 2.0L;   /* error: lvalue required as left operand of assignment */

#endif