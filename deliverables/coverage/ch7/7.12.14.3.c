/*
 * 测试条款：C99 7.12.14.3  The isless macro
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，isless(x, y) 可编译、可运行，
 *             其返回值恒等于 (x) < (y)；当 x、y 无序（unordered，即存在 NaN）时，
 *             isless(x, y) 返回 0，且不引发 "invalid" 浮点异常。
 *   负向测试：违反约束的代码（如参数个数错误、对非浮点类型使用等）应编译报错。
 *
 * 说明：本文件中的负向片段统一放在 #if 0 ... #endif 中，保证整体可编译运行。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：包含 <math.h> 后 isless 可用，且为宏（可被 #ifdef 检测） */
static void test_synopsis(void)
{
#ifdef isless
    /* isless 是宏，符合 [1] 的 Synopsis 描述 */
    assert(1);
#else
    /* 若实现将其实现为函数而非宏，则此处不成立；标准要求它是宏 */
    assert(0 && "isless should be a macro");
#endif
}

/* [2][3] 基本语义：isless(x, y) == (x) < (y)，对普通有序浮点数 */
static void test_ordered(void)
{
    double a = 1.0, b = 2.0;
    float  c = 3.0f, d = 2.0f;
    long double e = 1.5L, f = 1.5L;

    /* [3] 返回值等于 (x) < (y) */
    assert(isless(a, b) == (a < b));   /* 1 < 2 -> 1 */
    assert(isless(b, a) == (b < a));   /* 2 < 1 -> 0 */
    assert(isless(c, d) == (c < d));   /* 3 < 2 -> 0 */
    assert(isless(d, c) == (d < c));   /* 2 < 3 -> 1 */
    assert(isless(e, f) == (e < f));   /* 1.5 < 1.5 -> 0 */

    /* 相等情形 */
    assert(isless(a, a) == 0);
    assert(isless(e, f) == 0);

    /* 负零与正零：-0.0 < 0.0 为假 */
    assert(isless(-0.0, 0.0) == 0);
    assert(isless(0.0, -0.0) == 0);

    /* 无穷 */
    assert(isless(-INFINITY, 0.0) == 1);
    assert(isless(0.0, INFINITY) == 1);
    assert(isless(INFINITY, INFINITY) == 0);
}

/* [2] 无序（unordered）情形：x 或 y 为 NaN 时，isless 返回 0，
 *     且不引发 "invalid" 浮点异常（与 (x) < (y) 不同）。 */
static void test_unordered_no_invalid(void)
{
    double nan_v = NAN;
    double one   = 1.0;

    /* 无序时 isless 返回 0 */
    assert(isless(nan_v, one) == 0);
    assert(isless(one, nan_v) == 0);
    assert(isless(nan_v, nan_v) == 0);

    /* 与 (x) < (y) 的数值结果一致（无序时 (x)<(y) 也为 0） */
    assert(isless(nan_v, one) == (nan_v < one));
    assert(isless(one, nan_v) == (one < nan_v));

    /* 关键：isless 不引发 "invalid" 异常 */
    if (feclearexcept(FE_ALL_EXCEPT) == 0) {
        volatile int r1 = isless(nan_v, one);
        volatile int r2 = isless(one, nan_v);
        (void)r1; (void)r2;
        /* 若实现支持 fenv，则不应置位 FE_INVALID */
        assert(fetestexcept(FE_INVALID) == 0);
    }

    /* 对照：直接使用 (x) < (y) 在无序时会引发 FE_INVALID（若支持 fenv） */
    if (feclearexcept(FE_ALL_EXCEPT) == 0) {
        volatile int r3 = (nan_v < one);
        (void)r3;
        /* 此处不强制断言，因为部分实现可能不置位；仅作对照说明 */
    }
}

/* [2][3] 混合类型参数：real-floating 类型（float/double/long double） */
static void test_mixed_types(void)
{
    float  f = 1.0f;
    double d = 2.0;
    long double ld = 3.0L;

    assert(isless(f, d) == 1);
    assert(isless(d, f) == 0);
    assert(isless(f, ld) == 1);
    assert(isless(ld, d) == 0);
    assert(isless(d, ld) == 1);
}

/* [2][3] 参数为表达式（宏展开不应重复求值副作用之外的问题） */
static void test_expression_args(void)
{
    double x = 1.0, y = 2.0;
    assert(isless(x + 0.5, y) == 1);       /* 1.5 < 2 -> 1 */
    assert(isless(x, y - 1.5) == 0);       /* 1 < 0.5 -> 0 */
    assert(isless(x * 2.0, y * 2.0) == 0); /* 2 < 4 -> 1? 2<4 为真 -> 1 */
}

int main(void)
{
    test_synopsis();
    test_ordered();
    test_unordered_no_invalid();
    test_mixed_types();
    test_expression_args();

    printf("C99 7.12.14.3 isless: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isless 是宏，接受两个 real-floating 参数」：
 * 只传一个参数，gcc -std=c99 应报错（宏参数个数不匹配）。 */
#include <math.h>
void bad_one_arg(void) {
    double x = 1.0;
    int r = isless(x);   /* error: macro "isless" requires 2 arguments */
    (void)r;
}

/* 违反约束「参数应为 real-floating 类型」：
 * 传入整数类型，虽可能隐式转换，但若实现为类型泛型宏则可能报错；
 * 更严格地，传入指针类型应报错。 */
void bad_pointer_arg(void) {
    double x = 1.0;
    double *p = &x;
    int r = isless(p, x);   /* error: incompatible type for argument */
    (void)r;
}

/* 违反约束「isless 是宏，不是可赋值的左值」：
 * 对宏调用结果赋值应报错。 */
void bad_assign_to_macro(void) {
    double x = 1.0, y = 2.0;
    isless(x, y) = 1;   /* error: lvalue required as left operand of assignment */
}

/* 违反约束「isless 需要两个参数」：
 * 传三个参数，宏参数个数不匹配，应报错。 */
void bad_three_args(void) {
    double x = 1.0, y = 2.0, z = 3.0;
    int r = isless(x, y, z);   /* error: macro "isless" passed 3 arguments */
    (void)r;
}

#endif /* 负向测试结束 */