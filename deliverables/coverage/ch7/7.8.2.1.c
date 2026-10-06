/*
 * 测试条款：ISO/IEC 9899:1999 (C99) 7.8.2.1 —— imaxabs 函数
 *
 * 预期行为：
 *   正向测试：包含 <inttypes.h>，调用 imaxabs 计算 intmax_t 的绝对值，
 *             结果应能编译并运行通过（assert 验证）。
 *   负向测试：违反约束的代码（如未包含头文件、参数类型错误、返回值类型误用等）
 *             应导致编译报错；这些片段放在 #if 0 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] Synopsis: #include <inttypes.h>  intmax_t imaxabs(intmax_t j);
 *   [2] Description: 计算整数 j 的绝对值；结果不可表示时行为未定义。
 *   [3] Returns: 返回绝对值。
 *   Footnote 193: 最负数的绝对值在二进制补码中不可表示（UB，非约束）。
 */

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

#include <inttypes.h>   /* [1] 必须包含此头文件才能使用 imaxabs */
#include <stdio.h>
#include <assert.h>
#include <limits.h>

int main(void)
{
    /* [1] 函数原型：intmax_t imaxabs(intmax_t j);
     * 验证返回类型为 intmax_t，参数类型为 intmax_t。 */
    intmax_t r;

    /* [3] 返回绝对值：正数 */
    r = imaxabs((intmax_t)42);
    assert(r == (intmax_t)42);
    printf("imaxabs(42) = %" PRIdMAX "\n", r);

    /* [3] 返回绝对值：负数 */
    r = imaxabs((intmax_t)-42);
    assert(r == (intmax_t)42);
    printf("imaxabs(-42) = %" PRIdMAX "\n", r);

    /* [3] 返回绝对值：零 */
    r = imaxabs((intmax_t)0);
    assert(r == (intmax_t)0);
    printf("imaxabs(0) = %" PRIdMAX "\n", r);

    /* [2][3] 使用 INTMAX_MAX 验证：最大值的绝对值就是它本身 */
    r = imaxabs(INTMAX_MAX);
    assert(r == INTMAX_MAX);
    printf("imaxabs(INTMAX_MAX) = %" PRIdMAX "\n", r);

    /* [2][3] 使用 INTMAX_MIN + 1 验证：避免 UB（最负数绝对值不可表示） */
    r = imaxabs(INTMAX_MIN + 1);
    assert(r == -(INTMAX_MIN + 1));
    printf("imaxabs(INTMAX_MIN+1) = %" PRIdMAX "\n", r);

    /* [1] 验证返回类型确实是 intmax_t（可用于 intmax_t 上下文） */
    {
        intmax_t v = imaxabs((intmax_t)-7);
        assert(v == (intmax_t)7);
    }

    /* [1] 验证参数类型为 intmax_t：传入 intmax_t 变量 */
    {
        intmax_t arg = (intmax_t)-123;
        intmax_t res = imaxabs(arg);
        assert(res == (intmax_t)123);
    }

    /* [3] 幂等性：abs(abs(x)) == abs(x) */
    {
        intmax_t x = (intmax_t)-999;
        assert(imaxabs(imaxabs(x)) == imaxabs(x));
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [1]：未包含 <inttypes.h>，imaxabs 未声明。
 * 期望：gcc -std=c99 报 "implicit declaration of function 'imaxabs'" 警告/错误
 *       （C99 中隐式函数声明为约束违反，-Werror 下报错）。 */
void test_no_header(void)
{
    intmax_t x = imaxabs((intmax_t)-5);   /* 未声明 */
    (void)x;
}

/* 违反约束 [1]：参数类型不匹配（传指针而非 intmax_t）。
 * 期望：编译报错 "incompatible type for argument 1 of 'imaxabs'"。 */
void test_wrong_arg_type(void)
{
    intmax_t v = 10;
    intmax_t *p = &v;
    intmax_t r = imaxabs(p);   /* 传指针，类型错误 */
    (void)r;
}

/* 违反约束 [1]：参数个数错误（多传一个参数）。
 * 期望：编译报错 "too many arguments to function 'imaxabs'"。 */
void test_too_many_args(void)
{
    intmax_t r = imaxabs((intmax_t)1, (intmax_t)2);
    (void)r;
}

/* 违反约束 [1]：参数个数错误（少传参数）。
 * 期望：编译报错 "too few arguments to function 'imaxabs'"。 */
void test_too_few_args(void)
{
    intmax_t r = imaxabs();
    (void)r;
}

/* 违反约束 [1]：将返回值赋给不兼容类型（结构体）。
 * 期望：编译报错 "incompatible types when assigning to type 'struct S'"。 */
struct S { int x; };
void test_wrong_return_use(void)
{
    struct S s;
    s = imaxabs((intmax_t)-1);   /* 返回值类型不兼容 */
    (void)s;
}

/* 违反约束 [1]：对函数返回值取地址（函数返回值非左值）。
 * 期望：编译报错 "lvalue required as unary '&' operand"。 */
void test_addr_of_return(void)
{
    intmax_t *p = &imaxabs((intmax_t)-1);
    (void)p;
}

/* 违反约束 [1]：对函数返回值赋值（非左值）。
 * 期望：编译报错 "lvalue required as left operand of assignment"。 */
void test_assign_to_return(void)
{
    imaxabs((intmax_t)-1) = (intmax_t)5;
}

#endif /* 负向测试结束 */