/*
 * 测试条款：C99 7.12.9.2 The floor functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后调用 floor / floorf / floorl，
 *             验证它们返回“不大于 x 的最大整数值”（以浮点形式表示）。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数声明与原型
 *   [2] Description：计算不大于 x 的最大整数值
 *   [3] Returns：返回 [x]，以浮点形式表示
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证三个函数可用，且返回类型分别为 double/float/long double。
 *     通过赋值给对应类型并检查类型兼容性来间接验证原型存在。 */
static void test_synopsis(void)
{
    double      d = floor(2.5);      /* double floor(double) */
    float       f = floorf(2.5f);    /* float  floorf(float) */
    long double l = floorl(2.5L);    /* long double floorl(long double) */

    /* 若原型缺失，C99 下隐式声明会返回 int，赋值给浮点类型仍可编译，
     * 但下面的数值断言会失败，从而暴露问题。 */
    assert(d == 2.0);
    assert(f == 2.0f);
    assert(l == 2.0L);
}

/* [2] Description：floor 计算“不大于 x 的最大整数值”。
 *     对正数、负数、整数、零分别验证。 */
static void test_description(void)
{
    /* 正数非整数：floor(2.7) = 2 */
    assert(floor(2.7) == 2.0);
    /* 正数非整数：floor(2.0) = 2（本身是整数） */
    assert(floor(2.0) == 2.0);
    /* 负数非整数：floor(-2.3) = -3（不大于 -2.3 的最大整数是 -3） */
    assert(floor(-2.3) == -3.0);
    /* 负数整数：floor(-2.0) = -2 */
    assert(floor(-2.0) == -2.0);
    /* 零：floor(0.0) = 0 */
    assert(floor(0.0) == 0.0);
    /* 负零：floor(-0.0) = -0.0（数值上等于 0） */
    assert(floor(-0.0) == 0.0);
    /* 小数部分：floor(0.9) = 0 */
    assert(floor(0.9) == 0.0);
    /* 小数部分：floor(-0.9) = -1 */
    assert(floor(-0.9) == -1.0);

    /* floorf 同样语义 */
    assert(floorf(3.9f) == 3.0f);
    assert(floorf(-3.1f) == -4.0f);

    /* floorl 同样语义 */
    assert(floorl(3.9L) == 3.0L);
    assert(floorl(-3.1L) == -4.0L);
}

/* [3] Returns：返回 [x]，以浮点形式表示。
 *     验证返回值是浮点类型（不是整数类型），且等于数学上的 [x]。 */
static void test_returns(void)
{
    /* 返回值类型为浮点：用 1.0 / result 检查，若为整数类型会得到整数除法，
     * 但更直接的验证是检查结果与整数值的浮点表示相等。 */
    double r = floor(5.5);
    assert(r == 5.0);
    /* 验证是浮点：floor(5.5) 的返回值参与浮点运算 */
    assert(r / 2.0 == 2.5);

    /* 大整数附近：floor(1e15 + 0.5) 应等于 1e15 */
    double big = 1e15;
    assert(floor(big + 0.5) == big);

    /* 结果与 (double)(long long) 转换一致（在可表示范围内） */
    double x = 123.456;
    assert(floor(x) == (double)(long long)x);

    /* 负数的结果与截断不同，验证 floor 语义 */
    double y = -123.456;
    assert(floor(y) == -124.0);
    assert(floor(y) != (double)(long long)y); /* 截断为 -123 */

    /* floorf 返回 float */
    float rf = floorf(7.8f);
    assert(rf == 7.0f);

    /* floorl 返回 long double */
    long double rl = floorl(7.8L);
    assert(rl == 7.0L);
}

/* 边界与特殊值（在 C99 允许范围内，不依赖实现定义行为） */
static void test_edge_cases(void)
{
    /* 整数边界：刚好在整数上 */
    assert(floor(1.0) == 1.0);
    assert(floor(-1.0) == -1.0);

    /* 非常接近整数 */
    assert(floor(1.0 - DBL_EPSILON) == 0.0);
    assert(floor(-1.0 + DBL_EPSILON) == -1.0);

    /* 幂等性：floor(floor(x)) == floor(x) */
    double vals[] = { 3.7, -3.7, 0.0, 100.0, -100.0, 0.5, -0.5 };
    for (size_t i = 0; i < sizeof(vals)/sizeof(vals[0]); i++) {
        double v = vals[i];
        assert(floor(floor(v)) == floor(v));
    }
}

int main(void)
{
    test_synopsis();
    test_description();
    test_returns();
    test_edge_cases();

    printf("C99 7.12.9.2 floor functions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束「[1] Synopsis 要求 #include <math.h> 才能使用 floor 系列函数」：
 * 不包含 <math.h> 直接调用 floor，在 C99 中属于隐式函数声明，
 * 但 C99 已删除隐式声明，gcc -std=c99 -Werror=implicit-function-declaration 应报错。
 * 注意：部分编译器仅警告，严格模式下应报错。 */
double no_include_test(void)
{
    return floor(2.5);   /* 期望：error: implicit declaration of function 'floor' */
}

/* 违反约束「[1] 参数类型必须为 double/float/long double」：
 * 传入结构体类型，参数类型不匹配，应编译报错。 */
struct S { int x; };
double wrong_arg_type(struct S s)
{
    return floor(s);     /* 期望：error: incompatible type for argument 1 of 'floor' */
}

/* 违反约束「[1] 参数个数必须为 1」：
 * 传入两个参数，应编译报错。 */
double too_many_args(double a, double b)
{
    return floor(a, b);  /* 期望：error: too many arguments to function 'floor' */
}

/* 违反约束「[1] 参数个数必须为 1」：
 * 不传参数，应编译报错。 */
double no_args(void)
{
    return floor();      /* 期望：error: too few arguments to function 'floor' */
}

/* 违反约束「[1] floorf 参数必须为 float 兼容类型」：
 * 传入结构体，应编译报错。 */
float wrong_arg_type_f(struct S s)
{
    return floorf(s);    /* 期望：error: incompatible type for argument 1 of 'floorf' */
}

/* 违反约束「[1] floorl 参数必须为 long double 兼容类型」：
 * 传入结构体，应编译报错。 */
long double wrong_arg_type_l(struct S s)
{
    return floorl(s);    /* 期望：error: incompatible type for argument 1 of 'floorl' */
}

/* 违反约束「[1] 函数名必须正确」：
 * 拼写错误，应编译报错（未声明标识符）。 */
double wrong_name(double x)
{
    return flor(x);      /* 期望：error: implicit declaration of function 'flor' */
}

#endif /* 负向测试结束 */