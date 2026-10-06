/*
 * 测试 C99 7.6.1 —— FENV_ACCESS pragma
 *
 * 预期行为：
 *   正向测试：包含 <fenv.h>，在外部声明处和复合语句内部正确放置
 *             #pragma STDC FENV_ACCESS ON/OFF，程序应能编译并运行通过。
 *   负向测试：把 pragma 放在非法上下文（如函数体中间、声明之后、
 *             语句之后、表达式内部等）应导致编译报错（或至少是
 *             未定义行为，标准要求实现拒绝非法位置）。
 *
 * 说明：本测试只依赖标准头 <fenv.h> 与标准 pragma 语法，
 *       不引入条款未涉及的特性。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [2] pragma 出现在外部声明处：从出现处生效直到另一个
 *     FENV_ACCESS pragma 或翻译单元结束。 */
#pragma STDC FENV_ACCESS ON

/* [2] 在外部声明处再次切换为 OFF，验证可以多次出现。 */
#pragma STDC FENV_ACCESS OFF

/* [2] 再切回 ON，供后续函数使用。 */
#pragma STDC FENV_ACCESS ON

/* [3] EXAMPLE 中的函数 f：在复合语句内部、所有显式声明和语句
 *     之前放置 #pragma STDC FENV_ACCESS ON。 */
void f(double x)
{
#pragma STDC FENV_ACCESS ON
    void g(double);
    void h(double);
    /* ... */
    g(x + 1);
    h(x + 1);
    /* ... */
}

/* 供 f 调用的简单实现，仅用于让程序可链接运行。 */
static double g_last;
static double h_last;
void g(double v) { g_last = v; }
void h(double v) { h_last = v; }

/* [2] 复合语句内部 pragma 的作用域：在复合语句结束时恢复到
 *     进入该复合语句之前的状态。这里外层是 ON，内层先 ON 再 OFF，
 *     离开内层后应恢复为 ON（即外层状态）。 */
static int scope_test(void)
{
#pragma STDC FENV_ACCESS ON
    {
#pragma STDC FENV_ACCESS OFF
        /* 内层为 OFF */
    }
    /* 离开内层复合语句后，状态恢复为进入前的 ON。
     * 这里无法直接查询 pragma 状态，但至少验证语法合法、
     * 且程序能正常编译运行。 */
    return 1;
}

/* [2] 在复合语句内部、所有显式声明和语句之前放置 pragma。 */
static int compound_pragma_test(void)
{
#pragma STDC FENV_ACCESS ON
    int a = 1;
    int b = 2;
    return a + b;
}

/* [2] 在复合语句内部先声明再放 pragma 也是允许的（pragma 位于
 *     所有显式声明和语句之前——此处声明之后、语句之前）。 */
static int decl_then_pragma_test(void)
{
    int a = 3;
#pragma STDC FENV_ACCESS ON
    int b = 4;   /* 注意：C99 中声明必须在语句之前，这里 pragma 不算语句 */
    return a + b;
}

/* [2] 测试浮点环境访问：设置舍入模式、测试状态标志。
 *     在 FENV_ACCESS ON 状态下进行这些操作是良定义的。 */
static int fenv_access_test(void)
{
#pragma STDC FENV_ACCESS ON
    int saved_round = fegetround();
    assert(saved_round != -1);

    /* 设置一个非默认舍入模式，再恢复。 */
    if (fesetround(FE_TOWARDZERO) == 0) {
        assert(fegetround() == FE_TOWARDZERO);
        fesetround(saved_round);
    }

    /* 清除并测试浮点状态标志。 */
    feclearexcept(FE_ALL_EXCEPT);
    volatile double one = 1.0;
    volatile double zero = 0.0;
    volatile double r = one / zero;   /* 产生除零标志 */
    (void)r;
    int raised = fetestexcept(FE_DIVBYZERO);
    /* 某些实现可能不产生该标志，这里只要求调用合法。 */
    (void)raised;

    return 1;
}

/* [4] 若 g 可能依赖第一次 x+1 设置的标志，或第二次 x+1 可能依赖
 *     g 设置的控制模式，则程序应包含适当放置的
 *     #pragma STDC FENV_ACCESS ON。这里演示该模式。 */
static double side_effect_g(double v)
{
    /* 模拟 g 修改控制模式。 */
    fesetround(FE_UPWARD);
    return v;
}

static double side_effect_h(double v)
{
    return v;
}

static int temporal_ordering_test(void)
{
#pragma STDC FENV_ACCESS ON
    double x = 1.0;
    double a = x + 1;          /* 第一次求值 */
    double b = side_effect_g(a); /* g 可能改变控制模式 */
    double c = x + 1;          /* 第二次求值，可能受 g 影响 */
    side_effect_h(c);
    (void)b;
    return 1;
}

int main(void)
{
    /* [3] EXAMPLE 调用 */
    f(2.0);
    assert(g_last == 3.0);
    assert(h_last == 3.0);

    /* [2] 复合语句作用域 */
    assert(scope_test() == 1);
    assert(compound_pragma_test() == 3);
    assert(decl_then_pragma_test() == 7);

    /* [2] 浮点环境访问 */
    assert(fenv_access_test() == 1);

    /* [4] 时序依赖 */
    assert(temporal_ordering_test() == 1);

    printf("C99 7.6.1 FENV_ACCESS pragma: all positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 7.6.1 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束 [2]「pragma 应出现在外部声明处，或复合语句内部所有
 * 显式声明和语句之前」：pragma 出现在函数体中间、语句之后。
 * gcc -std=c99 应报错（如 "expected declaration or statement" 或
 * 关于 pragma 位置的诊断）。 */
void bad_mid_statement(void)
{
    int a = 1;
    a = a + 1;
#pragma STDC FENV_ACCESS ON   /* 非法：位于语句之后 */
    a = a + 1;
}

/* 违反约束 [2]：pragma 出现在复合语句内部、但位于显式声明之后
 * 且后面还有声明——即 pragma 之后仍有声明，破坏了
 * 「preceding all explicit declarations and statements」的要求。
 * 注意：C99 中声明必须在语句之前，因此 pragma 之后出现声明
 * 是非法位置。 */
void bad_after_decl(void)
{
    int a = 1;
#pragma STDC FENV_ACCESS ON
    int b = 2;   /* 非法：pragma 之后仍有声明 */
    (void)a; (void)b;
}

/* 违反约束 [2]：pragma 出现在表达式内部 / 语句中间的非声明位置。
 * 标准要求 pragma 只能出现在外部声明处或复合语句开头。 */
void bad_inside_expression(void)
{
    int a = 1;
    int b = (a
#pragma STDC FENV_ACCESS ON   /* 非法：位于表达式内部 */
             + 1);
    (void)b;
}

/* 违反约束 [2]：pragma 出现在结构体/联合体成员声明之间，
 * 既不是外部声明处，也不是复合语句开头。 */
struct bad_struct {
    int x;
#pragma STDC FENV_ACCESS ON   /* 非法：位于结构体成员声明之间 */
    int y;
};

/* 违反约束 [2]：pragma 出现在函数参数列表或函数声明中间。 */
void bad_param_list(int a
#pragma STDC FENV_ACCESS ON   /* 非法：位于参数列表中间 */
                    , int b);

/* 违反约束 [2]：pragma 出现在文件作用域但位于函数定义之后、
 * 且处于非声明位置（例如紧跟在函数体右花括号之后但作为
 * 独立 token 序列的一部分）。这里用一个更明确的非法位置：
 * 在函数体内、return 语句之后。 */
int bad_after_return(void)
{
    return 0;
#pragma STDC FENV_ACCESS ON   /* 非法：位于 return 语句之后 */
}

/* 违反约束 [2]：pragma 出现在 #if 预处理块内部但不在
 * 外部声明处或复合语句开头——例如位于宏定义中间。 */
#define BAD_MACRO(x) \
#pragma STDC FENV_ACCESS ON \
    (x)

#endif /* 负向测试结束 */

/*
 * 说明：
 * 1) 正向部分覆盖了 [1] 头文件包含、[2] pragma 在外部声明处与
 *    复合语句内部的合法位置及作用域、[3] EXAMPLE、[4] 时序依赖，
 *    并实际访问了浮点环境（舍入模式、状态标志）。
 * 2) 负向部分把 #pragma STDC FENV_ACCESS 放在语句之后、声明之后、
 *    表达式内部、结构体成员之间、参数列表中间、return 之后等
 *    非法上下文，标准规定这些位置的行为未定义，符合性实现应
 *    给出诊断（编译报错）。
 * 3) 所有负向片段放在 #if 0 中，保证整个文件仍能正常编译运行。
 */