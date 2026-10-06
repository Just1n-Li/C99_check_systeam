/*
 * 测试条款：C99 7.25.3.1.2  towupper 函数
 *
 * 预期行为：
 *   正向测试：包含 <wctype.h>，调用 towupper，验证：
 *     [1] 原型 wint_t towupper(wint_t);
 *     [2] 小写字母 -> 对应大写字母
 *     [3] 若 iswlower(wc) 为真且存在对应的大写宽字符，返回其中之一（同一 locale 恒定）；
 *         否则原样返回参数。
 *   负向测试：违反约束的代码应编译报错（见 #if 0 块）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra towupper_test.c -o towupper_test
 */

#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须为 wint_t (*)(wint_t) */
static wint_t (*fp_towupper)(wint_t) = towupper;

/* [2] 小写字母转换为对应大写字母 */
static void test_lower_to_upper(void)
{
    /* 基本 ASCII 小写字母 */
    assert(towupper(L'a') == L'A');
    assert(towupper(L'z') == L'Z');
    assert(towupper(L'm') == L'M');

    /* 转换结果应满足 iswupper 为真 */
    assert(iswupper(towupper(L'a')));
    assert(iswupper(towupper(L'z')));

    /* 通过函数指针调用，验证 [1] 的原型 */
    assert(fp_towupper(L'b') == L'B');
}

/* [3] 非小写字母的参数应原样返回 */
static void test_unchanged(void)
{
    /* 已经是大写字母：iswlower 为假 -> 原样返回 */
    assert(towupper(L'A') == L'A');
    assert(towupper(L'Z') == L'Z');

    /* 数字：iswlower 为假 -> 原样返回 */
    assert(towupper(L'0') == L'0');
    assert(towupper(L'9') == L'9');

    /* 标点与空白：iswlower 为假 -> 原样返回 */
    assert(towupper(L' ') == L' ');
    assert(towupper(L'!') == L'!');
    assert(towupper(L'\n') == L'\n');

    /* 宽字符 WEOF：iswlower(WEOF) 为假 -> 原样返回 */
    assert(towupper(WEOF) == WEOF);
}

/* [3] 同一 locale 下，对同一输入必须返回同一个结果（确定性） */
static void test_deterministic(void)
{
    wint_t r1 = towupper(L'a');
    wint_t r2 = towupper(L'a');
    wint_t r3 = towupper(L'a');
    assert(r1 == r2);
    assert(r2 == r3);
    assert(r1 == L'A');
}

/* [3] 若 iswlower(wc) 为真，则 towupper(wc) 的结果应满足 iswupper 为真
 *     （前提：该 locale 下存在对应的大写宽字符） */
static void test_consistency_with_iswlower(void)
{
    const wchar_t samples[] = { L'a', L'b', L'c', L'x', L'y', L'z' };
    size_t i;
    for (i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        wint_t wc = (wint_t)samples[i];
        if (iswlower(wc)) {
            wint_t up = towupper(wc);
            /* 存在对应大写字符时，结果应为大写 */
            assert(iswupper(up));
            /* 且结果不等于原小写字符 */
            assert(up != wc);
        }
    }
}

/* [2][3] 在 "C" locale 下验证完整映射表 */
static void test_c_locale_mapping(void)
{
    int c;
    for (c = 'a'; c <= 'z'; ++c) {
        wint_t up = towupper((wint_t)c);
        assert(up == (wint_t)(c - 'a' + 'A'));
    }
}

int main(void)
{
    /* 使用默认 "C" locale，保证行为可预测 */
    setlocale(LC_ALL, "C");

    test_lower_to_upper();          /* [1][2] */
    test_unchanged();               /* [3] */
    test_deterministic();           /* [3] */
    test_consistency_with_iswlower();/* [3] */
    test_c_locale_mapping();        /* [2][3] */

    printf("towupper: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「调用函数必须使用正确数量的实参」：
 * towupper 原型为 wint_t towupper(wint_t)，只接受 1 个实参。
 * gcc -std=c99 应报错：too many arguments to function 'towupper' */
void bad_too_many_args(void)
{
    (void)towupper(L'a', L'b');
}

/* 违反约束「调用函数必须使用正确数量的实参」：
 * 少传实参。gcc -std=c99 应报错：too few arguments to function 'towupper' */
void bad_too_few_args(void)
{
    (void)towupper();
}

/* 违反约束「实参类型必须与形参类型兼容」：
 * 传入指针而非 wint_t（整型）。gcc -std=c99 应报错：
 * incompatible type for argument 1 of 'towupper' */
void bad_arg_type(void)
{
    const char *p = "a";
    (void)towupper(p);
}

/* 违反约束「函数返回值不能被赋值」：
 * towupper 返回 wint_t（非左值），对其赋值违反赋值运算符约束。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
void bad_assign_to_return(void)
{
    towupper(L'a') = L'A';
}

/* 违反约束「不能对函数返回值取地址」：
 * 函数调用结果不是左值，取地址违反一元 & 运算符约束。
 * gcc -std=c99 应报错：lvalue required as unary '&' operand */
void bad_addr_of_return(void)
{
    wint_t *p = &towupper(L'a');
    (void)p;
}

/* 违反约束「不能对函数返回值自增」：
 * 自增运算符要求操作数为可修改左值。
 * gcc -std=c99 应报错：lvalue required as increment operand */
void bad_increment_return(void)
{
    ++towupper(L'a');
}

#endif /* 负向测试结束 */