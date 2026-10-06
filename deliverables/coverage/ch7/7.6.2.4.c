/*
 * 测试 C99 7.6.2.4 —— fesetexceptflag 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 *
 * 覆盖段落：
 *   [1] 原型声明 #include <fenv.h>
 *       int fesetexceptflag(const fexcept_t *flagp, int excepts);
 *   [2] 语义：把 *flagp 中保存的异常状态写入 excepts 指定的状态标志；
 *       *flagp 必须由先前 fegetexceptflag 调用设置，且其第二实参
 *       至少覆盖 excepts 所代表的异常；本函数不引发浮点异常，只设置标志状态。
 *   [3] 返回值：excepts 为 0 或全部指定标志成功设置时返回 0，否则返回非零。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 便于在无 pragma 支持的实现上仍能编译；C99 允许实现不支持 FENV_ACCESS，
 * 但本测试只做“设置/读取状态标志”这类不依赖求值顺序的操作。 */
#pragma STDC FENV_ACCESS ON

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：函数可被声明/调用，参数类型为
     *     const fexcept_t * 与 int，返回 int。 */
    {
        int (*fp)(const fexcept_t *, int) = fesetexceptflag;
        assert(fp != NULL);
    }

    /* [3] excepts == 0 时必须返回 0（不设置任何标志）。 */
    {
        fexcept_t flag;
        int r = fesetexceptflag(&flag, 0);
        assert(r == 0);
    }

    /* [2][3] 正常路径：
     *   1) 用 fegetexceptflag 把当前状态保存到 fexcept_t 对象；
     *   2) 用 fesetexceptflag 把保存的状态写回；
     *   3) 返回值应为 0（全部指定标志成功设置）。
     * 这里只针对实现支持的异常宏进行测试。 */
    {
        fexcept_t saved;
        int excepts = 0;

#ifdef FE_DIVBYZERO
        excepts |= FE_DIVBYZERO;
#endif
#ifdef FE_INEXACT
        excepts |= FE_INEXACT;
#endif
#ifdef FE_INVALID
        excepts |= FE_INVALID;
#endif
#ifdef FE_OVERFLOW
        excepts |= FE_OVERFLOW;
#endif
#ifdef FE_UNDERFLOW
        excepts |= FE_UNDERFLOW;
#endif

        if (excepts != 0) {
            /* 先清标志，保证状态可预期 */
            feclearexcept(FE_ALL_EXCEPT);

            /* fegetexceptflag 的第二实参至少覆盖 excepts 所代表的异常 */
            int rg = fegetexceptflag(&saved, excepts);
            assert(rg == 0);

            /* 把保存的状态写回 */
            int rs = fesetexceptflag(&saved, excepts);
            assert(rs == 0);   /* [3] 全部成功 -> 0 */

            /* 读回验证：设置后的标志状态应与保存时一致 */
            fexcept_t now;
            int rn = fegetexceptflag(&now, excepts);
            assert(rn == 0);
            /* fexcept_t 是不透明类型，不能直接比较；通过再次设置并
             * 检查返回值来间接验证状态可被接受。 */
            int rs2 = fesetexceptflag(&now, excepts);
            assert(rs2 == 0);
        }
    }

    /* [2] “本函数不引发浮点异常，只设置标志状态”：
     *     调用 fesetexceptflag 本身不应产生新的异常。
     *     做法：清空所有标志 -> 调用 -> 检查标志未被“引发”。 */
    {
        fexcept_t saved;
        int excepts = 0;

#ifdef FE_DIVBYZERO
        excepts |= FE_DIVBYZERO;
#endif
#ifdef FE_INVALID
        excepts |= FE_INVALID;
#endif
#ifdef FE_OVERFLOW
        excepts |= FE_OVERFLOW;
#endif

        if (excepts != 0) {
            feclearexcept(FE_ALL_EXCEPT);
            int rg = fegetexceptflag(&saved, excepts);
            assert(rg == 0);

            feclearexcept(FE_ALL_EXCEPT);
            int rs = fesetexceptflag(&saved, excepts);
            assert(rs == 0);

            /* 由于 saved 是在清空后取得的，写回后标志应仍为“未置位”。
             * 用 fetestexcept 检查：不应有 excepts 中的异常被置位。 */
            int raised = fetestexcept(excepts);
            assert(raised == 0);
        }
    }

    /* [2] 只设置 excepts 指定的子集：
     *     保存全部异常状态，但只写回其中一个子集，返回值应为 0。 */
    {
        fexcept_t saved;
        int all = 0;

#ifdef FE_DIVBYZERO
        all |= FE_DIVBYZERO;
#endif
#ifdef FE_INEXACT
        all |= FE_INEXACT;
#endif
#ifdef FE_INVALID
        all |= FE_INVALID;
#endif
#ifdef FE_OVERFLOW
        all |= FE_OVERFLOW;
#endif
#ifdef FE_UNDERFLOW
        all |= FE_UNDERFLOW;
#endif

        if (all != 0) {
            feclearexcept(FE_ALL_EXCEPT);
            int rg = fegetexceptflag(&saved, all);
            assert(rg == 0);

            /* 取一个非零子集 */
            int subset = all & (-all);   /* 最低置位 */
            int rs = fesetexceptflag(&saved, subset);
            assert(rs == 0);
        }
    }

    printf("C99 7.6.2.4 fesetexceptflag: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fesetexceptflag 的第一个参数类型为 const fexcept_t *」：
 * 传入 int * 而非 fexcept_t *，gcc -std=c99 应报 incompatible pointer type。 */
void bad_arg_type(void)
{
    int x = 0;
    fesetexceptflag(&x, 0);   /* 期望报错：参数类型不兼容 */
}

/* 违反约束「fesetexceptflag 的第二个参数类型为 int」：
 * 传入指针，gcc -std=c99 应报 incompatible type for argument 2。 */
void bad_arg2_type(void)
{
    fexcept_t f;
    fesetexceptflag(&f, (int *)0);   /* 期望报错：参数类型不兼容 */
}

/* 违反约束「fesetexceptflag 返回 int」：
 * 把返回值赋给结构体，gcc -std=c99 应报 incompatible types。 */
struct S { int a; };
void bad_return_use(void)
{
    fexcept_t f;
    struct S s;
    s = fesetexceptflag(&f, 0);   /* 期望报错：int 不能赋给 struct S */
}

/* 违反约束「fesetexceptflag 需要两个实参」：
 * 少传实参，gcc -std=c99 应报 too few arguments。 */
void bad_too_few_args(void)
{
    fexcept_t f;
    fesetexceptflag(&f);   /* 期望报错：实参个数不足 */
}

/* 违反约束「fesetexceptflag 需要两个实参」：
 * 多传实参，gcc -std=c99 应报 too many arguments。 */
void bad_too_many_args(void)
{
    fexcept_t f;
    fesetexceptflag(&f, 0, 0);   /* 期望报错：实参个数过多 */
}

/* 违反约束「fesetexceptflag 的第一个参数为指向 const fexcept_t 的指针」：
 * 传入非指针（整数），gcc -std=c99 应报 incompatible type。 */
void bad_arg_not_pointer(void)
{
    fesetexceptflag(0, 0);   /* 期望报错：0 不是 fexcept_t * */
}

#endif /* 负向测试结束 */