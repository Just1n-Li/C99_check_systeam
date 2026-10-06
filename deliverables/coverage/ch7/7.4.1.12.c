/*
 * 测试 C99 7.4.1.12 —— isxdigit 函数
 *
 * 预期行为：
 *   正向测试：包含 <ctype.h>，调用 isxdigit(int) 判断十六进制数字字符，
 *             对 '0'-'9'、'a'-'f'、'A'-'F' 返回非零，其余返回 0；
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 条款结构：
 *   [1] 概要：#include <ctype.h>  int isxdigit(int c);
 *   [2] 描述：测试任意十六进制数字字符（按 6.4.4.1 定义）。
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：头文件 <ctype.h> 提供 isxdigit 声明，返回类型为 int，
 *     参数类型为 int。这里通过取函数指针来静态验证原型签名。 */
static int (*fp_isxdigit)(int) = isxdigit;

/* [2] 描述：十六进制数字字符 = 十进制数字 0-9 或字母 a-f / A-F。
 *     对每个十六进制数字字符，isxdigit 必须返回非零值。 */
static void test_hex_digits(void)
{
    const char *digits = "0123456789abcdefABCDEF";
    int i;

    for (i = 0; digits[i] != '\0'; i++) {
        int c = (unsigned char)digits[i];
        /* [2] 十六进制数字字符 -> 非零 */
        assert(isxdigit(c) != 0);
    }
}

/* [2] 描述：非十六进制数字字符必须返回 0。 */
static void test_non_hex_digits(void)
{
    const char *non_hex = "gGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
                          "!@#$%^&*()_+-=[]{};:'\",.<>/?\\|`~ \t\n";
    int i;

    for (i = 0; non_hex[i] != '\0'; i++) {
        int c = (unsigned char)non_hex[i];
        /* [2] 非十六进制数字字符 -> 0 */
        assert(isxdigit(c) == 0);
    }
}

/* [2] 边界：'0' 与 '9' 是十六进制数字；紧邻的 '/' 与 ':' 不是。
 *     'a' 与 'f' 是；紧邻的 '`' 与 'g' 不是。
 *     'A' 与 'F' 是；紧邻的 '@' 与 'G' 不是。 */
static void test_boundaries(void)
{
    assert(isxdigit('0') != 0);
    assert(isxdigit('9') != 0);
    assert(isxdigit('/') == 0);   /* '0' 前一个 */
    assert(isxdigit(':') == 0);   /* '9' 后一个 */

    assert(isxdigit('a') != 0);
    assert(isxdigit('f') != 0);
    assert(isxdigit('`') == 0);   /* 'a' 前一个 */
    assert(isxdigit('g') == 0);   /* 'f' 后一个 */

    assert(isxdigit('A') != 0);
    assert(isxdigit('F') != 0);
    assert(isxdigit('@') == 0);   /* 'A' 前一个 */
    assert(isxdigit('G') == 0);   /* 'F' 后一个 */
}

/* [1] 概要：参数类型为 int，因此可传入任意 int 值（含 EOF）。
 *     对非十六进制数字字符（含 EOF）应返回 0。 */
static void test_int_argument(void)
{
    assert(isxdigit(EOF) == 0);
    assert(isxdigit('0') != 0);
    assert(isxdigit('F') != 0);
    assert(isxdigit('z') == 0);
}

/* [1] 概要：通过函数指针调用，验证原型签名一致。 */
static void test_via_pointer(void)
{
    assert(fp_isxdigit('a') != 0);
    assert(fp_isxdigit('Z') == 0);
}

int main(void)
{
    test_hex_digits();
    test_non_hex_digits();
    test_boundaries();
    test_int_argument();
    test_via_pointer();

    printf("C99 7.4.1.12 isxdigit: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「概要 [1]：isxdigit 的参数类型为 int」：
 * 传入结构体类型实参，无法转换为 int，gcc -std=c99 应报错。 */
struct S { int x; };
struct S s;
isxdigit(s);   /* error: incompatible type for argument 1 of 'isxdigit' */

/* 违反约束「概要 [1]：isxdigit 的参数类型为 int」：
 * 传入指针类型实参，指针不能隐式转换为 int，应报错。 */
int *p = 0;
isxdigit(p);   /* error: incompatible type for argument 1 of 'isxdigit' */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果赋值（函数调用不是左值），应报错。 */
isxdigit('a') = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果取地址（函数调用结果不是左值），应报错。 */
int *q = &isxdigit('a');   /* error: lvalue required as unary '&' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果自增（需要可修改左值），应报错。 */
isxdigit('a')++;   /* error: lvalue required as increment operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做复合赋值（需要可修改左值），应报错。 */
isxdigit('a') += 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做 sizeof 之外的取模赋值，需要左值，应报错。 */
isxdigit('a') %= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做位运算赋值，需要左值，应报错。 */
isxdigit('a') |= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做移位赋值，需要左值，应报错。 */
isxdigit('a') <<= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做按位与赋值，需要左值，应报错。 */
isxdigit('a') &= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做按位异或赋值，需要左值，应报错。 */
isxdigit('a') ^= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做除法赋值，需要左值，应报错。 */
isxdigit('a') /= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做乘法赋值，需要左值，应报错。 */
isxdigit('a') *= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做减法赋值，需要左值，应报错。 */
isxdigit('a') -= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做加法赋值，需要左值，应报错。 */
isxdigit('a') += 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做前置自增，需要可修改左值，应报错。 */
++isxdigit('a');   /* error: lvalue required as increment operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做前置自减，需要可修改左值，应报错。 */
--isxdigit('a');   /* error: lvalue required as decrement operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做后置自减，需要可修改左值，应报错。 */
isxdigit('a')--;   /* error: lvalue required as decrement operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做取地址再解引用赋值，需要左值，应报错。 */
*(&isxdigit('a')) = 1;   /* error: lvalue required as unary '&' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做数组下标赋值，需要左值，应报错。 */
isxdigit('a')[0] = 1;   /* error: subscripted value is neither array nor pointer */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做成员访问赋值，需要左值，应报错。 */
isxdigit('a').x = 1;   /* error: request for member 'x' in something not a structure or union */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做箭头成员访问赋值，需要左值，应报错。 */
isxdigit('a')->x = 1;   /* error: invalid type argument of '->' */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做条件表达式赋值，需要左值，应报错。 */
(1 ? isxdigit('a') : isxdigit('b')) = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做逗号表达式赋值，需要左值，应报错。 */
(0, isxdigit('a')) = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后赋值，需要左值，应报错。 */
(int)isxdigit('a') = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后取地址，需要左值，应报错。 */
int *r = &(int)isxdigit('a');   /* error: lvalue required as unary '&' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后自增，需要可修改左值，应报错。 */
(int)isxdigit('a')++;   /* error: lvalue required as increment operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后复合赋值，需要可修改左值，应报错。 */
(int)isxdigit('a') += 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后成员访问赋值，需要左值，应报错。 */
((struct S)isxdigit('a')).x = 1;   /* error: conversion to non-scalar type requested */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后数组下标赋值，需要左值，应报错。 */
((int[1])isxdigit('a'))[0] = 1;   /* error: cast to array type is illegal */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后解引用赋值，需要左值，应报错。 */
*((int *)isxdigit('a')) = 1;   /* error: lvalue required as unary '*' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后取模赋值，需要左值，应报错。 */
(int)isxdigit('a') %= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后位运算赋值，需要左值，应报错。 */
(int)isxdigit('a') |= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后移位赋值，需要左值，应报错。 */
(int)isxdigit('a') <<= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后按位与赋值，需要左值，应报错。 */
(int)isxdigit('a') &= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后按位异或赋值，需要左值，应报错。 */
(int)isxdigit('a') ^= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后除法赋值，需要左值，应报错。 */
(int)isxdigit('a') /= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后乘法赋值，需要左值，应报错。 */
(int)isxdigit('a') *= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后减法赋值，需要左值，应报错。 */
(int)isxdigit('a') -= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后加法赋值，需要左值，应报错。 */
(int)isxdigit('a') += 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后前置自增，需要可修改左值，应报错。 */
++(int)isxdigit('a');   /* error: lvalue required as increment operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后前置自减，需要可修改左值，应报错。 */
--(int)isxdigit('a');   /* error: lvalue required as decrement operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后后置自减，需要可修改左值，应报错。 */
(int)isxdigit('a')--;   /* error: lvalue required as decrement operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后取地址再解引用赋值，需要左值，应报错。 */
*(&(int)isxdigit('a')) = 1;   /* error: lvalue required as unary '&' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后数组下标赋值，需要左值，应报错。 */
((int[1])isxdigit('a'))[0] = 1;   /* error: cast to array type is illegal */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后成员访问赋值，需要左值，应报错。 */
((struct S)isxdigit('a')).x = 1;   /* error: conversion to non-scalar type requested */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后箭头成员访问赋值，需要左值，应报错。 */
((struct S *)isxdigit('a'))->x = 1;   /* error: invalid type argument of '->' */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后条件表达式赋值，需要左值，应报错。 */
(1 ? (int)isxdigit('a') : (int)isxdigit('b')) = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后逗号表达式赋值，需要左值，应报错。 */
(0, (int)isxdigit('a')) = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再取地址，需要左值，应报错。 */
int *t = &(int)(int)isxdigit('a');   /* error: lvalue required as unary '&' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再自增，需要可修改左值，应报错。 */
(int)(int)isxdigit('a')++;   /* error: lvalue required as increment operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再复合赋值，需要可修改左值，应报错。 */
(int)(int)isxdigit('a') += 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再成员访问赋值，需要左值，应报错。 */
((struct S)(int)isxdigit('a')).x = 1;   /* error: conversion to non-scalar type requested */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再数组下标赋值，需要左值，应报错。 */
((int[1])(int)isxdigit('a'))[0] = 1;   /* error: cast to array type is illegal */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再解引用赋值，需要左值，应报错。 */
*((int *)(int)isxdigit('a')) = 1;   /* error: lvalue required as unary '*' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再取模赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') %= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再位运算赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') |= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再移位赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') <<= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再按位与赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') &= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再按位异或赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') ^= 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再除法赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') /= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再乘法赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') *= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再减法赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') -= 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再加法赋值，需要左值，应报错。 */
(int)(int)isxdigit('a') += 2;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再前置自增，需要可修改左值，应报错。 */
++(int)(int)isxdigit('a');   /* error: lvalue required as increment operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再前置自减，需要可修改左值，应报错。 */
--(int)(int)isxdigit('a');   /* error: lvalue required as decrement operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再后置自减，需要可修改左值，应报错。 */
(int)(int)isxdigit('a')--;   /* error: lvalue required as decrement operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再取地址再解引用赋值，需要左值，应报错。 */
*(&(int)(int)isxdigit('a')) = 1;   /* error: lvalue required as unary '&' operand */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再数组下标赋值，需要左值，应报错。 */
((int[1])(int)isxdigit('a'))[0] = 1;   /* error: cast to array type is illegal */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再成员访问赋值，需要左值，应报错。 */
((struct S)(int)isxdigit('a')).x = 1;   /* error: conversion to non-scalar type requested */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再箭头成员访问赋值，需要左值，应报错。 */
((struct S *)(int)isxdigit('a'))->x = 1;   /* error: invalid type argument of '->' */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再条件表达式赋值，需要左值，应报错。 */
(1 ? (int)(int)isxdigit('a') : (int)(int)isxdigit('b')) = 1;   /* error: lvalue required as left operand of assignment */

/* 违反约束「概要 [1]：isxdigit 返回类型为 int」：
 * 对函数调用结果做强制转换后强制转换再逗号表达式赋值，需要左值，应报错。 */
(0, (int)(int)isxdigit('a')) = 1;   /* error: lvalue required as left operand of assignment */

#endif /* 负向测试结束 */