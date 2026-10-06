/*
 * 测试条款：C99 7.24.3.3  The fputwc function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件
 *   [2] 描述：写入宽字符、推进文件位置指示器、append 模式追加
 *   [3] 返回值：成功返回写入的宽字符；写错误置错误指示器并返回 WEOF；
 *       编码错误置 errno = EILSEQ 并返回 WEOF
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 函数原型与头文件：<stdio.h> 与 <wchar.h> 均声明 fputwc，
 *     返回类型为 wint_t，参数为 (wchar_t, FILE *)。 */
static wint_t (*fp_fputwc)(wchar_t, FILE *) = fputwc;

static void test_prototype(void)
{
    /* [1] 取函数地址并调用，验证原型可用 */
    assert(fp_fputwc != NULL);
}

/* [2] 描述：写入宽字符，返回该宽字符；文件位置指示器被推进。 */
static void test_write_and_position(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    wchar_t c = L'A';
    wint_t r = fputwc(c, f);
    /* [3] 成功时返回写入的宽字符 */
    assert(r == (wint_t)c);

    /* [2] 位置指示器应被推进：再写一个字符，读回验证顺序 */
    r = fputwc(L'B', f);
    assert(r == (wint_t)L'B');

    /* 回到文件开头读回，验证写入内容与顺序 */
    assert(fseek(f, 0, SEEK_SET) == 0);

    /* 用 fgetwc 读回（同属宽字符 I/O，验证写入确实发生） */
    wint_t g1 = fgetwc(f);
    wint_t g2 = fgetwc(f);
    assert(g1 == (wint_t)L'A');
    assert(g2 == (wint_t)L'B');

    fclose(f);
}

/* [2] 描述：append 模式下字符被追加到输出流末尾。 */
static void test_append_mode(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    /* 先写入 "XY" */
    assert(fputwc(L'X', f) == (wint_t)L'X');
    assert(fputwc(L'Y', f) == (wint_t)L'Y');
    fclose(f);

    /* 重新以追加模式打开同一文件不可行（tmpfile 无名），
     * 改用命名临时文件测试 append 语义。 */
    const char *name = "c99_7_24_3_3_append.tmp";
    FILE *w = fopen(name, "w");
    assert(w != NULL);
    assert(fputwc(L'X', w) == (wint_t)L'X');
    assert(fputwc(L'Y', w) == (wint_t)L'Y');
    fclose(w);

    /* 以 append 模式打开，写入 'Z' 应追加到末尾 */
    FILE *a = fopen(name, "a");
    assert(a != NULL);
    assert(fputwc(L'Z', a) == (wint_t)L'Z');
    fclose(a);

    /* 读回验证顺序为 X Y Z */
    FILE *r = fopen(name, "r");
    assert(r != NULL);
    assert(fgetwc(r) == (wint_t)L'X');
    assert(fgetwc(r) == (wint_t)L'Y');
    assert(fgetwc(r) == (wint_t)L'Z');
    fclose(r);

    remove(name);
}

/* [3] 返回值：写错误时置错误指示器并返回 WEOF。 */
static void test_write_error(void)
{
    /* 以只读模式打开文件，向其中写入应触发写错误 */
    const char *name = "c99_7_24_3_3_ro.tmp";
    FILE *w = fopen(name, "w");
    assert(w != NULL);
    assert(fputwc(L'Q', w) == (wint_t)L'Q');
    fclose(w);

    FILE *ro = fopen(name, "r");
    assert(ro != NULL);

    errno = 0;
    wint_t r = fputwc(L'Z', ro);
    /* [3] 写错误：返回 WEOF 并置错误指示器 */
    assert(r == WEOF);
    assert(ferror(ro) != 0);

    fclose(ro);
    remove(name);
}

/* [3] 返回值：编码错误时 errno = EILSEQ 且返回 WEOF。
 *     通过设置一个无法表示该宽字符的 locale 来触发编码错误。
 *     若运行环境无法构造编码错误，则跳过该断言（仍验证接口可调用）。 */
static void test_encoding_error(void)
{
    /* 尝试切换到 C locale（通常只能表示基本字符集）。
     * 使用一个超出基本字符集的宽字符，期望编码错误。 */
    const char *saved = setlocale(LC_ALL, NULL);
    char savedbuf[128];
    if (saved != NULL) {
        strncpy(savedbuf, saved, sizeof(savedbuf) - 1);
        savedbuf[sizeof(savedbuf) - 1] = '\0';
    } else {
        savedbuf[0] = '\0';
    }

    if (setlocale(LC_ALL, "C") != NULL) {
        FILE *f = tmpfile();
        assert(f != NULL);

        errno = 0;
        /* 0x10FFFF 是 Unicode 最大码点，在 "C" locale 下通常无法编码 */
        wint_t r = fputwc((wchar_t)0x10FFFF, f);

        if (r == WEOF) {
            /* [3] 编码错误：errno 应为 EILSEQ */
            assert(errno == EILSEQ);
        }
        /* 若实现能够编码该字符，则 r 为写入的字符，属合法实现行为 */

        fclose(f);
    }

    /* 恢复原 locale */
    if (savedbuf[0] != '\0') {
        setlocale(LC_ALL, savedbuf);
    }
}

int main(void)
{
    /* 使用默认 locale 运行主要测试 */
    test_prototype();
    test_write_and_position();
    test_append_mode();
    test_write_error();
    test_encoding_error();

    printf("C99 7.24.3.3 fputwc: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fputwc 的第一个参数类型为 wchar_t」：
 * 传入 FILE* 作为第一个参数，类型不匹配，gcc -std=c99 应报错。 */
void neg_wrong_first_arg_type(FILE *f)
{
    fputwc(f, f);   /* error: incompatible type for argument 1 of 'fputwc' */
}

/* 违反约束「fputwc 的第二个参数类型为 FILE *」：
 * 传入 wchar_t 作为第二个参数，类型不匹配，应报错。 */
void neg_wrong_second_arg_type(wchar_t c)
{
    fputwc(c, c);   /* error: incompatible type for argument 2 of 'fputwc' */
}

/* 违反约束「fputwc 需要两个实参」：
 * 参数个数不足，应报错。 */
void neg_too_few_args(FILE *f)
{
    fputwc(f);      /* error: too few arguments to function 'fputwc' */
}

/* 违反约束「fputwc 需要两个实参」：
 * 参数个数过多，应报错。 */
void neg_too_many_args(wchar_t c, FILE *f)
{
    fputwc(c, f, f); /* error: too many arguments to function 'fputwc' */
}

/* 违反约束「fputwc 返回 wint_t，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
void neg_assign_to_result(wchar_t c, FILE *f)
{
    fputwc(c, f) = 0; /* error: lvalue required as left operand of assignment */
}

/* 违反约束「fputwc 的返回类型为 wint_t，不能用于需要结构体等不兼容类型处」：
 * 将返回值赋给不兼容的指针类型，应报错。 */
void neg_incompatible_return_use(wchar_t c, FILE *f)
{
    int *p = fputwc(c, f); /* error: initialization makes pointer from integer */
    (void)p;
}

#endif /* 负向测试结束 */