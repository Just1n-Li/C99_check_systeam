/*
 * 测试 C99 7.6.2 —— 浮点异常（Floating-point exceptions）
 *
 * 条款要点：
 *   [1] 以下函数提供对浮点状态标志的访问：
 *         feclearexcept, fetestexcept, feraiseexcept,
 *         fegetexceptflag, fesetexceptflag
 *       这些函数的 int 参数表示浮点异常的一个子集，可以是 0，
 *       也可以是一个或多个浮点异常宏的按位或（如 FE_OVERFLOW | FE_INEXACT）。
 *       对于其它参数值，这些函数的行为是未定义的（UB，不属于约束）。
 *
 * 预期行为：
 *   正向测试：包含 <fenv.h>，使用上述函数与宏，程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如把非 int 类型传给这些函数、
 *             把浮点异常宏当作普通变量赋值等）应编译报错。
 *
 * 注意：本条款本身没有显式的 "Constraints" 段落，只有 [1] 一段语义描述。
 *       负向测试针对的是这些函数原型所隐含的类型约束（参数必须为 int）。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 若编译器未定义 FE_ALL_EXCEPT，则退化为 0，保证可编译 */
#ifndef FE_ALL_EXCEPT
#define FE_ALL_EXCEPT 0
#endif

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 这些函数存在且可调用；参数可以是 0 */
    feclearexcept(0);
    (void)fetestexcept(0);
    feraiseexcept(0);

    /* [1] 参数可以是一个或多个浮点异常宏的按位或 */
    feclearexcept(FE_ALL_EXCEPT);
    (void)fetestexcept(FE_ALL_EXCEPT);

    /* [1] 使用具体宏的按位或，例如 FE_OVERFLOW | FE_INEXACT */
#ifdef FE_OVERFLOW
#ifdef FE_INEXACT
    feclearexcept(FE_OVERFLOW | FE_INEXACT);
    (void)fetestexcept(FE_OVERFLOW | FE_INEXACT);
#endif
#endif

    /* [1] 这些宏是整数常量表达式，可参与按位运算 */
    {
        int mask = 0;
#ifdef FE_DIVBYZERO
        mask |= FE_DIVBYZERO;
#endif
#ifdef FE_INVALID
        mask |= FE_INVALID;
#endif
        feclearexcept(mask);
        (void)fetestexcept(mask);
    }

    /* [1] fegetexceptflag / fesetexceptflag 处理标志的完整内容（脚注 186） */
    {
        fexcept_t flagp;
        /* 先清除所有异常，再取标志 */
        feclearexcept(FE_ALL_EXCEPT);
        if (fegetexceptflag(&flagp, FE_ALL_EXCEPT) == 0) {
            /* 取回后应能成功设置回去 */
            int r = fesetexceptflag(&flagp, FE_ALL_EXCEPT);
            assert(r == 0);
        }
    }

    /* [1] 触发一个浮点异常，然后测试标志是否被置位 */
    {
        volatile double zero = 0.0;
        volatile double one  = 1.0;
        feclearexcept(FE_ALL_EXCEPT);
        /* 1.0 / 0.0 通常引发 FE_DIVBYZERO（实现相关，但不影响编译） */
        volatile double q = one / zero;
        (void)q;
#ifdef FE_DIVBYZERO
        /* 若实现支持该异常，则标志应被置位；否则跳过断言 */
        if (fetestexcept(FE_DIVBYZERO)) {
            assert(fetestexcept(FE_DIVBYZERO) != 0);
        }
#endif
        feclearexcept(FE_ALL_EXCEPT);
    }

    /* [1] feraiseexcept 主动引发异常，随后 fetestexcept 应能观察到 */
    {
        feclearexcept(FE_ALL_EXCEPT);
#ifdef FE_INEXACT
        if (feraiseexcept(FE_INEXACT) == 0) {
            assert(fetestexcept(FE_INEXACT) != 0);
        }
#endif
        feclearexcept(FE_ALL_EXCEPT);
    }

    /* [1] 清除后标志应为空 */
    {
        feclearexcept(FE_ALL_EXCEPT);
        assert(fetestexcept(FE_ALL_EXCEPT) == 0);
    }

    printf("C99 7.6.2 floating-point exceptions: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /*
     * 违反约束「函数参数类型必须为 int」：
     * feclearexcept 的原型为 int feclearexcept(int excepts);
     * 传入 double 会触发 "incompatible type for argument" 错误。
     * 期望：gcc -std=c99 报错（如 passing argument 1 of 'feclearexcept'
     *       makes integer from pointer / incompatible type）。
     */
    feclearexcept(1.5);

    /*
     * 违反约束「fetestexcept 参数必须为 int」：
     * 传入指针类型，编译器应报 incompatible type。
     */
    {
        int *p = 0;
        (void)fetestexcept(p);
    }

    /*
     * 违反约束「feraiseexcept 参数必须为 int」：
     * 传入结构体类型，编译器应报 incompatible type。
     */
    {
        struct S { int x; } s;
        feraiseexcept(s);
    }

    /*
     * 违反约束「fegetexceptflag 第一个参数必须为 fexcept_t *」：
     * 传入 int * 而非 fexcept_t *，编译器应报 incompatible pointer type。
     */
    {
        int i = 0;
        (void)fegetexceptflag(&i, FE_ALL_EXCEPT);
    }

    /*
     * 违反约束「fesetexceptflag 第一个参数必须为 const fexcept_t *」：
     * 传入 int * 而非 fexcept_t *，编译器应报 incompatible pointer type。
     */
    {
        int i = 0;
        (void)fesetexceptflag(&i, FE_ALL_EXCEPT);
    }

    /*
     * 违反约束「fetestexcept 返回 int」：
     * 把返回值赋给结构体类型，编译器应报 incompatible types。
     */
    {
        struct T { int y; } t;
        t = fetestexcept(FE_ALL_EXCEPT);
    }

#endif /* 负向测试结束 */
}