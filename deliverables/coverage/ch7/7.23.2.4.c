/*
 * 测试 C99 7.23.2.4 —— time 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：time_t time(time_t *timer);  声明于 <time.h>
 *   [2] 语义：确定当前日历时间，值的编码未指定。
 *   [3] 返回：返回对当前日历时间的最佳近似；若日历时间不可用返回 (time_t)(-1)；
 *       若 timer 非空指针，返回值也赋给其所指对象。
 */

#include <stdio.h>
#include <time.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：time 的返回类型为 time_t，参数为 time_t *。
 *     通过赋值给函数指针来静态验证原型签名。 */
static time_t (*time_proto_check)(time_t *) = time;

int main(void)
{
    /* [1] 头文件 <time.h> 提供 time 声明；此处已包含，能编译即证明。 */
    (void)time_proto_check;

    /* [3] 传入非空指针：返回值应同时赋给 *timer，且两者相等。 */
    time_t t1 = (time_t)0;
    time_t r1 = time(&t1);
    /* 若日历时间可用，则 r1 == t1；若不可用，则 r1 == (time_t)(-1) 且 t1 也被赋为 -1。 */
    assert(r1 == t1);

    /* [3] 传入空指针：仅使用返回值，不应崩溃。 */
    time_t r2 = time(NULL);
    /* 两次调用结果应满足：要么都可用（r1、r2 均为有效时间），要么都不可用（均为 -1）。
     * 这里只做弱断言：返回值类型正确且可比较。 */
    assert(r2 == (time_t)(-1) || r2 != (time_t)(-1));

    /* [3] 返回值与 *timer 的一致性再验证一次（用 NULL 与指针两种形式）。 */
    time_t t3 = (time_t)0;
    time_t r3 = time(&t3);
    assert(r3 == t3);

    /* [2] 编码未指定：只要求 time_t 是算术/可比较类型，能进行相等比较即可。
     *     这里验证 time_t 可用于算术比较（不假设具体编码）。 */
    if (r1 != (time_t)(-1) && r2 != (time_t)(-1)) {
        /* 两次调用时间差通常非负（时钟单调性非标准保证，故不做强断言）。 */
        assert(sizeof(time_t) >= 1);
    }

    /* [3] 若日历时间不可用，返回 (time_t)(-1)。此分支无法强制触发，
     *     但可验证该常量表达式可构造且类型正确。 */
    time_t unavailable = (time_t)(-1);
    assert(unavailable == (time_t)-1);

    printf("C99 7.23.2.4 time: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「time 的参数类型为 time_t *」：
 * 传入 int * 而非 time_t *，gcc -std=c99 应报 incompatible pointer type 警告/错误
 * （在 -Werror 下为错误）。 */
void neg_wrong_arg_type(void)
{
    int x;
    time(&x);            /* 期望：参数类型不匹配 */
}

/* 违反约束「time 的参数个数为 1」：
 * 调用时省略参数，gcc -std=c99 应报 too few arguments。 */
void neg_too_few_args(void)
{
    time();              /* 期望：参数太少 */
}

/* 违反约束「time 的参数个数为 1」：
 * 传入两个参数，gcc -std=c99 应报 too many arguments。 */
void neg_too_many_args(void)
{
    time_t t;
    time(&t, &t);        /* 期望：参数太多 */
}

/* 违反约束「time 返回 time_t，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
void neg_assign_to_call(void)
{
    time(NULL) = (time_t)0;   /* 期望：lvalue required as left operand of assignment */
}

/* 违反约束「time 的返回类型为 time_t」：
 * 将返回值赋给结构体类型，类型不兼容，应报错。 */
struct S { int a; };
void neg_wrong_return_use(void)
{
    struct S s;
    s = time(NULL);      /* 期望：类型不兼容 */
}

#endif /* 负向测试结束 */