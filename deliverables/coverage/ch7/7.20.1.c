/*
 * 测试 C99 7.20.1 —— Numeric conversion functions (atof, atoi, atol, atoll)
 *
 * 条款原文：
 *   [1] The functions atof, atoi, atol, and atoll need not affect the value of
 *       the integer expression errno on an error. If the value of the result
 *       cannot be represented, the behavior is undefined.
 *
 * 预期行为：
 *   正向测试：atof/atoi/atol/atoll 的正常转换结果符合语义，程序编译并运行通过。
 *   负向测试：违反约束的代码（如参数类型错误、赋值给非左值等）应编译报错。
 *
 * 说明：本条款本身没有显式 "Constraints" 段落，只有 [1] 一段语义。
 *       负向测试针对的是这些函数原型所隐含的约束（参数须为字符串指针、
 *       返回类型不可被赋值等），以及 C99 通用约束。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] atof：字符串转 double */
static void test_atof(void)
{
    double d;

    d = atof("3.14159");
    assert(d > 3.14158 && d < 3.14160);

    d = atof("-2.5e3");
    assert(d == -2500.0);

    d = atof("  42");          /* 前导空白 */
    assert(d == 42.0);

    d = atof("abc");           /* 无法转换：返回 0.0 */
    assert(d == 0.0);

    d = atof("");              /* 空串：返回 0.0 */
    assert(d == 0.0);

    d = atof("1e10");
    assert(d == 1e10);
}

/* [1] atoi：字符串转 int */
static void test_atoi(void)
{
    int i;

    i = atoi("12345");
    assert(i == 12345);

    i = atoi("-678");
    assert(i == -678);

    i = atoi("+99");
    assert(i == 99);

    i = atoi("  7");           /* 前导空白 */
    assert(i == 7);

    i = atoi("12abc");         /* 部分转换 */
    assert(i == 12);

    i = atoi("xyz");           /* 无法转换：返回 0 */
    assert(i == 0);

    i = atoi("");
    assert(i == 0);
}

/* [1] atol：字符串转 long */
static void test_atol(void)
{
    long l;

    l = atol("1000000");
    assert(l == 1000000L);

    l = atol("-123456789");
    assert(l == -123456789L);

    l = atol("0");
    assert(l == 0L);

    l = atol("nope");
    assert(l == 0L);
}

/* [1] atoll：字符串转 long long（C99 新增） */
static void test_atoll(void)
{
    long long ll;

    ll = atoll("9000000000");
    assert(ll == 9000000000LL);

    ll = atoll("-9000000000");
    assert(ll == -9000000000LL);

    ll = atoll("0");
    assert(ll == 0LL);

    ll = atoll("bad");
    assert(ll == 0LL);
}

/* [1] errno 语义：条款说这些函数 "need not affect errno"。
 * 也就是说，标准不要求它们在出错时设置 errno，也不要求它们清除 errno。
 * 因此我们不能断言 errno 一定被修改或一定不被修改——两种行为都符合标准。
 * 这里只验证：调用这些函数不会导致程序崩溃，且 errno 的取值要么保持原样，
 * 要么被设为某个合法值（即不产生未定义行为）。 */
static void test_errno_semantics(void)
{
    int saved;

    errno = 0;
    (void)atoi("not-a-number");
    /* 标准允许 errno 保持 0，也允许被修改；两者都合法，不做强断言 */

    errno = 0;
    saved = errno;
    (void)atof("not-a-number");
    /* 同上：errno 可能被改也可能不被改 */

    /* 唯一可确定的是：函数返回了确定值（0），程序继续正常运行 */
    assert(atoi("not-a-number") == 0);
    assert(atof("not-a-number") == 0.0);

    (void)saved;
}

/* [1] 返回值类型检查：确保原型声明的返回类型正确 */
static void test_return_types(void)
{
    /* 这些赋值若类型不符，编译器会给出警告/错误 */
    double d = atof("1.5");
    int    i = atoi("1");
    long   l = atol("1");
    long long ll = atoll("1");

    assert(d == 1.5);
    assert(i == 1);
    assert(l == 1L);
    assert(ll == 1LL);
}

int main(void)
{
    test_atof();
    test_atoi();
    test_atol();
    test_atoll();
    test_errno_semantics();
    test_return_types();

    printf("C99 7.20.1 numeric conversion functions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「atof 的参数必须是指向字符串的指针（const char *）」：
 * 传入整数常量，gcc -std=c99 应报错（参数类型不兼容）。 */
double bad1 = atof(42);

/* 违反约束「atoi 的参数必须是指向字符串的指针」：
 * 传入 double，应报错。 */
int bad2 = atoi(3.14);

/* 违反约束「atol 的参数必须是指向字符串的指针」：
 * 传入结构体，应报错。 */
struct S { int x; } s;
long bad3 = atol(s);

/* 违反约束「atoll 的参数必须是指向字符串的指针」：
 * 传入指针数组元素类型不匹配，应报错。 */
int arr[3];
long long bad4 = atoll(arr);

/* 违反约束「函数调用结果不是左值，不能赋值」：
 * atoi 返回 int 值，是右值，不能作为赋值目标，应报错。 */
atoi("5") = 10;

/* 违反约束「函数调用结果不是左值，不能取地址」：
 * 对 atof 的返回值取地址，应报错。 */
double *p = &atof("1.0");

/* 违反约束「函数调用结果不是左值，不能自增」：
 * 对 atol 的返回值使用 ++，应报错。 */
++atol("1");

/* 违反约束「函数调用结果不是左值，不能作为复合赋值左操作数」：
 * 对 atoll 的返回值使用 +=，应报错。 */
atoll("1") += 5;

/* 违反约束「函数调用结果不是左值，不能作为 sizeof 之外的操作数参与取址」：
 * 对 atoi 返回值使用 & 取址，应报错。 */
int *q = &atoi("1");

/* 违反约束「函数调用结果不是左值，不能作为数组下标以外的左值」：
 * 对 atof 返回值使用 --，应报错。 */
--atof("1.0");

#endif /* 负向测试结束 */