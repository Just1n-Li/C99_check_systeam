/*
 * 测试 C99 7.6.2.5 —— fetestexcept 函数
 *
 * 预期行为：
 *   正向测试：包含 <fenv.h>，调用 fetestexcept 查询浮点异常标志，
 *             返回值应为所查询标志中当前已置位标志的按位或（[3]）。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如参数个数/类型错误、未包含头文件等）
 *             应导致编译报错，统一放在 #if 0 ... #endif 中。
 *
 * 覆盖段落：
 *   [1] 原型声明 int fetestexcept(int excepts);
 *   [2] 查询 excepts 指定的浮点状态标志子集
 *   [3] 返回当前已置位且包含在 excepts 中的异常宏的按位或
 *   [4] EXAMPLE：先 feclearexcept，再 fetestexcept，按位测试
 *   Footnote 188：一次调用可测试多个异常
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 允许编译器对 fenv 访问进行优化控制（C99 6.5.1 中 #pragma STDC FENV_ACCESS） */
#pragma STDC FENV_ACCESS ON

/* 供 EXAMPLE 使用的两个函数 */
static int f_called = 0;
static int g_called = 0;
static void f(void) { f_called = 1; }
static void g(void) { g_called = 1; }

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型：fetestexcept 接受一个 int 参数，返回 int。
     *     这里通过取函数指针来静态验证原型签名。 */
    {
        int (*fp)(int) = fetestexcept;
        assert(fp != NULL);
    }

    /* [2][3] 清除指定异常后，查询同一子集应返回 0（无已置位标志）。
     *        feclearexcept 清除 FE_INVALID|FE_OVERFLOW，
     *        fetestexcept 查询同一子集，结果应为 0。 */
    {
        int r;
        feclearexcept(FE_INVALID | FE_OVERFLOW);
        r = fetestexcept(FE_INVALID | FE_OVERFLOW);
        assert(r == 0);
    }

    /* [2][3] 查询一个未清除的、可能被置位的标志子集：
     *        返回值必须是 excepts 的子集（按位与后等于自身），
     *        即返回的位不会超出所查询的 excepts。 */
    {
        int r;
        r = fetestexcept(FE_INVALID | FE_OVERFLOW | FE_DIVBYZERO);
        /* 返回值只能包含 excepts 中的位 */
        assert((r & ~(FE_INVALID | FE_OVERFLOW | FE_DIVBYZERO)) == 0);
    }

    /* [2][3] 查询空子集（excepts == 0）应返回 0。 */
    {
        int r = fetestexcept(0);
        assert(r == 0);
    }

    /* [3] 返回值是「按位或」语义：查询单个标志时，
     *     返回值要么是 0，要么恰好等于该标志本身。 */
    {
        int r;
        feclearexcept(FE_DIVBYZERO);
        r = fetestexcept(FE_DIVBYZERO);
        assert(r == 0 || r == FE_DIVBYZERO);
    }

    /* [4] EXAMPLE：先清除 FE_INVALID|FE_OVERFLOW，
     *     再查询，按位测试决定调用 f 还是 g。
     *     此处清除后未主动触发异常，故两者都不应被调用。 */
    {
        int set_excepts;
        f_called = 0;
        g_called = 0;

        feclearexcept(FE_INVALID | FE_OVERFLOW);
        /* maybe raise exceptions —— 此处不主动触发 */
        set_excepts = fetestexcept(FE_INVALID | FE_OVERFLOW);
        if (set_excepts & FE_INVALID) f();
        if (set_excepts & FE_OVERFLOW) g();

        /* 清除后未触发异常，两个函数都不应被调用 */
        assert(f_called == 0);
        assert(g_called == 0);
    }

    /* [4] EXAMPLE 变体：主动触发一个浮点异常（除以零），
     *     验证 fetestexcept 能检测到 FE_DIVBYZERO 被置位。 */
    {
        volatile double zero = 0.0;
        volatile double one = 1.0;
        volatile double res;
        int set_excepts;

        feclearexcept(FE_ALL_EXCEPT);
        res = one / zero;          /* 触发 FE_DIVBYZERO */
        (void)res;

        set_excepts = fetestexcept(FE_DIVBYZERO);
        /* 若实现支持 FE_DIVBYZERO，则应检测到该标志 */
        assert(set_excepts & FE_DIVBYZERO);
    }

    /* Footnote 188：一次调用测试多个异常。
     * 清除全部异常后，查询多个标志的组合应返回 0。 */
    {
        int r;
        feclearexcept(FE_ALL_EXCEPT);
        r = fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
        assert(r == 0);
    }

    printf("C99 7.6.2.5 fetestexcept: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「fetestexcept 接受一个 int 类型参数」：
     * 传入结构体类型实参，gcc -std=c99 应报错
     * （incompatible type for argument / passing struct to int parameter）。 */
    {
        struct S { int x; } s;
        fetestexcept(s);
    }

    /* 违反约束「fetestexcept 接受一个 int 类型参数」：
     * 传入指针类型实参，gcc -std=c99 应报错
     * （incompatible type for argument）。 */
    {
        int *p = 0;
        fetestexcept(p);
    }

    /* 违反约束「fetestexcept 接受恰好一个参数」：
     * 不传参数，gcc -std=c99 应报错（too few arguments to function）。 */
    {
        fetestexcept();
    }

    /* 违反约束「fetestexcept 接受恰好一个参数」：
     * 传两个参数，gcc -std=c99 应报错（too many arguments to function）。 */
    {
        fetestexcept(FE_INVALID, FE_OVERFLOW);
    }

    /* 违反约束「fetestexcept 返回 int」：
     * 将返回值赋给结构体类型对象，gcc -std=c99 应报错
     * （incompatible types in assignment）。 */
    {
        struct S { int x; } s;
        s = fetestexcept(FE_INVALID);
    }

    /* 违反约束「调用函数前需有可见声明」：
     * 若未包含 <fenv.h>，则 fetestexcept 无声明，
     * 在 C99 中调用未声明函数为约束违反，gcc -std=c99 应报错
     * （implicit declaration of function 'fetestexcept'）。
     * 注意：本文件已包含 <fenv.h>，此片段仅示意。 */
    {
        /* 假设此处无 <fenv.h> 声明 */
        /* fetestexcept(FE_INVALID); */
    }

#endif
}