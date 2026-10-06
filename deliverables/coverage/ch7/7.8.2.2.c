/*
 * 测试 C99 7.8.2.2 —— imaxdiv 函数
 *
 * 预期行为：
 *   正向测试：包含 <inttypes.h> 后调用 imaxdiv，返回 imaxdiv_t 结构，
 *             其成员 quot / rem 均为 intmax_t，分别等于 numer/denom 与 numer%denom，
 *             且满足 numer == quot*denom + rem。程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如缺少 <inttypes.h> 声明、参数类型错误、
 *             对返回结构体成员赋值等）应导致编译报错。
 */

#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <inttypes.h> 提供 imaxdiv 声明与 imaxdiv_t 类型 */
static void test_synopsis_and_type(void)
{
    /* [1] 函数原型：imaxdiv_t imaxdiv(intmax_t, intmax_t); */
    imaxdiv_t (*fp)(intmax_t, intmax_t) = imaxdiv;
    assert(fp != NULL);

    /* [3] 返回类型为 imaxdiv_t，其成员 quot、rem 均为 intmax_t */
    imaxdiv_t r = imaxdiv((intmax_t)10, (intmax_t)3);
    intmax_t q = r.quot;
    intmax_t m = r.rem;
    (void)q;
    (void)m;
}

/* [2] 一次运算同时计算 numer/denom 与 numer%denom */
/* [3] 返回结构体含 quot（商）与 rem（余数），均为 intmax_t */
static void test_basic_division(void)
{
    intmax_t numer = (intmax_t)17;
    intmax_t denom = (intmax_t)5;
    imaxdiv_t r = imaxdiv(numer, denom);

    assert(r.quot == (intmax_t)3);   /* 17 / 5 == 3 */
    assert(r.rem  == (intmax_t)2);   /* 17 % 5 == 2 */

    /* 恒等式：numer == quot * denom + rem */
    assert(numer == r.quot * denom + r.rem);
}

/* [2][3] 负数情形：C99 中除法向零截断，余数符号与被除数相同 */
static void test_negative_operands(void)
{
    intmax_t numer = (intmax_t)-17;
    intmax_t denom = (intmax_t)5;
    imaxdiv_t r = imaxdiv(numer, denom);

    assert(r.quot == (intmax_t)-3);  /* -17 / 5 == -3 */
    assert(r.rem  == (intmax_t)-2);  /* -17 % 5 == -2 */
    assert(numer == r.quot * denom + r.rem);

    numer = (intmax_t)17;
    denom = (intmax_t)-5;
    r = imaxdiv(numer, denom);
    assert(r.quot == (intmax_t)-3);  /* 17 / -5 == -3 */
    assert(r.rem  == (intmax_t)2);   /* 17 % -5 == 2 */
    assert(numer == r.quot * denom + r.rem);

    numer = (intmax_t)-17;
    denom = (intmax_t)-5;
    r = imaxdiv(numer, denom);
    assert(r.quot == (intmax_t)3);   /* -17 / -5 == 3 */
    assert(r.rem  == (intmax_t)-2);  /* -17 % -5 == -2 */
    assert(numer == r.quot * denom + r.rem);
}

/* [3] 成员类型为 intmax_t：可用 intmax_t 变量接收，且与 / 和 % 结果一致 */
static void test_member_types_and_consistency(void)
{
    intmax_t numer = (intmax_t)1234567890123LL;
    intmax_t denom = (intmax_t)1000;
    imaxdiv_t r = imaxdiv(numer, denom);

    intmax_t expected_q = numer / denom;
    intmax_t expected_m = numer % denom;

    assert(r.quot == expected_q);
    assert(r.rem  == expected_m);
    assert(numer == r.quot * denom + r.rem);
}

/* [3] 边界：denom 为 1、numer 为 0、numer 为 INTMAX_MAX 等可表示情形 */
static void test_boundary_representable(void)
{
    imaxdiv_t r;

    r = imaxdiv((intmax_t)0, (intmax_t)7);
    assert(r.quot == (intmax_t)0);
    assert(r.rem  == (intmax_t)0);

    r = imaxdiv((intmax_t)42, (intmax_t)1);
    assert(r.quot == (intmax_t)42);
    assert(r.rem  == (intmax_t)0);

    r = imaxdiv((intmax_t)-42, (intmax_t)1);
    assert(r.quot == (intmax_t)-42);
    assert(r.rem  == (intmax_t)0);

    /* INTMAX_MAX / 1 与 % 1 均可表示 */
    r = imaxdiv(INTMAX_MAX, (intmax_t)1);
    assert(r.quot == INTMAX_MAX);
    assert(r.rem  == (intmax_t)0);

    /* INTMAX_MIN / 1 与 % 1 均可表示 */
    r = imaxdiv(INTMAX_MIN, (intmax_t)1);
    assert(r.quot == INTMAX_MIN);
    assert(r.rem  == (intmax_t)0);
}

/* [3] 结构体成员顺序未规定（either order），但成员名固定为 quot / rem */
static void test_member_names(void)
{
    imaxdiv_t r = imaxdiv((intmax_t)100, (intmax_t)7);
    /* 通过成员名访问，不依赖成员在结构体中的物理顺序 */
    assert(r.quot == (intmax_t)14);
    assert(r.rem  == (intmax_t)2);
}

int main(void)
{
    test_synopsis_and_type();
    test_basic_division();
    test_negative_operands();
    test_member_types_and_consistency();
    test_boundary_representable();
    test_member_names();

    printf("C99 7.8.2.2 imaxdiv: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 函数原型要求两个参数均为 intmax_t」：
 * 传入结构体类型，gcc -std=c99 应报 incompatible type 错误。 */
struct NotInt { int x; };
void bad_arg_type(void)
{
    struct NotInt s = { 1 };
    imaxdiv_t r = imaxdiv(s, (intmax_t)2);  /* 错误：第一个实参不是 intmax_t */
    (void)r;
}

/* 违反约束「[1] 参数个数必须为 2」：
 * 只传一个实参，gcc -std=c99 应报 too few arguments 错误。 */
void bad_arg_count(void)
{
    imaxdiv_t r = imaxdiv((intmax_t)10);  /* 错误：缺少 denom 实参 */
    (void)r;
}

/* 违反约束「[3] 返回结构体成员 quot/rem 为 intmax_t 值，非左值不可赋值」：
 * 对函数返回结构体的成员赋值，gcc -std=c99 应报 lvalue required 错误。 */
void bad_assign_to_member(void)
{
    imaxdiv((intmax_t)10, (intmax_t)3).quot = (intmax_t)5;  /* 错误：非左值 */
}

/* 违反约束「[3] 返回值为结构体，不能直接当作整数使用」：
 * 把 imaxdiv_t 赋给 intmax_t，gcc -std=c99 应报 incompatible type 错误。 */
void bad_assign_struct_to_int(void)
{
    intmax_t x = imaxdiv((intmax_t)10, (intmax_t)3);  /* 错误：类型不兼容 */
    (void)x;
}

/* 违反约束「[1] 未包含 <inttypes.h> 时无 imaxdiv 声明」：
 * 若注释掉上面的 #include <inttypes.h>，调用 imaxdiv 在 C99 下
 * 应报 implicit declaration 错误（C99 取消隐式函数声明）。 */
void bad_missing_header(void)
{
    /* 假设未包含 <inttypes.h>：
     * imaxdiv_t r = imaxdiv((intmax_t)10, (intmax_t)3); */
}

#endif /* 负向测试结束 */