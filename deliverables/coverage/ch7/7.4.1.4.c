/*
 * 测试条款：C99 7.4.1.4 —— iscntrl 函数
 *
 * 预期行为：
 *   正向测试：包含 <ctype.h> 后调用 iscntrl(int c)，对控制字符返回非零，
 *             对非控制字符返回 0；程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如参数个数错误、缺少 <ctype.h> 声明等）
 *             应导致编译报错，统一放在 #if 0 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] Synopsis: #include <ctype.h>  int iscntrl(int c);
 *   [2] Description: 测试任意控制字符。
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <ctype.h> 已包含，iscntrl 声明可见，返回类型为 int */
static int (*fp_iscntrl)(int) = iscntrl;   /* [1] 函数指针类型匹配 int(int) */

int main(void)
{
    /* [1] 原型：int iscntrl(int c); 参数为 int，返回 int */
    int r;

    /* [2] 控制字符：C 标准定义的控制字符集合。
     * 在 C locale 下，控制字符为 0x00–0x1F 以及 0x7F (DEL)。
     * 逐一对这些值验证 iscntrl 返回非零。 */
    for (int c = 0; c <= 0x1F; ++c) {
        r = iscntrl(c);
        assert(r != 0);                 /* [2] 控制字符 -> 非零 */
    }
    assert(iscntrl(0x7F) != 0);         /* [2] DEL 是控制字符 */

    /* [2] 非控制字符：可打印字符与空格应返回 0 */
    assert(iscntrl('A') == 0);
    assert(iscntrl('z') == 0);
    assert(iscntrl('0') == 0);
    assert(iscntrl(' ') == 0);          /* 空格是可打印字符，不是控制字符 */
    assert(iscntrl('~') == 0);
    assert(iscntrl('!') == 0);

    /* [1] 参数类型为 int：传入 int 值合法。
     * 传入 EOF 也是合法的（EOF 为负值，不是控制字符）。 */
    assert(iscntrl(EOF) == 0);

    /* [1] 通过函数指针调用，验证原型 int(int) */
    assert(fp_iscntrl('\n') != 0);      /* 换行是控制字符 */
    assert(fp_iscntrl('a') == 0);

    /* [2] 返回值语义：非零表示是控制字符，0 表示不是。
     * 标准只要求“非零”，不要求具体值，因此只判断 != 0 / == 0。 */
    r = iscntrl('\t');
    assert(r != 0);                     /* 制表符是控制字符 */

    /* [1] 参数为 int，可传入 char 提升后的值（char 提升为 int） */
    char ch = '\r';                     /* 回车，控制字符 */
    assert(iscntrl(ch) != 0);

    printf("C99 7.4.1.4 iscntrl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「iscntrl 原型为 int iscntrl(int)」：参数个数错误。
 * gcc -std=c99 应报错：too few arguments to function 'iscntrl' */
#include <ctype.h>
int bad1(void) { return iscntrl(); }

/* 违反约束「参数个数错误」：多传参数。
 * gcc -std=c99 应报错：too many arguments to function 'iscntrl' */
int bad2(void) { return iscntrl('a', 'b'); }

/* 违反约束「函数返回类型为 int，不能当作其他类型使用」：
 * 将 iscntrl 的返回值赋给不兼容的指针类型（int 到指针的隐式转换非法）。
 * gcc -std=c99 应报错：initialization of 'char *' from incompatible type 'int' */
int bad3(void) { char *p = iscntrl('a'); (void)p; return 0; }

/* 违反约束「调用函数前必须有可见声明」：未包含 <ctype.h> 且未声明 iscntrl。
 * 在 C99 中，隐式函数声明是约束违反（C99 6.5.2.2）。
 * gcc -std=c99 应报错：implicit declaration of function 'iscntrl' */
int bad4(void) { return iscntrl('a'); }

#endif