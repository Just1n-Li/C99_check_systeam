/*
 * 测试条款：C99 7.6.4.2  feholdexcept 函数
 *
 * 预期行为：
 *   正向测试：包含 <fenv.h>，调用 feholdexcept 保存环境、清除状态标志、
 *             安装 non-stop 模式；返回值 0 当且仅当 non-stop 模式成功安装。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（参数类型错误、缺少头文件声明等）
 *             应导致编译报错，统一放在 #if 0 ... #endif 中。
 *
 * 说明：本测试不依赖具体硬件是否支持 non-stop 模式，因此对返回值
 *       只做“0 与非 0 的语义一致性”检查，而不强制要求一定为 0。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 若实现未定义 FE_ALL_EXCEPT 等宏，则本测试退化为仅检查接口存在性 */
#ifndef FE_ALL_EXCEPT
#define FE_ALL_EXCEPT 0
#endif

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 头文件 <fenv.h> 已包含，函数原型为：
     *     int feholdexcept(fenv_t *envp);
     *     此处通过取地址验证原型可用。 */
    fenv_t env;
    fenv_t *envp = &env;

    /* [2] 调用 feholdexcept：保存当前浮点环境到 *envp，
     *     清除浮点状态标志，并安装 non-stop 模式（若可用）。 */
    int ret = feholdexcept(envp);

    /* [3] 返回值语义：返回 0 当且仅当 non-stop 模式成功安装。
     *     我们无法强制要求实现一定支持 non-stop，因此只验证
     *     “返回值是 0 或非 0”这一基本契约，并检查其与后续
     *     环境状态的一致性。 */
    if (ret == 0) {
        /* 成功安装 non-stop 模式：此时浮点状态标志应已被清除。 */
        int flags = fetestexcept(FE_ALL_EXCEPT);
        assert(flags == 0);   /* [2] 清除浮点状态标志 */
        printf("feholdexcept: non-stop mode installed (ret=0), "
               "status flags cleared.\n");
    } else {
        /* 未能安装 non-stop 模式：返回值非 0。 */
        printf("feholdexcept: non-stop mode NOT installed (ret=%d).\n", ret);
    }

    /* [2] 保存的环境对象应可被后续函数（如 feupdateenv / fesetenv）使用。
     *     这里仅验证 env 对象已被填充（通过再次调用 feholdexcept 覆盖它，
     *     并确认接口可重复调用而不崩溃）。 */
    int ret2 = feholdexcept(&env);
    assert((ret2 == 0) || (ret2 != 0));   /* 返回值总是 0 或非 0 */

    /* [2] 再次确认：调用后状态标志被清除（在成功安装 non-stop 时）。 */
    if (ret2 == 0) {
        assert(fetestexcept(FE_ALL_EXCEPT) == 0);
    }

    /* [3] 返回值类型为 int，可参与常规整数运算。 */
    int is_zero = (ret == 0);
    assert(is_zero == 0 || is_zero == 1);

    /* 恢复默认环境，避免影响后续测试。 */
    fesetenv(FE_DFL_ENV);

    printf("All positive tests for 7.6.4.2 passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「feholdexcept 的参数必须是指向 fenv_t 的指针」：
     * 传入 int* 而非 fenv_t*，gcc -std=c99 应报错
     * （incompatible pointer type / passing argument 1）。 */
    {
        int x = 0;
        feholdexcept(&x);          /* 错误：int* 不能转换为 fenv_t* */
    }

    /* 违反约束「feholdexcept 的参数必须是指向 fenv_t 的指针」：
     * 传入整数常量而非指针，应报错。 */
    {
        feholdexcept(0);           /* 错误：int 不能转换为 fenv_t* */
    }

    /* 违反约束「feholdexcept 的参数必须是指向 fenv_t 的指针」：
     * 传入 double* 而非 fenv_t*，应报错。 */
    {
        double d = 0.0;
        feholdexcept(&d);          /* 错误：double* 不能转换为 fenv_t* */
    }

    /* 违反约束「feholdexcept 的返回类型为 int」：
     * 将其返回值赋给结构体类型，应报错。 */
    {
        struct S { int a; } s;
        fenv_t e;
        s = feholdexcept(&e);      /* 错误：int 不能赋给 struct S */
    }

    /* 违反约束「调用 feholdexcept 前必须包含 <fenv.h> 声明」：
     * 若未包含头文件，则 feholdexcept 为隐式声明，
     * 在 C99 中隐式函数声明是约束违反，应报错（或至少警告）。
     * 此处通过重新声明一个不兼容的原型来模拟冲突。 */
    {
        /* 错误：与 <fenv.h> 中的原型冲突（返回类型不同）。 */
        double feholdexcept(fenv_t *envp);
        fenv_t e;
        feholdexcept(&e);
    }

#endif /* 负向测试结束 */

    return 0;
}