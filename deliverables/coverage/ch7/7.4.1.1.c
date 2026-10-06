/*
 * 测试 C99 7.4.1.1 —— isalnum 函数
 *
 * 预期行为：
 *   正向测试：<ctype.h> 提供 int isalnum(int c)；对任意字符 c，
 *             isalnum(c) 为真当且仅当 isalpha(c) 或 isdigit(c) 为真。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如参数个数错误、缺少声明等）应编译报错，
 *             统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型：int isalnum(int c);  —— 通过取函数指针验证签名 */
static int (*fp_isalnum)(int) = isalnum;

/* [2] isalnum(c) 为真 <=> isalpha(c) 为真 或 isdigit(c) 为真 */
static void test_equivalence(void)
{
    int c;
    /* 覆盖全部 unsigned char 值域以及 EOF */
    for (c = 0; c <= 255; ++c) {
        int a = isalnum(c);
        int b = (isalpha(c) || isdigit(c));
        /* 两者必须同为真或同为假 */
        assert((a != 0) == (b != 0));
    }
    /* EOF 不是字母也不是数字 */
    assert(isalnum(EOF) == 0);
    assert((isalpha(EOF) || isdigit(EOF)) == 0);
}

/* [2] 具体字符的语义检查 */
static void test_specific_chars(void)
{
    /* 字母：isalnum 为真 */
    assert(isalnum('a') != 0);
    assert(isalnum('z') != 0);
    assert(isalnum('A') != 0);
    assert(isalnum('Z') != 0);

    /* 数字：isalnum 为真 */
    assert(isalnum('0') != 0);
    assert(isalnum('5') != 0);
    assert(isalnum('9') != 0);

    /* 既非字母也非数字：isalnum 为假 */
    assert(isalnum(' ') == 0);
    assert(isalnum('\t') == 0);
    assert(isalnum('\n') == 0);
    assert(isalnum('!') == 0);
    assert(isalnum('@') == 0);
    assert(isalnum('[') == 0);
    assert(isalnum('~') == 0);
    assert(isalnum('\0') == 0);
}

/* [1] 参数类型为 int，可接受 EOF 与 unsigned char 值 */
static void test_int_argument(void)
{
    assert(fp_isalnum == isalnum);   /* 函数指针签名匹配 */
    assert(fp_isalnum('a') != 0);
    assert(fp_isalnum('7') != 0);
    assert(fp_isalnum(' ') == 0);
    assert(fp_isalnum(EOF) == 0);
}

int main(void)
{
    test_equivalence();
    test_specific_chars();
    test_int_argument();

    printf("C99 7.4.1.1 isalnum: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isalnum 的原型为 int isalnum(int)」：
 * 实参个数多于原型声明的参数个数，gcc -std=c99 应报错
 *   error: too many arguments to function 'isalnum' */
#include <ctype.h>
int bad1 = isalnum('a', 'b');

/* 违反约束「isalnum 的原型为 int isalnum(int)」：
 * 实参个数少于原型声明的参数个数，gcc -std=c99 应报错
 *   error: too few arguments to function 'isalnum' */
int bad2 = isalnum();

/* 违反约束「调用函数必须在其作用域内有可见声明」：
 * 未包含 <ctype.h> 且未声明 isalnum，C99 不允许隐式函数声明，
 * gcc -std=c99 应报错
 *   error: implicit declaration of function 'isalnum' */
int bad3 = isalnum('a');

/* 违反约束「isalnum 返回 int，不能作为函数被再次调用」：
 * 对 int 结果使用函数调用运算符，gcc -std=c99 应报错
 *   error: called object is not a function or function pointer */
int bad4 = isalnum('a')('b');

#endif