/*
 * 测试 C99 7.12.14.1 —— isgreater 宏
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，isgreater(x, y) 可用，其返回值恒等于 (x) > (y)；
 *             当 x、y 无序（NaN 参与）时，isgreater 不引发 "invalid" 浮点异常，
 *             而普通 (x) > (y) 会引发。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：
 *   [1] 概要：需要 <math.h>，原型 int isgreater(real-floating x, real-floating y);
 *   [2] 描述：判定第一个参数是否大于第二个；值恒等于 (x) > (y)；
 *             与 (x) > (y) 不同之处在于 x、y 无序时不引发 "invalid" 异常。
 *   [3] 返回值：返回 (x) > (y) 的值。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：isgreater 是 <math.h> 中声明的宏，接受 real-floating 实参。
 *     这里用 double / float / long double 三种实浮点类型调用，验证可用性。 */
static void test_synopsis(void)
{
    double dx = 3.0, dy = 2.0;
    float  fx = 3.0f, fy = 2.0f;
    long double lx = 3.0L, ly = 2.0L;

    /* [1] 宏可被调用，且返回 int 值 */
    int r1 = isgreater(dx, dy);
    int r2 = isgreater(fx, fy);
    int r3 = isgreater(lx, ly);

    assert(r1 == 1);
    assert(r2 == 1);
    assert(r3 == 1);
    printf("[1] isgreater 对 double/float/long double 均可调用: OK\n");
}

/* [2][3] 描述与返回值：isgreater(x, y) 的值恒等于 (x) > (y)。
 *     对有序的普通数值，逐一比较各种大小关系。 */
static void test_value_equals_gt(void)
{
    double vals[] = { -1.0, 0.0, 1.0, 2.5, 100.0 };
    size_t n = sizeof(vals) / sizeof(vals[0]);
    size_t i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int a = isgreater(vals[i], vals[j]);
            int b = (vals[i] > vals[j]);
            /* [3] 返回值恒等于 (x) > (y) */
            assert(a == b);
        }
    }
    printf("[2][3] isgreater(x,y) == (x)>(y) 对所有有序组合成立: OK\n");
}

/* [2] 描述：x、y 无序（NaN 参与）时，isgreater 不引发 "invalid" 浮点异常。
 *     对比 (x) > (y) 会引发 "invalid"。 */
static void test_no_invalid_exception(void)
{
    double nan_v = NAN;
    double one   = 1.0;
    int raised;

    /* 先清空浮点异常标志 */
    feclearexcept(FE_ALL_EXCEPT);

    /* [2] isgreater 在无序时不引发 invalid */
    (void)isgreater(nan_v, one);
    raised = fetestexcept(FE_INVALID);
    assert(raised == 0);
    printf("[2] isgreater(NaN, 1.0) 未引发 FE_INVALID: OK\n");

    /* 对照：普通 > 在无序时会引发 invalid */
    feclearexcept(FE_ALL_EXCEPT);
    (void)(nan_v > one);
    raised = fetestexcept(FE_INVALID);
    /* 若实现支持 FE_INVALID，则此处应被置位；否则跳过对照断言 */
    if (raised != 0) {
        printf("[2] 对照: (NaN > 1.0) 引发了 FE_INVALID: OK\n");
    } else {
        printf("[2] 对照: 本实现未报告 FE_INVALID（仍满足条款，仅对照）\n");
    }

    /* 无序时 isgreater 返回 0（因为 (x) > (y) 为假） */
    assert(isgreater(nan_v, one) == 0);
    assert(isgreater(one, nan_v) == 0);
    assert(isgreater(nan_v, nan_v) == 0);
    printf("[2][3] 无序时 isgreater 返回 0: OK\n");
}

/* [2] 描述：isgreater 的实参为 real-floating，允许混合类型（如 float 与 double），
 *     由通常算术转换处理。 */
static void test_mixed_types(void)
{
    float  f = 2.0f;
    double d = 3.0;

    assert(isgreater(d, f) == 1);
    assert(isgreater(f, d) == 0);
    printf("[2] 混合 float/double 实参: OK\n");
}

int main(void)
{
    test_synopsis();
    test_value_equals_gt();
    test_no_invalid_exception();
    test_mixed_types();

    printf("\n所有正向测试通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isgreater 的实参必须为 real-floating（实浮点类型）」：
 * 传入整数类型，gcc -std=c99 应报错（类型不匹配 / 隐式转换警告升级为错误）。
 * 期望：编译错误。 */
#include <math.h>
int bad_int_arg(void)
{
    int a = 1, b = 2;
    return isgreater(a, b);   /* 错误：实参不是实浮点类型 */
}

/* 违反约束「isgreater 接受两个实参」：
 * 只传一个实参，gcc -std=c99 应报错（宏参数个数不匹配）。
 * 期望：编译错误。 */
int bad_arg_count(void)
{
    double x = 1.0;
    return isgreater(x);      /* 错误：参数个数不足 */
}

/* 违反约束「isgreater 接受两个实参」：
 * 传三个实参，gcc -std=c99 应报错。
 * 期望：编译错误。 */
int bad_arg_count3(void)
{
    double x = 1.0, y = 2.0, z = 3.0;
    return isgreater(x, y, z); /* 错误：参数个数过多 */
}

/* 违反约束「isgreater 的实参必须为 real-floating」：
 * 传入指针类型，gcc -std=c99 应报错。
 * 期望：编译错误。 */
int bad_ptr_arg(void)
{
    double x = 1.0, y = 2.0;
    double *px = &x, *py = &y;
    return isgreater(px, py);  /* 错误：实参为指针而非实浮点 */
}

/* 违反约束「isgreater 的实参必须为 real-floating」：
 * 传入结构体类型，gcc -std=c99 应报错。
 * 期望：编译错误。 */
struct S { double v; };
int bad_struct_arg(void)
{
    struct S s1 = { 1.0 }, s2 = { 2.0 };
    return isgreater(s1, s2);  /* 错误：实参为结构体而非实浮点 */
}

#endif /* 负向测试结束 */