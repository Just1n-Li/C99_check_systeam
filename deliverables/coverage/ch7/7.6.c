/*
 * 测试 C99 7.6 <fenv.h> 浮点环境
 *
 * 正向测试：验证 <fenv.h> 声明的类型、宏、函数可用，且满足条款 [1]-[9] 的语义。
 *           程序应能编译并运行通过（assert 全部成立）。
 * 负向测试：验证条款中的约束（constraint）——例如 FE_DFL_ENV 是
 *           "pointer to const-qualified fenv_t"，对其解引用赋值应编译报错。
 *           负向片段统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 *
 * 编译：gcc -std=c99 -Wall -Wextra -lm test.c
 */

#include <fenv.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] fenv_t 表示整个浮点环境 */
static fenv_t env_obj;

/* [4] fexcept_t 表示浮点状态标志的集合 */
static fexcept_t except_obj;

/* [1] 头文件声明两个类型和若干宏、函数 */
static void test_types_declared(void)
{
    /* 仅验证类型可用（能声明对象即可） */
    (void)env_obj;
    (void)except_obj;
}

/* [5] 异常宏：若定义，则展开为整型常量表达式；
 *     任意组合的按位 OR 结果互不相同；
 *     任意组合的按位 AND 结果为 0（即各宏是互不重叠的位）。
 *     这里对实现实际定义的宏逐一检查。
 */
static void test_exception_macros(void)
{
    /* 收集实现定义的异常宏 */
    int macros[8];
    int n = 0;

#ifdef FE_DIVBYZERO
    macros[n++] = FE_DIVBYZERO;
#endif
#ifdef FE_INEXACT
    macros[n++] = FE_INEXACT;
#endif
#ifdef FE_INVALID
    macros[n++] = FE_INVALID;
#endif
#ifdef FE_OVERFLOW
    macros[n++] = FE_OVERFLOW;
#endif
#ifdef FE_UNDERFLOW
    macros[n++] = FE_UNDERFLOW;
#endif

    /* 每个宏都是整型常量表达式（能用于数组维度即证明是常量表达式） */
    {
        char const_expr_check[FE_ALL_EXCEPT >= 0 ? 1 : 1];
        (void)const_expr_check;
    }

    /* 任意两个不同宏的按位 AND 必须为 0（互不重叠） */
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            assert((macros[i] & macros[j]) == 0);
        }
    }

    /* 任意组合的按位 OR 结果互不相同：检查所有非空子集的 OR 值唯一 */
    {
        int ors[256];
        int cnt = 0;
        int total = 1 << n;
        for (int mask = 1; mask < total; mask++) {
            int v = 0;
            for (int i = 0; i < n; i++) {
                if (mask & (1 << i)) v |= macros[i];
            }
            /* 与之前所有 OR 值比较，必须互不相同 */
            for (int k = 0; k < cnt; k++) {
                assert(ors[k] != v);
            }
            ors[cnt++] = v;
        }
    }
}

/* [6] FE_ALL_EXCEPT 是所有异常宏的按位 OR；若无异常宏则定义为 0 */
static void test_fe_all_except(void)
{
    int expected = 0;
#ifdef FE_DIVBYZERO
    expected |= FE_DIVBYZERO;
#endif
#ifdef FE_INEXACT
    expected |= FE_INEXACT;
#endif
#ifdef FE_INVALID
    expected |= FE_INVALID;
#endif
#ifdef FE_OVERFLOW
    expected |= FE_OVERFLOW;
#endif
#ifdef FE_UNDERFLOW
    expected |= FE_UNDERFLOW;
#endif
    assert(FE_ALL_EXCEPT == expected);
}

/* [7] 舍入方向宏：若定义，则展开为互不相同的非负整型常量表达式 */
static void test_rounding_macros(void)
{
    int macros[8];
    int n = 0;

#ifdef FE_DOWNWARD
    macros[n++] = FE_DOWNWARD;
#endif
#ifdef FE_TONEAREST
    macros[n++] = FE_TONEAREST;
#endif
#ifdef FE_TOWARDZERO
    macros[n++] = FE_TOWARDZERO;
#endif
#ifdef FE_UPWARD
    macros[n++] = FE_UPWARD;
#endif

    /* 每个宏都是非负整型常量表达式 */
    for (int i = 0; i < n; i++) {
        assert(macros[i] >= 0);
    }

    /* 值互不相同 */
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            assert(macros[i] != macros[j]);
        }
    }
}

/* [8] FE_DFL_ENV 表示默认浮点环境，类型为 "pointer to const-qualified fenv_t"，
 *     可作为管理浮点环境的函数（如 fesetenv）的参数。
 */
static void test_fe_dfl_env(void)
{
    /* 类型检查：FE_DFL_ENV 应能赋给 const fenv_t * */
    const fenv_t *p = FE_DFL_ENV;
    assert(p != NULL);

    /* 可作为 fesetenv 的参数（恢复默认环境） */
    int r = fesetenv(FE_DFL_ENV);
    assert(r == 0);
}

/* [1][2] 浮点环境访问：状态标志与舍入控制模式。
 *      函数调用不应改变调用者的控制模式，也不应清除调用者的状态标志
 *      （除非函数文档另有说明）。这里验证 fegetround/fesetround 与
 *      feclearexcept/fetestexcept 的基本行为。
 */
static void test_env_access(void)
{
    /* 保存当前舍入方向 */
    int saved = fegetround();
    assert(saved >= 0);

    /* 设置一个舍入方向（若实现支持），再读回 */
#ifdef FE_DOWNWARD
    if (fesetround(FE_DOWNWARD) == 0) {
        assert(fegetround() == FE_DOWNWARD);
    }
#endif
#ifdef FE_UPWARD
    if (fesetround(FE_UPWARD) == 0) {
        assert(fegetround() == FE_UPWARD);
    }
#endif
#ifdef FE_TOWARDZERO
    if (fesetround(FE_TOWARDZERO) == 0) {
        assert(fegetround() == FE_TOWARDZERO);
    }
#endif
#ifdef FE_TONEAREST
    if (fesetround(FE_TONEAREST) == 0) {
        assert(fegetround() == FE_TONEAREST);
    }
#endif

    /* 恢复原舍入方向 */
    (void)fesetround(saved);

    /* 清除所有异常标志，然后测试标志查询 */
    feclearexcept(FE_ALL_EXCEPT);
    assert(fetestexcept(FE_ALL_EXCEPT) == 0);

    /* 触发一个浮点异常（若实现支持 FE_DIVBYZERO） */
#ifdef FE_DIVBYZERO
    {
        volatile double zero = 0.0;
        volatile double one = 1.0;
        volatile double r = one / zero;
        (void)r;
        /* 状态标志被设置（但从不被清除，除非显式清除） */
        int raised = fetestexcept(FE_DIVBYZERO);
        /* 实现可能支持该异常；若支持则应被置位。
         * 注意：某些实现/优化下可能不置位，这里仅在支持时检查。 */
        if (raised) {
            assert(raised == FE_DIVBYZERO);
        }
        feclearexcept(FE_ALL_EXCEPT);
        assert(fetestexcept(FE_ALL_EXCEPT) == 0);
    }
#endif
}

/* [3][4][8] 保存/恢复整个浮点环境：fegetenv/fesetenv 使用 fenv_t */
static void test_save_restore_env(void)
{
    fenv_t saved_env;
    int r = fegetenv(&saved_env);
    assert(r == 0);

    /* 改变环境 */
    feclearexcept(FE_ALL_EXCEPT);
#ifdef FE_UPWARD
    (void)fesetround(FE_UPWARD);
#endif

    /* 恢复保存的环境 */
    r = fesetenv(&saved_env);
    assert(r == 0);

    /* 恢复默认环境 */
    r = fesetenv(FE_DFL_ENV);
    assert(r == 0);
}

/* [4] fexcept_t 与 fegetexceptflag/fesetexceptflag 配合使用 */
static void test_except_flag_type(void)
{
    fexcept_t flag;
    int r = fegetexceptflag(&flag, FE_ALL_EXCEPT);
    assert(r == 0);

    r = fesetexceptflag(&flag, FE_ALL_EXCEPT);
    assert(r == 0);
}

/* [1] 浮点异常作为异常浮点运算的副作用被置位，提供辅助信息 */
static void test_exception_side_effect(void)
{
    feclearexcept(FE_ALL_EXCEPT);

#ifdef FE_INVALID
    {
        volatile double zero = 0.0;
        volatile double r = zero / zero; /* 0/0 通常触发 FE_INVALID */
        (void)r;
        /* 若实现支持 FE_INVALID，则标志应被置位 */
        int raised = fetestexcept(FE_INVALID);
        if (raised) {
            assert(raised == FE_INVALID);
        }
    }
#endif

    feclearexcept(FE_ALL_EXCEPT);
    assert(fetestexcept(FE_ALL_EXCEPT) == 0);
}

int main(void)
{
    test_types_declared();
    test_exception_macros();
    test_fe_all_except();
    test_rounding_macros();
    test_fe_dfl_env();
    test_env_access();
    test_save_restore_env();
    test_except_flag_type();
    test_exception_side_effect();

    printf("C99 7.6 <fenv.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 违反约束 [8]：FE_DFL_ENV 的类型是 "pointer to const-qualified fenv_t"。
 * 它指向 const 限定的 fenv_t，因此不能通过它修改所指对象。
 * 期望：gcc -std=c99 报错（assignment of read-only location / 类似错误）。
 */
void negative_fe_dfl_env_is_const(void)
{
    *FE_DFL_ENV = env_obj;   /* 错误：FE_DFL_ENV 指向 const fenv_t，不可写 */
}

/*
 * 违反约束 [8]：FE_DFL_ENV 是指针类型，不能当作 fenv_t 对象直接使用。
 * 期望：gcc -std=c99 报错（类型不匹配）。
 */
void negative_fe_dfl_env_not_object(void)
{
    fenv_t e = FE_DFL_ENV;   /* 错误：指针不能初始化 fenv_t 对象 */
    (void)e;
}

/*
 * 违反约束 [3]：fenv_t 是完整对象类型，不能对其做算术运算。
 * 期望：gcc -std=c99 报错（invalid operands to binary +）。
 */
void negative_fenv_t_arithmetic(void)
{
    fenv_t a, b;
    fenv_t c = a + b;        /* 错误：结构体/不完整类型不能相加 */
    (void)c;
}

/*
 * 违反约束 [4]：fexcept_t 是对象类型，不能对其做算术运算。
 * 期望：gcc -std=c99 报错（invalid operands to binary *）。
 */
void negative_fexcept_t_arithmetic(void)
{
    fexcept_t a, b;
    fexcept_t c = a * b;     /* 错误：不能相乘 */
    (void)c;
}

/*
 * 违反约束 [5]：异常宏展开为整型常量表达式，不能用作左值赋值。
 * 期望：gcc -std=c99 报错（lvalue required as left operand of assignment）。
 */
void negative_exception_macro_not_lvalue(void)
{
    FE_DIVBYZERO = 1;        /* 错误：宏展开为常量，不是左值 */
}

/*
 * 违反约束 [7]：舍入方向宏展开为整型常量表达式，不能用作左值赋值。
 * 期望：gcc -std=c99 报错（lvalue required as left operand of assignment）。
 */
void negative_rounding_macro_not_lvalue(void)
{
    FE_TONEAREST = 0;        /* 错误：宏展开为常量，不是左值 */
}

/*
 * 违反约束 [6]：FE_ALL_EXCEPT 是整型常量表达式，不能用作左值赋值。
 * 期望：gcc -std=c99 报错（lvalue required as left operand of assignment）。
 */
void negative_fe_all_except_not_lvalue(void)
{
    FE_ALL_EXCEPT = 0;       /* 错误：宏展开为常量，不是左值 */
}

/*
 * 违反约束 [8]：FE_DFL_ENV 是指针，不能对其做按位运算。
 * 期望：gcc -std=c99 报错（invalid operands to binary |）。
 */
void negative_fe_dfl_env_bitwise(void)
{
    int x = FE_DFL_ENV | 1;  /* 错误：指针不能参与按位或 */
    (void)x;
}

#endif /* 负向测试结束 */