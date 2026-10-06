/*
 * 测试条款：C99 7.4.1.7  The islower function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int islower(int c);  声明于 <ctype.h>
 *   [2] 语义：测试字符是否为小写字母，或为 locale 特定集合中
 *           使 iscntrl/isdigit/ispunct/isspace 均为假的字符；
 *           在 "C" locale 中，仅对小写字母返回真（见 5.2.1）。
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：islower 接受 int 参数并返回 int。
 *     通过取函数指针类型来静态验证原型签名。 */
static int (*islower_proto)(int) = islower;

int main(void)
{
    /* [1] 头文件 <ctype.h> 提供 islower 声明，且可正常调用。 */
    assert(islower_proto != NULL);

    /* [2] "C" locale 下，小写字母返回真。 */
    assert(islower('a') != 0);
    assert(islower('b') != 0);
    assert(islower('m') != 0);
    assert(islower('z') != 0);

    /* [2] "C" locale 下，大写字母返回假。 */
    assert(islower('A') == 0);
    assert(islower('Z') == 0);

    /* [2] 数字不是小写字母。 */
    assert(islower('0') == 0);
    assert(islower('9') == 0);

    /* [2] 标点不是小写字母。 */
    assert(islower('!') == 0);
    assert(islower('.') == 0);
    assert(islower('@') == 0);

    /* [2] 空白字符不是小写字母。 */
    assert(islower(' ') == 0);
    assert(islower('\t') == 0);
    assert(islower('\n') == 0);

    /* [2] 控制字符不是小写字母。 */
    assert(islower('\0') == 0);
    assert(islower('\a') == 0);
    assert(islower('\r') == 0);

    /* [2] 返回值语义：非零表示真，零表示假。
     *     标准只要求“返回真/假”，不要求具体非零值，
     *     因此只断言 != 0 / == 0。 */
    assert((islower('q') != 0) == 1);
    assert((islower('Q') == 0) == 1);

    /* [2] 参数为 EOF 时，islower 应返回假（EOF 不是小写字母）。 */
    assert(islower(EOF) == 0);

    /* [2] 参数必须是可表示为 unsigned char 的值或 EOF；
     *     这里用 unsigned char 范围内的值测试，行为有定义。 */
    {
        unsigned char uc = (unsigned char)'g';
        assert(islower((int)uc) != 0);
    }

    /* [2] 遍历所有小写字母，逐一验证。 */
    {
        int ch;
        for (ch = 'a'; ch <= 'z'; ++ch) {
            assert(islower(ch) != 0);
        }
    }

    /* [2] 遍历所有大写字母，逐一验证为假。 */
    {
        int ch;
        for (ch = 'A'; ch <= 'Z'; ++ch) {
            assert(islower(ch) == 0);
        }
    }

    /* [2] 遍历所有十进制数字字符，逐一验证为假。 */
    {
        int ch;
        for (ch = '0'; ch <= '9'; ++ch) {
            assert(islower(ch) == 0);
        }
    }

    /* [2] 与 isupper 互补性（在 "C" locale 下对字母成立）。 */
    {
        int ch;
        for (ch = 'a'; ch <= 'z'; ++ch) {
            assert(islower(ch) != 0);
            assert(isupper(ch) == 0);
        }
        for (ch = 'A'; ch <= 'Z'; ++ch) {
            assert(islower(ch) == 0);
            assert(isupper(ch) != 0);
        }
    }

    printf("正向测试全部通过：islower 符合 C99 7.4.1.7\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「islower 的原型为 int islower(int)」：
 * 用不兼容的函数指针类型初始化，gcc -std=c99 应报错
 * （incompatible pointer type / initialization from incompatible pointer type）。 */
int (*bad_proto)(double) = islower;

/* 违反约束「islower 接受一个 int 参数」：
 * 以错误参数个数调用，gcc -std=c99 应报错（too many arguments）。 */
int bad_call_arity = islower('a', 'b');

/* 违反约束「islower 返回 int」：
 * 把返回值当作结构体使用，gcc -std=c99 应报错。 */
struct S { int x; };
struct S bad_ret = islower('a');

/* 违反约束「islower 是函数，不是对象」：
 * 对函数名取地址后解引用赋值，gcc -std=c99 应报错。 */
int bad_assign = 0;
void bad_fn_assign(void) {
    *islower = bad_assign;   /* 对函数指示符赋值，应报错 */
}

/* 违反约束「islower 声明于 <ctype.h>」：
 * 若未包含 <ctype.h> 而直接调用，在 C99 中为隐式声明，
 * 但若同时以不兼容方式使用，gcc -std=c99 应报错。 */
int bad_no_header(void) {
    return islower_undeclared('a');  /* 未声明的标识符，应报错 */
}

#endif