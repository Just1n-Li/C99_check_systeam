/*
 * 测试条款：C99 7.6.4.4  The feupdateenv function
 *
 * 预期行为：
 *   正向测试：包含 <fenv.h>，调用 feupdateenv，验证：
 *     [1] 原型 int feupdateenv(const fenv_t *envp);
 *     [2] 保存当前已引发的浮点异常 -> 安装 envp 指向的环境 -> 重新引发保存的异常；
 *         envp 必须来自 feholdexcept / fegetenv 或浮点环境宏。
 *     [3] 成功返回 0，否则返回非零。
 *     [4] EXAMPLE：隐藏伪下溢异常的用法。
 *   负向测试：违反约束（参数类型不匹配、向 const 指针写入等）应编译报错。
 *
 * 编译：gcc -std=c99 -Wall -Wextra -lm
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

#pragma STDC FENV_ACCESS ON

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与 int(const fenv_t *) 兼容 */
static int (*fp_feupdateenv)(const fenv_t *) = feupdateenv;

/* [4] EXAMPLE：隐藏伪下溢异常（按标准示例改写，去掉伪代码部分） */
static double f_hide_underflow(double x)
{
    double result;
    fenv_t save_env;

    /* 保存当前环境并清除所有异常标志，进入“无异常”计算状态 */
    if (feholdexcept(&save_env))
        return -1.0; /* 环境问题指示 */

    /* 计算 result（这里用一个可能产生下溢的运算） */
    result = x * 1e-300 * 1e-300;

    /* 测试伪下溢：若发生下溢则清除它 */
    if (fetestexcept(FE_UNDERFLOW)) {
        if (feclearexcept(FE_UNDERFLOW))
            return -1.0; /* 环境问题指示 */
    }

    /* 恢复原环境，并重新引发此前保存的异常 */
    if (feupdateenv(&save_env))
        return -1.0; /* 环境问题指示 */

    return result;
}

int main(void)
{
    /* [1] 原型可用性 */
    assert(fp_feupdateenv == feupdateenv);

    /* [2][3] 用 fegetenv 取得环境，再用 feupdateenv 安装，应返回 0 */
    {
        fenv_t env;
        int r = fegetenv(&env);
        assert(r == 0);
        r = feupdateenv(&env);
        assert(r == 0); /* [3] 成功返回 0 */
    }

    /* [2][3] 用 feholdexcept 取得环境，再用 feupdateenv 安装，应返回 0 */
    {
        fenv_t env;
        int r = feholdexcept(&env);
        assert(r == 0);
        r = feupdateenv(&env);
        assert(r == 0);
    }

    /* [2] 使用浮点环境宏作为参数（FE_DFL_ENV 是标准宏） */
    {
        int r = feupdateenv(FE_DFL_ENV);
        assert(r == 0);
    }

    /* [2] 语义验证：feupdateenv 应“重新引发”保存的异常。
     * 步骤：清除所有异常 -> 引发 FE_DIVBYZERO -> feholdexcept 保存并清除
     *       -> 此时异常标志为空 -> feupdateenv 恢复后应重新出现 FE_DIVBYZERO。
     */
    {
        fenv_t env;
        int r;

        r = feclearexcept(FE_ALL_EXCEPT);
        assert(r == 0);

        /* 人为引发除零异常 */
        volatile double zero = 0.0, one = 1.0;
        volatile double q = one / zero;
        (void)q;

        if (fetestexcept(FE_DIVBYZERO)) {
            /* 保存当前环境（含 FE_DIVBYZERO）并清除异常标志 */
            r = feholdexcept(&env);
            assert(r == 0);
            /* 此刻异常标志应已被清除 */
            assert(fetestexcept(FE_ALL_EXCEPT) == 0);

            /* feupdateenv 应恢复环境并重新引发保存的 FE_DIVBYZERO */
            r = feupdateenv(&env);
            assert(r == 0);
            assert(fetestexcept(FE_DIVBYZERO) != 0);
        }

        /* 清理，避免影响后续 */
        feclearexcept(FE_ALL_EXCEPT);
    }

    /* [4] EXAMPLE 函数可正常调用（结果值不强制断言，仅验证不返回错误指示） */
    {
        double y = f_hide_underflow(1.0);
        /* 正常路径下不应返回 -1.0 的错误指示 */
        assert(y != -1.0);
    }

    printf("C99 7.6.4.4 feupdateenv: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [1]：feupdateenv 的参数类型为 const fenv_t *，
 * 传入 int * 类型不兼容，gcc -std=c99 应报错（incompatible pointer type）。 */
void neg_wrong_arg_type(void)
{
    int x = 0;
    feupdateenv(&x); /* 期望报错：参数类型不匹配 */
}

/* 违反约束 [1]：参数个数错误，缺少实参，应报错。 */
void neg_missing_arg(void)
{
    feupdateenv(); /* 期望报错：too few arguments */
}

/* 违反约束 [1]：返回值被当作 void 使用（对 void 表达式赋值），应报错。 */
void neg_void_use(void)
{
    void *p = (void)feupdateenv(FE_DFL_ENV); /* 期望报错：void 值不能这样使用 */
    (void)p;
}

/* 违反约束 [1]：把 feupdateenv 的返回值赋给结构体，类型不兼容，应报错。 */
struct S { int a; };
void neg_assign_to_struct(void)
{
    struct S s;
    s = feupdateenv(FE_DFL_ENV); /* 期望报错：int 不能赋给 struct S */
}

/* 违反约束 [2]：envp 指向的对象被声明为 const，
 * 试图通过 feupdateenv 之外的途径修改它（这里演示向 const 对象写入），应报错。 */
void neg_write_const(void)
{
    const fenv_t env = {0};
    env = *(const fenv_t *)0; /* 期望报错：向 const 对象赋值 */
}

#endif /* 负向测试结束 */