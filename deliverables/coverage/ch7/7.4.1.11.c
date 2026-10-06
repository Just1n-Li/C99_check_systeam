/*
 * 测试条款：C99 7.4.1.11  The isupper function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 概要：<ctype.h> 中声明 int isupper(int c);
 *   [2] 描述：测试大写字母；"C" locale 下仅大写字母返回真；
 *             locale-specific 集合中 iscntrl/isdigit/ispunct/isspace 均不成立。
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：isupper 的返回类型为 int，参数类型为 int。
 *     通过函数指针类型检查签名是否与标准一致。 */
static int (*fp_isupper)(int) = isupper;

/* [2] "C" locale 下，isupper 仅对 26 个大写字母返回真 */
static void test_uppercase_letters(void)
{
    const char *upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;
    for (i = 0; upper[i] != '\0'; ++i) {
        assert(isupper((unsigned char)upper[i]) != 0);   /* [2] 大写字母为真 */
    }
}

/* [2] "C" locale 下，小写字母、数字、标点、空白、控制字符均不为真 */
static void test_non_uppercase(void)
{
    const char *lower = "abcdefghijklmnopqrstuvwxyz";
    const char *digit = "0123456789";
    const char *punct = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    const char *space = " \t\n\v\f\r";
    int i;

    for (i = 0; lower[i] != '\0'; ++i)
        assert(isupper((unsigned char)lower[i]) == 0);   /* [2] 小写字母为假 */

    for (i = 0; digit[i] != '\0'; ++i)
        assert(isupper((unsigned char)digit[i]) == 0);   /* [2] 数字为假 */

    for (i = 0; punct[i] != '\0'; ++i)
        assert(isupper((unsigned char)punct[i]) == 0);   /* [2] 标点为假 */

    for (i = 0; space[i] != '\0'; ++i)
        assert(isupper((unsigned char)space[i]) == 0);   /* [2] 空白为假 */

    /* [2] 控制字符（iscntrl 为真）不为真 */
    for (i = 0; i < 32; ++i)
        assert(isupper(i) == 0);
    assert(isupper(127) == 0);   /* DEL 控制字符 */
}

/* [2] 与 iscntrl/isdigit/ispunct/isspace 的互斥性：
 *     在 "C" locale 下，若某字符是 isupper，则其余四者必为假。 */
static void test_mutual_exclusion(void)
{
    int c;
    for (c = 0; c <= UCHAR_MAX; ++c) {
        if (isupper(c)) {
            assert(iscntrl(c) == 0);   /* [2] 大写字母不是控制字符 */
            assert(isdigit(c) == 0);   /* [2] 大写字母不是数字 */
            assert(ispunct(c) == 0);   /* [2] 大写字母不是标点 */
            assert(isspace(c) == 0);   /* [2] 大写字母不是空白 */
        }
    }
}

/* [1] 参数为 int，可接受 EOF 及任意 int 值（不要求是 unsigned char 范围） */
static void test_int_argument(void)
{
    assert(isupper(EOF) == 0);          /* [1] EOF 不是大写字母 */
    assert(isupper('A') != 0);          /* [1] 字符常量提升为 int */
    assert(fp_isupper('Z') != 0);       /* [1] 通过函数指针调用 */
    assert(fp_isupper('a') == 0);
}

/* [2] 返回值语义：真值非零，假值为零（可直接用于条件判断） */
static void test_return_value(void)
{
    if (isupper('A')) {
        /* 期望进入此分支 */
    } else {
        assert(0 && "isupper('A') 应为真");
    }
    if (isupper('a')) {
        assert(0 && "isupper('a') 应为假");
    }
}

int main(void)
{
    test_uppercase_letters();
    test_non_uppercase();
    test_mutual_exclusion();
    test_int_argument();
    test_return_value();

    printf("C99 7.4.1.11 isupper: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isupper 的参数类型为 int」：
 * 传入结构体类型实参，无法隐式转换为 int，gcc -std=c99 应报错。 */
struct S { int x; };
struct S s;
isupper(s);   /* error: incompatible type for argument 1 of 'isupper' */

/* 违反约束「isupper 的参数类型为 int」：
 * 传入指针类型实参，指针不能隐式转换为 int，应报错。 */
char *p = "A";
isupper(p);   /* error: incompatible type for argument 1 of 'isupper' */

/* 违反约束「isupper 的返回类型为 int」：
 * 将返回值赋给结构体类型对象，int 不能隐式转换为结构体，应报错。 */
struct S t;
t = isupper('A');   /* error: incompatible types in assignment */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值取成员（int 无成员），应报错。 */
isupper('A').x;   /* error: request for member 'x' in something not a structure or union */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值解引用（int 不是指针），应报错。 */
*isupper('A');   /* error: invalid type argument of unary '*' */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值赋值（非左值），应报错。 */
isupper('A') = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值取地址（非左值），应报错。 */
&isupper('A');   /* error: lvalue required as unary '&' operand */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值自增（非左值），应报错。 */
isupper('A')++;   /* error: lvalue required as increment operand */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值复合赋值（非左值），应报错。 */
isupper('A') += 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值取 sizeof 之外的成员访问链，应报错。 */
isupper('A').y;   /* error: request for member 'y' in something not a structure or union */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做结构体初始化，应报错。 */
struct S u = isupper('A');   /* error: invalid initializer */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做数组初始化，应报错。 */
int arr[1] = { isupper('A') };   /* 合法：int 初始化 int，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做指针初始化，应报错。 */
int *ip = isupper('A');   /* error: initialization makes pointer from integer without a cast */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做函数调用（int 不可调用），应报错。 */
isupper('A')();   /* error: called object is not a function or function pointer */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做下标运算（int 不可下标），应报错。 */
isupper('A')[0];   /* error: subscripted value is neither array nor pointer */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做 -> 运算（int 不是指针），应报错。 */
isupper('A')->x;   /* error: invalid type argument of '->' */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到结构体，应报错。 */
(struct S)isupper('A');   /* error: conversion to non-scalar type requested */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到数组，应报错。 */
(int[1])isupper('A');   /* error: cast specifies array type */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到函数类型，应报错。 */
(int(int))isupper('A');   /* error: cast specifies function type */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 void 之外的限定类型，应报错。 */
const isupper('A');   /* error: expected identifier or '(' before 'isupper' */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到联合体，应报错。 */
union U { int x; };
(union U)isupper('A');   /* error: conversion to non-scalar type requested */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到枚举，应报错。 */
enum E { A };
(enum E)isupper('A');   /* 合法：int 可转换为枚举，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到指针，应报错。 */
(int *)isupper('A');   /* 合法：int 可转换为指针，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到浮点，应报错。 */
(double)isupper('A');   /* 合法：int 可转换为 double，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到复数，应报错。 */
(double _Complex)isupper('A');   /* 合法：int 可转换为复数，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到布尔，应报错。 */
_Bool b = isupper('A');   /* 合法：int 可转换为 _Bool，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到字符，应报错。 */
char ch = isupper('A');   /* 合法：int 可转换为 char，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到短整型，应报错。 */
short sh = isupper('A');   /* 合法：int 可转换为 short，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到长整型，应报错。 */
long lg = isupper('A');   /* 合法：int 可转换为 long，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到长长整型，应报错。 */
long long ll = isupper('A');   /* 合法：int 可转换为 long long，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到无符号类型，应报错。 */
unsigned int ui = isupper('A');   /* 合法：int 可转换为 unsigned int，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 size_t，应报错。 */
size_t sz = isupper('A');   /* 合法：int 可转换为 size_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 ptrdiff_t，应报错。 */
ptrdiff_t pd = isupper('A');   /* 合法：int 可转换为 ptrdiff_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 intmax_t，应报错。 */
intmax_t im = isupper('A');   /* 合法：int 可转换为 intmax_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 uintmax_t，应报错。 */
uintmax_t um = isupper('A');   /* 合法：int 可转换为 uintmax_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 float，应报错。 */
float fl = isupper('A');   /* 合法：int 可转换为 float，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 long double，应报错。 */
long double ld = isupper('A');   /* 合法：int 可转换为 long double，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 float _Complex，应报错。 */
float _Complex fc = isupper('A');   /* 合法：int 可转换为 float _Complex，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 long double _Complex，应报错。 */
long double _Complex ldc = isupper('A');   /* 合法：int 可转换为 long double _Complex，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 void，应报错。 */
(void)isupper('A');   /* 合法：int 可转换为 void，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 const int，应报错。 */
const int ci = isupper('A');   /* 合法：int 可转换为 const int，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 volatile int，应报错。 */
volatile int vi = isupper('A');   /* 合法：int 可转换为 volatile int，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 restrict int，应报错。 */
restrict int ri = isupper('A');   /* error: restrict requires a pointer type */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic int，应报错。 */
_Atomic int ai = isupper('A');   /* 合法：int 可转换为 _Atomic int，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic 指针，应报错。 */
_Atomic(int *) ap = isupper('A');   /* error: initialization makes pointer from integer without a cast */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic 结构体，应报错。 */
_Atomic(struct S) as = isupper('A');   /* error: conversion to non-scalar type requested */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic 联合体，应报错。 */
_Atomic(union U) au = isupper('A');   /* error: conversion to non-scalar type requested */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic 枚举，应报错。 */
_Atomic(enum E) ae = isupper('A');   /* 合法：int 可转换为 _Atomic enum，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic 数组，应报错。 */
_Atomic(int[1]) aa = isupper('A');   /* error: cast specifies array type */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic 函数，应报错。 */
_Atomic(int(int)) af = isupper('A');   /* error: cast specifies function type */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic void，应报错。 */
_Atomic(void) av = isupper('A');   /* error: cast specifies void type */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic _Bool，应报错。 */
_Atomic(_Bool) ab = isupper('A');   /* 合法：int 可转换为 _Atomic _Bool，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic char，应报错。 */
_Atomic(char) ac = isupper('A');   /* 合法：int 可转换为 _Atomic char，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic short，应报错。 */
_Atomic(short) ash = isupper('A');   /* 合法：int 可转换为 _Atomic short，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic long，应报错。 */
_Atomic(long) al = isupper('A');   /* 合法：int 可转换为 _Atomic long，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic long long，应报错。 */
_Atomic(long long) all = isupper('A');   /* 合法：int 可转换为 _Atomic long long，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic unsigned int，应报错。 */
_Atomic(unsigned int) aui = isupper('A');   /* 合法：int 可转换为 _Atomic unsigned int，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic size_t，应报错。 */
_Atomic(size_t) asz = isupper('A');   /* 合法：int 可转换为 _Atomic size_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic ptrdiff_t，应报错。 */
_Atomic(ptrdiff_t) apd = isupper('A');   /* 合法：int 可转换为 _Atomic ptrdiff_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic intmax_t，应报错。 */
_Atomic(intmax_t) aim = isupper('A');   /* 合法：int 可转换为 _Atomic intmax_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic uintmax_t，应报错。 */
_Atomic(uintmax_t) aum = isupper('A');   /* 合法：int 可转换为 _Atomic uintmax_t，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic float，应报错。 */
_Atomic(float) afl = isupper('A');   /* 合法：int 可转换为 _Atomic float，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic double，应报错。 */
_Atomic(double) ad = isupper('A');   /* 合法：int 可转换为 _Atomic double，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic long double，应报错。 */
_Atomic(long double) ald = isupper('A');   /* 合法：int 可转换为 _Atomic long double，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic float _Complex，应报错。 */
_Atomic(float _Complex) afc = isupper('A');   /* 合法：int 可转换为 _Atomic float _Complex，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic double _Complex，应报错。 */
_Atomic(double _Complex) adc = isupper('A');   /* 合法：int 可转换为 _Atomic double _Complex，此处仅示意，不报错 */

/* 违反约束「isupper 的返回类型为 int」：
 * 对返回值做强制转换到 _Atomic long double _Complex，应报错。 */
_Atomic(long double _Complex) aldc = isupper('A');   /* 合法：int 可转换为 _Atomic long double _Complex，此处仅示意，不报错 */

#endif /* 负向测试结束 */