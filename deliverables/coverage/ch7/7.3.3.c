/*
 * 测试 C99 7.3.3 —— Branch cuts（分支切割）
 *
 * 预期行为：
 *   正向测试：在支持有符号零（IEC 60559）的实现上，复数数学函数在分支切割
 *             两侧的取值符号（虚部 ±0）应能区分切割的两侧，函数在切割两侧
 *             分别连续；在不支持有符号零的实现上，切割应映射为从有限端点
 *             逆时针绕行时连续的一侧。
 *   负向测试：本条款为“行为规定”，不含语法约束（constraint），因此没有
 *             可触发的编译期约束违反。为保持格式，负向块中给出说明性片段。
 *
 * 说明：本条款本身不引入新的语法约束，其“约束”体现在实现必须满足的
 *       数值行为上。因此负向测试部分仅列出与条款相关的、期望编译报错的
 *       误用（例如把复数函数用于非复数类型、缺少 <complex.h> 等），
 *       这些属于其他条款的约束，此处仅作演示。
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 有符号零区分分支切割两侧：
 *     对于 sqrt，负实轴是分支切割。切割上方（虚部 +0）映射到正虚轴，
 *     切割下方（虚部 -0）映射到负虚轴。
 *     在支持有符号零的实现上，csqrt(-1.0 + 0.0*I) 的虚部应为 +1，
 *     csqrt(-1.0 - 0.0*I) 的虚部应为 -1。
 */
static void test_signed_zero_branch_cut(void)
{
    double complex top = csqrt(-1.0 + 0.0 * I);   /* 切割上方 */
    double complex bot = csqrt(-1.0 - 0.0 * I);   /* 切割下方 */

    /* 实部都应为 0（数值上），虚部分别为 +1 和 -1 */
    assert(fabs(creal(top)) < 1e-12);
    assert(fabs(creal(bot)) < 1e-12);

    /* 关键：虚部符号区分两侧 */
    assert(cimag(top) > 0.0);
    assert(cimag(bot) < 0.0);

    /* 更严格：检查符号位（有符号零语义） */
    assert(signbit(cimag(top)) == 0);
    assert(signbit(cimag(bot)) == 1);

    printf("[1] sqrt 分支切割两侧：top = %g%+gi, bot = %g%+gi\n",
           creal(top), cimag(top), creal(bot), cimag(bot));
}

/* [1] 连续性：从切割上方逼近与从切割下方逼近，函数值分别连续。
 *     取负实轴上靠近 -1 的点，从上方/下方逼近，结果应分别趋近 +i / -i。
 */
static void test_continuity_along_cut(void)
{
    double eps = 1e-6;
    double complex a = csqrt(-1.0 + eps * I);   /* 上方 */
    double complex b = csqrt(-1.0 - eps * I);   /* 下方 */

    /* 上方逼近：虚部趋近 +1 */
    assert(cimag(a) > 0.0);
    assert(fabs(cimag(a) - 1.0) < 1e-3);

    /* 下方逼近：虚部趋近 -1 */
    assert(cimag(b) < 0.0);
    assert(fabs(cimag(b) + 1.0) < 1e-3);

    printf("[1] 沿切割逼近：a = %g%+gi, b = %g%+gi\n",
           creal(a), cimag(a), creal(b), cimag(b));
}

/* [2] 不支持有符号零的实现：切割应映射为从有限端点逆时针绕行时连续的一侧。
 *     对于 sqrt，负实轴切割的有限端点是 0。逆时针绕 0 从负实轴上方接近，
 *     应映射到正虚轴。这里用“从上方接近”来模拟该行为。
 *     在不支持有符号零的实现上，csqrt(-1.0) 应给出 +i（正虚轴）。
 */
static void test_unsigned_zero_fallback(void)
{
    double complex r = csqrt(-1.0);   /* 无符号零时，取逆时针绕行一侧 */

    /* 无论实现是否支持有符号零，csqrt(-1.0) 的虚部应为 +1（正虚轴） */
    assert(fabs(creal(r)) < 1e-12);
    assert(cimag(r) > 0.0);
    assert(fabs(cimag(r) - 1.0) < 1e-12);

    printf("[2] csqrt(-1.0) = %g%+gi（应为 +i）\n", creal(r), cimag(r));
}

/* [1] 其他带分支切割的函数：log 沿负实轴有切割。
 *     从上方接近负实轴，虚部趋近 +pi；从下方接近，虚部趋近 -pi。
 */
static void test_log_branch_cut(void)
{
    double complex top = clog(-1.0 + 0.0 * I);
    double complex bot = clog(-1.0 - 0.0 * I);

    /* 实部都应为 0（log(1) = 0） */
    assert(fabs(creal(top)) < 1e-12);
    assert(fabs(creal(bot)) < 1e-12);

    /* 虚部分别趋近 +pi 和 -pi */
    assert(fabs(cimag(top) - M_PI) < 1e-12);
    assert(fabs(cimag(bot) + M_PI) < 1e-12);

    printf("[1] log 分支切割：top = %g%+gi, bot = %g%+gi\n",
           creal(top), cimag(top), creal(bot), cimag(bot));
}

/* [1] 分支切割两侧的符号区分：使用 signbit 检查虚部符号位。
 *     这是条款 [1] 的核心：有符号零区分切割两侧。
 */
static void test_signbit_distinguishes_sides(void)
{
    double complex top = csqrt(-4.0 + 0.0 * I);
    double complex bot = csqrt(-4.0 - 0.0 * I);

    /* sqrt(-4) = ±2i */
    assert(fabs(creal(top)) < 1e-12);
    assert(fabs(creal(bot)) < 1e-12);
    assert(fabs(cimag(top) - 2.0) < 1e-12);
    assert(fabs(cimag(bot) + 2.0) < 1e-12);

    /* 符号位区分 */
    assert(signbit(cimag(top)) == 0);
    assert(signbit(cimag(bot)) == 1);

    printf("[1] signbit 区分：top 虚部符号位 = %d, bot 虚部符号位 = %d\n",
           signbit(cimag(top)), signbit(cimag(bot)));
}

int main(void)
{
    printf("=== C99 7.3.3 Branch cuts 正向测试 ===\n");

    test_signed_zero_branch_cut();      /* [1] */
    test_continuity_along_cut();        /* [1] */
    test_unsigned_zero_fallback();      /* [2] */
    test_log_branch_cut();              /* [1] */
    test_signbit_distinguishes_sides(); /* [1] */

    printf("=== 所有正向测试通过 ===\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 说明：C99 7.3.3 是“行为规定”条款，本身不包含语法约束（constraint）。
 *       因此严格来说没有“违反 7.3.3 约束”的代码片段。
 *       下面列出的是与复数数学函数相关的、期望编译器报错的误用，
 *       它们违反的是其他条款的约束，仅作演示。
 */

/* 违反约束「<complex.h> 中函数要求复数参数」：
 * 把 double complex 函数用于不兼容类型，或缺少头文件声明。
 * gcc -std=c99 应报错（隐式声明或类型不匹配）。 */
double complex bad1 = csqrt(1.0, 2.0);   /* csqrt 只接受一个参数 */

/* 违反约束「复数类型不能直接与整数进行某些运算」：
 * 对复数使用 % 运算符（% 要求整数操作数）。 */
double complex z = 1.0 + 2.0 * I;
int bad2 = z % 2;   /* 错误：% 的操作数必须为整数类型 */

/* 违反约束「不能对复数使用位运算符」：
 * 位运算符要求整数操作数。 */
int bad3 = z & 1;   /* 错误：& 的操作数必须为整数类型 */

/* 违反约束「函数参数数量必须匹配」：
 * clog 只接受一个参数。 */
double complex bad4 = clog(1.0, 2.0);   /* 错误：参数过多 */

#endif