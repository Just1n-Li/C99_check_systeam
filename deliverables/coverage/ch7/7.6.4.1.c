/*
 * 测试条款：ISO/IEC 9899:1999 (C99) 7.6.4.1  fegetenv 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int fegetenv(fenv_t *envp);  需要 #include <fenv.h>
 *   [2] 语义：尝试把当前浮点环境存入 envp 所指对象
 *   [3] 返回：成功返回 0；否则返回非零值
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 若编译器未定义 __STDC_IEC_559__ 或未提供 fenv.h，本测试仍应能编译；
 * 这里直接使用标准头，符合 C99 7.6 的要求。 */

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型检查：fegetenv 接受 fenv_t * 并返回 int。
     *     通过取函数指针并赋值来静态验证签名。 */
    {
        int (*fp)(fenv_t *) = fegetenv;   /* [1] 原型匹配 */
        assert(fp != NULL);
    }

    /* [2][3] 基本调用：把当前浮点环境存入 fenv_t 对象，成功返回 0。 */
    {
        fenv_t env;
        int ret = fegetenv(&env);         /* [2] 存储当前浮点环境 */
        assert(ret == 0);                 /* [3] 成功返回 0 */
    }

    /* [2][3] 在修改浮点环境后再次获取，仍应成功返回 0，
     *        并且获取到的环境应能反映修改（用 feholdexcept/fesetenv 配合验证）。 */
    {
        fenv_t saved, fetched;
        int r1, r2, r3;

        /* 先保存一份环境 */
        r1 = fegetenv(&saved);
        assert(r1 == 0);

        /* 改变舍入方向（若实现支持） */
        (void)fesetround(FE_UPWARD);

        /* 获取修改后的环境 */
        r2 = fegetenv(&fetched);
        assert(r2 == 0);                  /* [3] 成功返回 0 */

        /* 恢复原环境 */
        r3 = fesetenv(&saved);
        assert(r3 == 0);

        /* 再次获取，应成功 */
        {
            fenv_t again;
            int r4 = fegetenv(&again);
            assert(r4 == 0);              /* [3] 成功返回 0 */
        }
    }

    /* [2][3] 连续多次调用，每次都应成功返回 0（幂等性检查）。 */
    {
        fenv_t e1, e2;
        int i;
        for (i = 0; i < 3; ++i) {
            assert(fegetenv(&e1) == 0);   /* [3] */
            assert(fegetenv(&e2) == 0);   /* [3] */
        }
    }

    /* [2] 存储结果可用于后续 fesetenv 恢复环境（语义闭环）。 */
    {
        fenv_t env;
        int r = fegetenv(&env);
        assert(r == 0);                   /* [3] */
        assert(fesetenv(&env) == 0);      /* 用获取的环境恢复，应成功 */
    }

    printf("fegetenv: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「[1] 参数类型必须为 fenv_t *」：
     * 传入 int * 而非 fenv_t *，gcc -std=c99 应报 incompatible pointer type 错误。 */
    {
        int x;
        fegetenv(&x);                     /* 期望报错：参数类型不匹配 */
    }

    /* 违反约束「[1] 参数个数必须为 1」：
     * 不传参数，gcc -std=c99 应报 too few arguments 错误。 */
    {
        fegetenv();                       /* 期望报错：参数太少 */
    }

    /* 违反约束「[1] 参数个数必须为 1」：
     * 传两个参数，gcc -std=c99 应报 too many arguments 错误。 */
    {
        fenv_t a, b;
        fegetenv(&a, &b);                 /* 期望报错：参数太多 */
    }

    /* 违反约束「[1] 返回值类型为 int，不可作为左值赋值」：
     * 对函数调用结果赋值，gcc -std=c99 应报 lvalue required 错误。 */
    {
        fenv_t env;
        fegetenv(&env) = 0;               /* 期望报错：非左值不能赋值 */
    }

    /* 违反约束「[1] 参数必须为对象指针，不能传整型常量」：
     * 传 0 作为指针，gcc -std=c99 应报 incompatible 类型错误。 */
    {
        fegetenv(0);                      /* 期望报错：0 不是 fenv_t * */
    }

#endif /* 负向测试结束 */
}