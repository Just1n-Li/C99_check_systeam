/*
 * 测试 C99 7.6.4.3 —— fesetenv 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int fesetenv(const fenv_t *envp);
 *   [2] 用 envp 指向的对象建立浮点环境；envp 必须指向由 fegetenv/feholdexcept
 *       设置的对象，或等于某个浮点环境宏；fesetenv 仅安装状态标志，不引发异常。
 *   [3] 成功返回 0，否则返回非零值。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 若编译器未定义 FE_DFL_ENV 等宏，则跳过相关测试，保证可移植编译 */
#pragma STDC FENV_ACCESS ON

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型检查：函数指针类型必须为 int (*)(const fenv_t *) */
    {
        int (*fp)(const fenv_t *) = fesetenv;
        assert(fp != NULL);
    }

    /* [2] 用 fegetenv 保存的环境对象调用 fesetenv，应成功返回 0 */
    {
        fenv_t env;
        int r_get = fegetenv(&env);
        assert(r_get == 0);                 /* fegetenv 成功 */
        int r_set = fesetenv(&env);         /* [3] 成功应返回 0 */
        assert(r_set == 0);
    }

    /* [2] 用 feholdexcept 保存的环境对象调用 fesetenv，应成功返回 0 */
    {
        fenv_t env;
        int r_hold = feholdexcept(&env);
        assert(r_hold == 0);
        int r_set = fesetenv(&env);
        assert(r_set == 0);
    }

    /* [2] envp 可等于浮点环境宏 FE_DFL_ENV，应成功返回 0 */
    {
        int r_set = fesetenv(FE_DFL_ENV);
        assert(r_set == 0);
    }

    /* [2] fesetenv 仅安装状态标志，不引发浮点异常：
     *     先制造一个被 raise 的异常标志，保存环境，清除标志，
     *     再用 fesetenv 恢复，检查标志被恢复（说明状态被安装），
     *     且过程中不应因 fesetenv 本身触发异常处理。 */
    {
        fenv_t env;
        int r;

        /* 清除所有异常标志 */
        r = feclearexcept(FE_ALL_EXCEPT);
        assert(r == 0);

        /* 手动 raise 一个异常标志（不触发陷阱，仅置标志） */
        r = feraiseexcept(FE_DIVBYZERO);
        assert(r == 0);

        /* 保存当前环境（含刚置上的标志） */
        r = fegetenv(&env);
        assert(r == 0);

        /* 清除标志，确认已清除 */
        r = feclearexcept(FE_ALL_EXCEPT);
        assert(r == 0);
        assert(fetestexcept(FE_DIVBYZERO) == 0);

        /* [2] 用 fesetenv 恢复环境：标志应被重新安装 */
        r = fesetenv(&env);
        assert(r == 0);                     /* [3] 成功返回 0 */
        assert(fetestexcept(FE_DIVBYZERO) != 0);  /* 标志已恢复 */

        /* 清理 */
        r = feclearexcept(FE_ALL_EXCEPT);
        assert(r == 0);
    }

    /* [2] 恢复默认环境后，环境应处于默认状态（标志清零） */
    {
        int r = fesetenv(FE_DFL_ENV);
        assert(r == 0);
        /* 默认环境下通常无异常标志；此处仅验证调用成功 */
    }

    printf("正向测试全部通过。\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束 [1]：fesetenv 的参数类型为 const fenv_t *，
     * 传入 int * 不兼容，gcc -std=c99 应报错
     * （incompatible pointer type / passing argument 1 ...）。 */
    {
        int x = 0;
        fesetenv(&x);
    }

    /* 违反约束 [1]：参数应为指针，传入整数常量应报错
     * （passing argument 1 ... makes pointer from integer）。 */
    {
        fesetenv(0);
    }

    /* 违反约束 [1]：参数个数错误（缺少实参），应报错
     * （too few arguments to function 'fesetenv'）。 */
    {
        fesetenv();
    }

    /* 违反约束 [1]：参数个数错误（多余实参），应报错
     * （too many arguments to function 'fesetenv'）。 */
    {
        fenv_t env;
        fesetenv(&env, &env);
    }

    /* 违反约束 [1]：返回值类型为 int，赋给不兼容的指针类型应报错
     * （assignment makes pointer from integer without a cast）。 */
    {
        fenv_t env;
        int *p = fesetenv(&env);
        (void)p;
    }

#endif

    return 0;
}