/*
 * 测试 C99 7.12.9.6 —— round / roundf / roundl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：double round(double); float roundf(float); long double roundl(long double);
 *   [2] 语义：四舍五入到最接近的整数值（浮点格式），
 *       恰好处于半整数（halfway）时向远离零的方向舍入，
 *       且不受当前舍入方向（fesetround）影响。
 *   [3] 返回值：舍入后的整数值。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* 辅助：判断浮点值是否为整数（无小数部分） */
static int is_integral_value(double v)
{
    return v == floor(v);
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：三个函数均可被调用，返回类型正确 */
    {
        double d = round(1.0);
        float  f = roundf(1.0f);
        long double ld = roundl(1.0L);
        assert(d == 1.0);
        assert(f == 1.0f);
        assert(ld == 1.0L);
    }

    /* [2] 普通非半整数：舍入到最接近的整数 */
    {
        assert(round(2.3) == 2.0);
        assert(round(2.7) == 3.0);
        assert(round(-2.3) == -2.0);
        assert(round(-2.7) == -3.0);

        assert(roundf(2.3f) == 2.0f);
        assert(roundf(2.7f) == 3.0f);
        assert(roundf(-2.3f) == -2.0f);
        assert(roundf(-2.7f) == -3.0f);

        assert(roundl(2.3L) == 2.0L);
        assert(roundl(2.7L) == 3.0L);
        assert(roundl(-2.3L) == -2.0L);
        assert(roundl(-2.7L) == -3.0L);
    }

    /* [2] 半整数情形：向远离零方向舍入（round half away from zero） */
    {
        /* 正半整数 -> 向上 */
        assert(round(0.5) == 1.0);
        assert(round(1.5) == 2.0);
        assert(round(2.5) == 3.0);
        assert(round(3.5) == 4.0);

        /* 负半整数 -> 向下（远离零） */
        assert(round(-0.5) == -1.0);
        assert(round(-1.5) == -2.0);
        assert(round(-2.5) == -3.0);
        assert(round(-3.5) == -4.0);

        /* float 版本 */
        assert(roundf(0.5f) == 1.0f);
        assert(roundf(-0.5f) == -1.0f);
        assert(roundf(2.5f) == 3.0f);
        assert(roundf(-2.5f) == -3.0f);

        /* long double 版本 */
        assert(roundl(0.5L) == 1.0L);
        assert(roundl(-0.5L) == -1.0L);
        assert(roundl(2.5L) == 3.0L);
        assert(roundl(-2.5L) == -3.0L);
    }

    /* [2] 整数输入应原样返回 */
    {
        assert(round(0.0) == 0.0);
        assert(round(5.0) == 5.0);
        assert(round(-5.0) == -5.0);
        assert(roundf(7.0f) == 7.0f);
        assert(roundl(-7.0L) == -7.0L);
    }

    /* [2] 结果必须是整数值（无小数部分） */
    {
        assert(is_integral_value(round(1.234)));
        assert(is_integral_value(round(-1.234)));
        assert(is_integral_value(round(0.5)));
        assert(is_integral_value(round(-0.5)));
    }

    /* [2] 不受当前舍入方向影响：分别设置 FE_TONEAREST / FE_UPWARD /
     *     FE_DOWNWARD / FE_TOWARDZERO，round 的结果应保持一致。
     *     注意：fesetround 需要 <fenv.h>，某些实现可能不支持，故用条件判断。 */
    {
        double half_pos = 2.5;
        double half_neg = -2.5;
        double near_pos = 2.3;
        double near_neg = -2.3;

        int saved = fegetround();

        if (fesetround(FE_TONEAREST) == 0) {
            assert(round(half_pos) == 3.0);
            assert(round(half_neg) == -3.0);
            assert(round(near_pos) == 2.0);
            assert(round(near_neg) == -2.0);
        }
        if (fesetround(FE_UPWARD) == 0) {
            /* 即使向上舍入，round 仍按 half-away-from-zero 处理 */
            assert(round(half_pos) == 3.0);
            assert(round(half_neg) == -3.0);
            assert(round(near_pos) == 2.0);
            assert(round(near_neg) == -2.0);
        }
        if (fesetround(FE_DOWNWARD) == 0) {
            assert(round(half_pos) == 3.0);
            assert(round(half_neg) == -3.0);
            assert(round(near_pos) == 2.0);
            assert(round(near_neg) == -2.0);
        }
        if (fesetround(FE_TOWARDZERO) == 0) {
            assert(round(half_pos) == 3.0);
            assert(round(half_neg) == -3.0);
            assert(round(near_pos) == 2.0);
            assert(round(near_neg) == -2.0);
        }

        /* 恢复原舍入方向 */
        if (saved != -1) {
            fesetround(saved);
        }
    }

    /* [3] 返回值类型：round 返回 double，roundf 返回 float，roundl 返回 long double。
     *     通过 sizeof 与赋值兼容性间接验证。 */
    {
        double d = round(1.5);
        float  f = roundf(1.5f);
        long double ld = roundl(1.5L);
        assert(sizeof(d) == sizeof(double));
        assert(sizeof(f) == sizeof(float));
        assert(sizeof(ld) == sizeof(long double));
        assert(d == 2.0);
        assert(f == 2.0f);
        assert(ld == 2.0L);
    }

    /* [3] 返回值可参与后续算术运算 */
    {
        double r = round(2.5) + round(-2.5);   /* 3.0 + (-3.0) = 0.0 */
        assert(r == 0.0);
        double s = round(1.4) * 2.0;           /* 1.0 * 2.0 = 2.0 */
        assert(s == 2.0);
    }

    printf("All positive tests for C99 7.12.9.6 (round/roundf/roundl) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「round 的参数必须为算术类型（浮点）」：
     * 传入结构体类型，gcc -std=c99 应报错（incompatible type for argument）。 */
    struct S { int x; } s;
    round(s);

    /* 违反约束「round 的参数个数必须为 1」：
     * 传入两个参数，gcc -std=c99 应报错（too many arguments to function）。 */
    round(1.0, 2.0);

    /* 违反约束「round 的参数必须为算术类型」：
     * 传入指针类型，gcc -std=c99 应报错（incompatible type for argument）。 */
    int *p = 0;
    round(p);

    /* 违反约束「roundf 的参数必须为算术类型」：
     * 传入结构体，gcc -std=c99 应报错。 */
    roundf(s);

    /* 违反约束「roundl 的参数必须为算术类型」：
     * 传入指针，gcc -std=c99 应报错。 */
    roundl(p);

    /* 违反约束「round 系列函数返回值不可作为左值被赋值」：
     * 函数调用结果不是左值，对其赋值应编译报错（lvalue required as left operand of assignment）。 */
    round(1.5) = 2.0;

    /* 违反约束「roundf 返回值不可赋值」：
     * 应编译报错。 */
    roundf(1.5f) = 2.0f;

    /* 违反约束「roundl 返回值不可赋值」：
     * 应编译报错。 */
    roundl(1.5L) = 2.0L;

#endif

    return 0;
}