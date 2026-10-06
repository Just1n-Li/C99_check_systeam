/*
 * 测试 C99 7.6.2.2 —— fegetexceptflag 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（放在 #if 0 中，不参与编译）。
 *
 * 条款要点：
 *   [1] 原型：int fegetexceptflag(fexcept_t *flagp, int excepts);
 *   [2] 尝试把 excepts 指定的浮点状态标志的实现定义表示存入 *flagp。
 *   [3] 成功返回 0，否则返回非零值。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* 若编译器未定义 FE_ALL_EXCEPT 等宏，则本测试无法进行，给出提示 */
#ifndef FE_ALL_EXCEPT
#error "fenv.h 未定义 FE_ALL_EXCEPT，无法测试 7.6.2.2"
#endif

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 检查函数原型可用：取函数地址，类型应为 int(*)(fexcept_t *, int) */
static int (*fp)(fexcept_t *, int) = fegetexceptflag;

int main(void)
{
    fexcept_t flagp;
    int ret;

    /* [1] 原型存在且可调用 */
    (void)fp;

    /* [2][3] 正常调用：请求保存所有异常标志的表示，成功应返回 0 */
    ret = fegetexceptflag(&flagp, FE_ALL_EXCEPT);
    assert(ret == 0);

    /* [2][3] 请求保存单个异常标志（FE_DIVBYZERO），成功应返回 0 */
    ret = fegetexceptflag(&flagp, FE_DIVBYZERO);
    assert(ret == 0);

    /* [2][3] 请求保存 FE_INVALID，成功应返回 0 */
    ret = fegetexceptflag(&flagp, FE_INVALID);
    assert(ret == 0);

    /* [2][3] excepts 为 0（不请求任何标志），实现应能成功存储空表示，返回 0 */
    ret = fegetexceptflag(&flagp, 0);
    assert(ret == 0);

    /* [2] 先制造一个浮点异常，再保存其状态表示，仍应成功返回 0 */
    {
        volatile double zero = 0.0;
        volatile double one  = 1.0;
        volatile double r    = one / zero;   /* 触发 FE_DIVBYZERO */
        (void)r;
        ret = fegetexceptflag(&flagp, FE_DIVBYZERO);
        assert(ret == 0);
        feclearexcept(FE_ALL_EXCEPT);
    }

    /* [2] 保存后的 fexcept_t 对象可被 fesetexceptflag 使用（表示有效） */
    {
        fexcept_t saved;
        ret = fegetexceptflag(&saved, FE_ALL_EXCEPT);
        assert(ret == 0);
        /* 用保存的表示恢复标志，应成功返回 0 */
        ret = fesetexceptflag(&saved, FE_ALL_EXCEPT);
        assert(ret == 0);
        feclearexcept(FE_ALL_EXCEPT);
    }

    printf("7.6.2.2 fegetexceptflag: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fegetexceptflag 的第一个参数类型为 fexcept_t *」：
 * 传入 int * 而非 fexcept_t *，gcc -std=c99 应报 incompatible pointer type 错误。 */
void neg_wrong_first_arg(void)
{
    int x;
    fegetexceptflag(&x, FE_ALL_EXCEPT);   /* 期望：编译报错 */
}

/* 违反约束「fegetexceptflag 的第二个参数类型为 int」：
 * 传入 double 而非 int，gcc -std=c99 应报类型不匹配错误。 */
void neg_wrong_second_arg(void)
{
    fexcept_t f;
    fegetexceptflag(&f, 1.5);             /* 期望：编译报错 */
}

/* 违反约束「fegetexceptflag 需要两个实参」：
 * 只传一个实参，gcc -std=c99 应报 too few arguments 错误。 */
void neg_too_few_args(void)
{
    fexcept_t f;
    fegetexceptflag(&f);                  /* 期望：编译报错 */
}

/* 违反约束「fegetexceptflag 需要两个实参」：
 * 传三个实参，gcc -std=c99 应报 too many arguments 错误。 */
void neg_too_many_args(void)
{
    fexcept_t f;
    fegetexceptflag(&f, FE_ALL_EXCEPT, 0); /* 期望：编译报错 */
}

/* 违反约束「第一个参数必须是指针」：
 * 传入 fexcept_t 值而非指针，gcc -std=c99 应报类型不匹配错误。 */
void neg_non_pointer(void)
{
    fexcept_t f;
    fegetexceptflag(f, FE_ALL_EXCEPT);    /* 期望：编译报错 */
}

/* 违反约束「函数返回值类型为 int」：
 * 把返回值赋给结构体，gcc -std=c99 应报类型不兼容错误。 */
struct S { int a; };
void neg_bad_return_use(void)
{
    fexcept_t f;
    struct S s;
    s = fegetexceptflag(&f, FE_ALL_EXCEPT); /* 期望：编译报错 */
}

#endif /* 负向测试结束 */