/*
 * 测试目标：C99 7.6.4 —— 浮点环境（floating-point environment）管理
 *
 * 条款原文：
 *   [1] The functions in this section manage the floating-point environment
 *       -- status flags and control modes -- as one entity.
 *
 * 预期行为：
 *   - 正向测试：<fenv.h> 中声明的函数（feclearexcept / fetestexcept /
 *     feraiseexcept / fegetenv / fesetenv / feholdexcept / feupdateenv /
 *     fegetround / fesetround 等）应能编译并正确运行，把「状态标志」与
 *     「控制模式」作为一个整体（fenv_t 实体）来管理。
 *   - 负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 说明：本测试只使用 C99 7.6 明确规定的接口，不引入其他特性。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 若编译器未定义 FE_ALL_EXCEPT / FE_TONEAREST 等宏，则视为不符合 7.6 */
#ifndef FE_ALL_EXCEPT
#error "C99 7.6: <fenv.h> 必须定义 FE_ALL_EXCEPT"
#endif
#ifndef FE_TONEAREST
#error "C99 7.6: <fenv.h> 必须定义 FE_TONEAREST"
#endif

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 状态标志（status flags）作为实体管理：
 *     feclearexcept 清除、feraiseexcept 置位、fetestexcept 查询。 */
static void test_status_flags(void)
{
    int raised;

    /* 先清空所有异常标志 */
    feclearexcept(FE_ALL_EXCEPT);

    /* 清除后不应有任何标志被置位 */
    raised = fetestexcept(FE_ALL_EXCEPT);
    assert(raised == 0);

    /* 人为置位 FE_DIVBYZERO（若实现支持该标志） */
#ifdef FE_DIVBYZERO
    feraiseexcept(FE_DIVBYZERO);
    raised = fetestexcept(FE_DIVBYZERO);
    assert(raised & FE_DIVBYZERO);

    /* 只清除 FE_DIVBYZERO，其他标志不受影响 */
    feclearexcept(FE_DIVBYZERO);
    raised = fetestexcept(FE_DIVBYZERO);
    assert((raised & FE_DIVBYZERO) == 0);
#endif

    /* 通过实际浮点运算产生标志（除以零） */
    feclearexcept(FE_ALL_EXCEPT);
    {
        volatile double zero = 0.0;
        volatile double one  = 1.0;
        volatile double r    = one / zero;   /* 产生除零标志 */
        (void)r;
    }
#ifdef FE_DIVBYZERO
    raised = fetestexcept(FE_DIVBYZERO);
    assert(raised & FE_DIVBYZERO);
#endif

    feclearexcept(FE_ALL_EXCEPT);
    printf("[1] status flags 管理正常\n");
}

/* [1] 控制模式（control modes）作为实体管理：
 *     fegetround / fesetround 读写舍入方向。 */
static void test_control_modes(void)
{
    int saved = fegetround();
    assert(saved == FE_TONEAREST || saved == FE_DOWNWARD ||
           saved == FE_UPWARD   || saved == FE_TOWARDZERO);

    /* 设置并读回每一种舍入模式 */
    assert(fesetround(FE_TONEAREST) == 0);
    assert(fegetround() == FE_TONEAREST);

    assert(fesetround(FE_DOWNWARD) == 0);
    assert(fegetround() == FE_DOWNWARD);

    assert(fesetround(FE_UPWARD) == 0);
    assert(fegetround() == FE_UPWARD);

    assert(fesetround(FE_TOWARDZERO) == 0);
    assert(fegetround() == FE_TOWARDZERO);

    /* 恢复原值 */
    fesetround(saved);
    printf("[1] control modes 管理正常\n");
}

/* [1] 把「状态标志 + 控制模式」作为一个整体实体（fenv_t）保存/恢复：
 *     fegetenv / fesetenv / feholdexcept / feupdateenv。 */
static void test_environment_as_entity(void)
{
    fenv_t env;
    int saved_round = fegetround();

    /* 保存整个环境 */
    assert(fegetenv(&env) == 0);

    /* 改变控制模式并置位标志 */
    fesetround(FE_UPWARD);
    feclearexcept(FE_ALL_EXCEPT);
#ifdef FE_INEXACT
    feraiseexcept(FE_INEXACT);
#endif

    /* 恢复整个环境：控制模式与状态标志应一并还原 */
    assert(fesetenv(&env) == 0);
    assert(fegetround() == saved_round);

    /* feholdexcept：保存环境并屏蔽所有异常，返回 0 */
    {
        fenv_t hold;
        assert(feholdexcept(&hold) == 0);
        /* 屏蔽后不应有标志被置位 */
        assert(fetestexcept(FE_ALL_EXCEPT) == 0);

        /* feupdateenv：恢复环境，同时把当前标志并入 */
        assert(feupdateenv(&hold) == 0);
    }

    /* FE_DFL_ENV：默认环境常量，可传给 fesetenv */
    assert(fesetenv(FE_DFL_ENV) == 0);

    fesetround(saved_round);
    printf("[1] 环境作为整体实体（fenv_t）管理正常\n");
}

int main(void)
{
    test_status_flags();
    test_control_modes();
    test_environment_as_entity();
    printf("正向测试全部通过：C99 7.6.4 浮点环境管理符合预期\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「feclearexcept 的参数必须为 int 类型的异常标志」：
 * 传入结构体类型，gcc -std=c99 应报错（incompatible type）。 */
struct NotAnInt { int x; };
struct NotAnInt bad;
feclearexcept(bad);

/* 违反约束「fesetround 的参数必须为 int 类型的舍入模式」：
 * 传入指针，gcc -std=c99 应报错。 */
int *p = 0;
fesetround(p);

/* 违反约束「fegetenv 的参数必须为 fenv_t *」：
 * 传入 int *，gcc -std=c99 应报错（incompatible pointer type）。 */
int i;
fegetenv(&i);

/* 违反约束「fetestexcept 返回 int，不能当作函数指针调用」：
 * 对返回值直接加括号调用，gcc -std=c99 应报错。 */
fetestexcept(FE_ALL_EXCEPT)();

/* 违反约束「fegetround 无参数」：
 * 传入多余实参，gcc -std=c99 应报错（too many arguments）。 */
fegetround(1);

/* 违反约束「fesetenv 需要恰好一个 fenv_t * 或 FE_DFL_ENV 实参」：
 * 不传实参，gcc -std=c99 应报错（too few arguments）。 */
fesetenv();

#endif