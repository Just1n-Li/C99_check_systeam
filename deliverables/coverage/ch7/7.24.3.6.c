/*
 * 测试条款：C99 7.24.3.6  The getwc function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] Synopsis：头文件 <stdio.h>、<wchar.h>，原型 wint_t getwc(FILE *stream);
 *   [2] Description：等价于 fgetwc；若实现为宏，可能多次求值 stream，
 *                    故实参不应带副作用。
 *   [3] Returns：返回流中下一个宽字符，或 WEOF。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stdlib.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证 getwc 的原型与返回类型 wint_t。
 *     通过取函数指针并赋值给匹配类型的指针来静态检查签名。 */
static wint_t (*getwc_proto_check)(FILE *) = getwc;

/* [1] 验证 wint_t 是整数类型，且 WEOF 是 wint_t 类型的常量表达式。 */
static void test_synopsis_types(void)
{
    /* wint_t 必须是整数类型 */
    wint_t w = WEOF;
    (void)w;

    /* getwc 的返回类型为 wint_t：把返回值赋给 wint_t 变量 */
    wint_t (*fp)(FILE *) = getwc;
    assert(fp == getwc_proto_check);
}

/* [3] 从流中读取宽字符，返回下一个宽字符或 WEOF。
 *     这里用 tmpfile() 构造一个可读写的流，写入若干宽字符后回绕读取。 */
static void test_returns_next_wide_char(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    /* 写入三个宽字符 */
    assert(fputwc(L'A', fp) != WEOF);
    assert(fputwc(L'B', fp) != WEOF);
    assert(fputwc(L'C', fp) != WEOF);

    /* 回绕到文件开头 */
    assert(fseek(fp, 0, SEEK_SET) == 0);

    /* [3] 依次读取，应得到刚写入的宽字符 */
    wint_t c1 = getwc(fp);
    wint_t c2 = getwc(fp);
    wint_t c3 = getwc(fp);
    assert(c1 == (wint_t)L'A');
    assert(c2 == (wint_t)L'B');
    assert(c3 == (wint_t)L'C');

    /* [3] 到达文件末尾后应返回 WEOF */
    wint_t c4 = getwc(fp);
    assert(c4 == WEOF);

    fclose(fp);
}

/* [2] getwc 等价于 fgetwc：对同样的输入，两者应产生相同结果。
 *     分别用两个内容相同的流，比较 getwc 与 fgetwc 的返回值。 */
static void test_equivalent_to_fgetwc(void)
{
    FILE *fp1 = tmpfile();
    FILE *fp2 = tmpfile();
    assert(fp1 != NULL && fp2 != NULL);

    /* 两个流写入相同内容 */
    assert(fputwc(L'X', fp1) != WEOF);
    assert(fputwc(L'Y', fp1) != WEOF);
    assert(fputwc(L'X', fp2) != WEOF);
    assert(fputwc(L'Y', fp2) != WEOF);

    assert(fseek(fp1, 0, SEEK_SET) == 0);
    assert(fseek(fp2, 0, SEEK_SET) == 0);

    /* [2] 逐个比较 getwc 与 fgetwc 的结果 */
    for (int i = 0; i < 3; ++i) {
        wint_t a = getwc(fp1);
        wint_t b = fgetwc(fp2);
        assert(a == b);
    }

    fclose(fp1);
    fclose(fp2);
}

/* [2] 若 getwc 被实现为宏，可能多次求值 stream，因此实参不应有副作用。
 *     这里演示“无副作用”的正确用法：传入一个普通 FILE* 变量。 */
static void test_no_side_effect_argument(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    assert(fputwc(L'Z', fp) != WEOF);
    assert(fseek(fp, 0, SEEK_SET) == 0);

    /* 正确用法：实参是简单变量，无副作用 */
    wint_t c = getwc(fp);
    assert(c == (wint_t)L'Z');

    fclose(fp);
}

/* [3] 空流（未写入任何内容）读取时应立即返回 WEOF。 */
static void test_empty_stream_returns_weof(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    wint_t c = getwc(fp);
    assert(c == WEOF);

    fclose(fp);
}

int main(void)
{
    /* 使用 C locale 即可，宽字符 L'A' 等在此 locale 下可正常处理 */
    setlocale(LC_ALL, "C");

    test_synopsis_types();
    test_returns_next_wide_char();
    test_equivalent_to_fgetwc();
    test_no_side_effect_argument();
    test_empty_stream_returns_weof();

    printf("C99 7.24.3.6 getwc: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「getwc 的实参类型必须为 FILE *」：
 * 传入 int 而非 FILE*，gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg_type(void)
{
    int x = 0;
    getwc(x);            /* error: 实参类型不匹配 */
}

/* 违反约束「getwc 需要恰好一个实参」：
 * 少传实参，gcc -std=c99 应报 too few arguments 错误。 */
void bad_too_few_args(void)
{
    getwc();             /* error: 实参个数不足 */
}

/* 违反约束「getwc 需要恰好一个实参」：
 * 多传实参，gcc -std=c99 应报 too many arguments 错误。 */
void bad_too_many_args(FILE *fp)
{
    getwc(fp, fp);       /* error: 实参个数过多 */
}

/* 违反约束「getwc 的返回类型为 wint_t，不能作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
void bad_assign_to_call(FILE *fp)
{
    getwc(fp) = 0;       /* error: 赋值目标不是左值 */
}

/* 违反约束「getwc 的返回类型为 wint_t，不能取地址」：
 * 函数调用结果不是左值，取地址应编译报错。 */
void bad_address_of_call(FILE *fp)
{
    wint_t *p = &getwc(fp);   /* error: 无法对非左值取地址 */
    (void)p;
}

/* 违反约束「getwc 的实参必须为 FILE *，不能为 void*」：
 * 隐式转换 void* 到 FILE* 在 C 中虽允许，但此处用结构体指针，
 * 结构体指针不能隐式转换为 FILE*，应编译报错。 */
struct NotAFile { int dummy; };
void bad_struct_ptr_arg(struct NotAFile *p)
{
    getwc(p);            /* error: 结构体指针不能转换为 FILE* */
}

#endif /* 负向测试结束 */