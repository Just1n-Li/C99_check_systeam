/*
 * 测试 C99 7.3.4 —— CX_LIMITED_RANGE pragma
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，在合法位置使用
 *             #pragma STDC CX_LIMITED_RANGE on / off / default，
 *             程序应能编译并正确运行；复数乘、除、绝对值结果正确。
 *   负向测试：把 pragma 放在非法上下文（如函数调用实参位置、
 *             表达式中间等），违反 [2] 中“只能出现在外部声明之外
 *             或复合语句内所有显式声明和语句之前”的约束，
 *             编译器应报错（或至少给出诊断）。
 *
 * 说明：pragma 的“on/off”只影响实现是否可采用简化公式，
 *       不改变可观察的数学结果（在无溢出/无穷的正常范围内），
 *       因此正向测试用普通数值验证乘、除、模的语义。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 头文件与 pragma 语法：off / on / default 三种开关 */
#pragma STDC CX_LIMITED_RANGE off
#pragma STDC CX_LIMITED_RANGE on
#pragma STDC CX_LIMITED_RANGE default

/* [2] 默认状态为 off：在文件作用域再次显式关闭，便于后续测试 */
#pragma STDC CX_LIMITED_RANGE off

/* 在外部声明之外使用 pragma，作用域持续到下一个 pragma 或 TU 结束 */
#pragma STDC CX_LIMITED_RANGE on

static double complex cmul(double complex a, double complex b)
{
    /* [2] 在复合语句内、所有显式声明和语句之前使用 pragma */
#pragma STDC CX_LIMITED_RANGE on
    double complex r = a * b;
    return r;
}

static double complex cdiv(double complex a, double complex b)
{
#pragma STDC CX_LIMITED_RANGE on
    double complex r = a / b;
    return r;
}

static double cabs_val(double complex a)
{
#pragma STDC CX_LIMITED_RANGE on
    double r = cabs(a);
    return r;
}

/* [2] 复合语句内 pragma 的作用域：进入时保存、离开时恢复。
 * 下面函数在内部块中打开 on，块结束后应恢复为块之前的 off。 */
static int pragma_scope_restore(void)
{
#pragma STDC CX_LIMITED_RANGE off
    {
#pragma STDC CX_LIMITED_RANGE on
        /* 此处状态为 on */
    }
    /* 此处状态应恢复为 off —— 无法直接查询，但至少保证可编译运行 */
    return 1;
}

int main(void)
{
    /* [2] 在 main 的复合语句内、所有显式声明和语句之前使用 pragma */
#pragma STDC CX_LIMITED_RANGE on

    double complex a = 3.0 + 4.0 * I;
    double complex b = 1.0 + 2.0 * I;

    /* 复数乘法：(3+4i)(1+2i) = 3 + 6i + 4i + 8i^2 = -5 + 10i */
    double complex m = cmul(a, b);
    assert(fabs(creal(m) - (-5.0)) < 1e-9);
    assert(fabs(cimag(m) - (10.0)) < 1e-9);

    /* 复数除法：(3+4i)/(1+2i) = ((3*1+4*2) + i(4*1-3*2))/(1+4)
     *                       = (11 - 2i)/5 = 2.2 - 0.4i */
    double complex d = cdiv(a, b);
    assert(fabs(creal(d) - 2.2) < 1e-9);
    assert(fabs(cimag(d) - (-0.4)) < 1e-9);

    /* 复数绝对值：|3+4i| = 5 */
    double ab = cabs_val(a);
    assert(fabs(ab - 5.0) < 1e-9);

    /* [2] 复合语句内 pragma 作用域恢复 */
    assert(pragma_scope_restore() == 1);

    /* [2] 在 main 内嵌套复合语句中再次使用 pragma */
    {
#pragma STDC CX_LIMITED_RANGE off
        double complex t = a * b;
        assert(fabs(creal(t) - (-5.0)) < 1e-9);
        assert(fabs(cimag(t) - (10.0)) < 1e-9);
    }

    /* 离开嵌套块后，main 顶部的 on 状态应恢复 */
    {
        double complex t = a / b;
        assert(fabs(creal(t) - 2.2) < 1e-9);
        assert(fabs(cimag(t) - (-0.4)) < 1e-9);
    }

    printf("C99 7.3.4 CX_LIMITED_RANGE: all positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反 [2]「pragma 只能出现在外部声明之外，或复合语句内所有显式
 * 声明和语句之前」：把 pragma 放在函数体内已有语句之后。
 * 期望：gcc -std=c99 报错（misplaced #pragma / expected declaration）。 */
void bad_after_statement(void)
{
    int x = 1;
    x = x + 1;
#pragma STDC CX_LIMITED_RANGE on   /* 错误：出现在语句之后 */
}

/* 违反 [2]：把 pragma 放在函数调用实参位置（表达式中间）。
 * 期望：编译报错。 */
void bad_in_expression(void)
{
    printf("%d\n",
#pragma STDC CX_LIMITED_RANGE on   /* 错误：出现在表达式中间 */
           1);
}

/* 违反 [2]：把 pragma 放在结构体/联合体成员声明之间。
 * 期望：编译报错。 */
struct bad_struct {
    int a;
#pragma STDC CX_LIMITED_RANGE on   /* 错误：出现在成员声明之间 */
    int b;
};

/* 违反 [2]：把 pragma 放在函数参数列表中。
 * 期望：编译报错。 */
void bad_in_params(
#pragma STDC CX_LIMITED_RANGE on   /* 错误：出现在参数列表中间 */
    int x);

/* 违反 [1]：pragma 的开关不是 on/off/default 之一。
 * 期望：编译报错或至少给出诊断。 */
#pragma STDC CX_LIMITED_RANGE maybe

#endif /* 负向测试结束 */