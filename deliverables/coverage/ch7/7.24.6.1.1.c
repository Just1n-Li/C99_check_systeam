/*
 * 测试 C99 7.24.6.1.1 —— btowc 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应被编译器拒绝（编译报错），
 *             这些片段统一放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 条款要点：
 *   [1] 原型：wint_t btowc(int c);  需要 <stdio.h> 与 <wchar.h>
 *   [2] 判断 c 在初始移位状态下是否构成有效的单字节字符
 *   [3] 若 c == EOF，或 (unsigned char)c 不是有效单字节字符，返回 WEOF；
 *       否则返回该字符的宽字符表示
 */

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：btowc 的返回类型为 wint_t，参数类型为 int。
 *     通过函数指针赋值来静态验证签名。 */
static wint_t (*btowc_sig_check)(int) = btowc;

int main(void)
{
    /* [1] 头文件与原型可用性：能取到函数地址即说明声明可见 */
    assert(btowc_sig_check == btowc);

    /* [3] c == EOF 时必须返回 WEOF */
    assert(btowc(EOF) == WEOF);

    /* [3] 对 ASCII 可打印字符，返回其宽字符表示（值等于字符本身） */
    assert(btowc('A') == (wint_t)L'A');
    assert(btowc('z') == (wint_t)L'z');
    assert(btowc('0') == (wint_t)L'0');
    assert(btowc(' ') == (wint_t)L' ');

    /* [3] 控制字符在初始移位状态下也是有效的单字节字符 */
    assert(btowc('\n') == (wint_t)L'\n');
    assert(btowc('\t') == (wint_t)L'\t');
    assert(btowc('\0') == (wint_t)L'\0');

    /* [2][3] 对 0..UCHAR_MAX 范围内的每个值，结果要么是 WEOF，
     *        要么是等于 (unsigned char)c 的宽字符。
     *        这验证了「有效单字节字符 -> 宽字符表示」的语义。 */
    {
        int c;
        for (c = 0; c <= UCHAR_MAX; ++c) {
            wint_t w = btowc(c);
            if (w != WEOF) {
                assert(w == (wint_t)(unsigned char)c);
            }
        }
    }

    /* [2][3] 参数按 int 传入，但语义上只看低 8 位（(unsigned char)c）。
     *        对 ASCII 字符，c 与 c + 256 的低字节相同，结果应一致。 */
    assert(btowc('A' + 256) == btowc('A'));
    assert(btowc('A' + 512) == btowc('A'));

    /* [3] 返回值类型为 wint_t，可与 WEOF 比较 */
    {
        wint_t r = btowc('Q');
        assert(r != WEOF);
        assert(r == (wint_t)L'Q');
    }

    /* [3] 与 wctob 的往返一致性（对有效单字节字符）：
     *     wctob(btowc(c)) 应还原出 (unsigned char)c。 */
    {
        int c;
        for (c = 0; c <= UCHAR_MAX; ++c) {
            wint_t w = btowc(c);
            if (w != WEOF) {
                assert(wctob(w) == (int)(unsigned char)c);
            }
        }
    }

    printf("C99 7.24.6.1.1 btowc: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「btowc 的参数类型为 int」：
 * 传入结构体类型实参，无法隐式转换为 int，gcc -std=c99 应报错。 */
struct S { int x; };
struct S s;
btowc(s);   /* error: incompatible type for argument 1 of 'btowc' */

/* 违反约束「btowc 的参数类型为 int」：
 * 传入指针类型，指针不能隐式转换为 int，应报错。 */
int *p;
btowc(p);   /* error: incompatible type for argument 1 of 'btowc' */

/* 违反约束「btowc 的参数类型为 int」：
 * 传入 double，浮点不能隐式转换为 int（无原型调用才会做默认提升，
 * 但此处有原型，故为约束违反），应报错。 */
btowc(3.14);   /* error: incompatible type for argument 1 of 'btowc' */

/* 违反约束「btowc 的参数个数为 1」：
 * 多传一个实参，应报错。 */
btowc('A', 'B');   /* error: too many arguments to function 'btowc' */

/* 违反约束「btowc 的参数个数为 1」：
 * 不传实参，应报错。 */
btowc();   /* error: too few arguments to function 'btowc' */

/* 违反约束「btowc 的返回类型为 wint_t，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
btowc('A') = 0;   /* error: lvalue required as left operand of assignment */

/* 违反约束「btowc 的返回类型为 wint_t」：
 * 用结构体类型接收返回值，无法隐式转换，应报错。 */
struct S t = btowc('A');   /* error: incompatible types in initialization */

#endif