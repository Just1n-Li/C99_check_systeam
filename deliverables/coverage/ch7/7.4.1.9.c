/*
 * 测试 C99 7.4.1.9 —— ispunct 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int ispunct(int c);  声明于 <ctype.h>
 *   [2] 语义：测试“打印字符”中，既非 isspace 又非 isalnum 的标点字符。
 *       在 "C" locale 下，对每个打印字符，当且仅当 isspace 与 isalnum
 *       均为假时 ispunct 返回真。
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：ispunct 接受 int 参数并返回 int。
 *     这里用一个函数指针来静态验证原型签名。 */
static int (*fp_ispunct)(int) = ispunct;

/* [2] 在 "C" locale 下，对每个打印字符验证：
 *     ispunct(c) 为真  <=>  isspace(c) 为假 且 isalnum(c) 为假
 *     对非打印字符，ispunct 应为假。 */
static void test_c_locale_consistency(void)
{
    int c;
    for (c = 0; c <= UCHAR_MAX; c++) {
        int is_print = isprint(c);
        int p = ispunct(c);
        int s = isspace(c);
        int a = isalnum(c);

        if (is_print) {
            /* 打印字符：ispunct 当且仅当 !isspace && !isalnum */
            assert((p != 0) == ((s == 0) && (a == 0)));
        } else {
            /* 非打印字符：不是标点 */
            assert(p == 0);
        }
    }
}

/* [2] 具体字符的已知结果（"C" locale）：
 *     标点字符：! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
 *     非标点：字母、数字、空白、控制字符 */
static void test_known_punctuation(void)
{
    const char *punct = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    const char *p;

    for (p = punct; *p != '\0'; p++) {
        assert(ispunct((unsigned char)*p) != 0);
        assert(isspace((unsigned char)*p) == 0);
        assert(isalnum((unsigned char)*p) == 0);
    }

    /* 字母、数字不是标点 */
    assert(ispunct('A') == 0);
    assert(ispunct('z') == 0);
    assert(ispunct('0') == 0);
    assert(ispunct('9') == 0);

    /* 空白字符不是标点 */
    assert(ispunct(' ') == 0);
    assert(ispunct('\t') == 0);
    assert(ispunct('\n') == 0);
    assert(ispunct('\v') == 0);
    assert(ispunct('\f') == 0);
    assert(ispunct('\r') == 0);

    /* 控制字符不是标点 */
    assert(ispunct('\0') == 0);
    assert(ispunct(0x01) == 0);
    assert(ispunct(0x1F) == 0);
    assert(ispunct(0x7F) == 0); /* DEL 非打印 */
}

/* [1] 参数为 int，且必须可表示为 unsigned char 或 EOF。
 *     验证 EOF 传入不会崩溃，且返回 0（EOF 不是标点）。 */
static void test_eof_argument(void)
{
    assert(ispunct(EOF) == 0);
}

/* [1] 通过函数指针调用，验证原型可用。 */
static void test_via_pointer(void)
{
    assert(fp_ispunct('!') != 0);
    assert(fp_ispunct('A') == 0);
    assert(fp_ispunct(' ') == 0);
}

int main(void)
{
    test_c_locale_consistency();
    test_known_punctuation();
    test_eof_argument();
    test_via_pointer();

    printf("C99 7.4.1.9 ispunct: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「ispunct 的参数类型为 int」：
 * 传入结构体类型，无法隐式转换为 int，gcc -std=c99 应报错。 */
struct S { int x; } s;
ispunct(s);

/* 违反约束「ispunct 的参数类型为 int」：
 * 传入指针类型，指针不能隐式转换为 int，应报错。 */
int *ptr = 0;
ispunct(ptr);

/* 违反约束「ispunct 返回 int」：
 * 试图把返回值赋给结构体类型，类型不兼容，应报错。 */
struct S t;
t = ispunct('!');

/* 违反约束「ispunct 的参数类型为 int」：
 * 传入浮点类型，浮点不能隐式转换为 int（无原型时才会默认提升，
 * 但此处有原型，参数必须为 int），应报错。 */
ispunct(3.14);

/* 违反约束「ispunct 的参数类型为 int」：
 * 传入数组类型，数组不能隐式转换为 int，应报错。 */
int arr[4];
ispunct(arr);

#endif