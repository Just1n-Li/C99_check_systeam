/*
 * 测试 C99 7.24.3.1 —— fgetwc 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明：wint_t fgetwc(FILE *stream);
 *   [2] 语义：若未置 EOF 指示器且存在下一个宽字符，则取得该宽字符
 *           （wchar_t 转换为 wint_t），并推进文件位置指示器。
 *   [3] 返回：EOF 指示器已置或流处于文件尾 -> 置 EOF 指示器并返回 WEOF；
 *           否则返回下一个宽字符；读错误 -> 置错误指示器并返回 WEOF；
 *           编码错误 -> errno 置 EILSEQ 并返回 WEOF。
 *   Footnote 292：feof/ferror 可区分 EOF 与读错误；errno 仅在编码错误时置 EILSEQ。
 */

#include <stdio.h>
#include <wchar.h>
#include <wchar.h>   /* 重复包含，验证头文件保护 */
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证函数原型存在且类型正确：wint_t fgetwc(FILE *); */
static void test_prototype(void)
{
    /* 取函数地址赋给正确类型的函数指针，若原型不符则编译失败 */
    wint_t (*fp)(FILE *) = fgetwc;
    assert(fp != NULL);
    /* 返回值类型为 wint_t */
    {
        wint_t w = WEOF;
        (void)w;
    }
}

/* [2][3] 正常读取：写入若干宽字符，逐个读回，验证值与位置推进 */
static void test_normal_read(void)
{
    FILE *fp;
    wint_t wc;
    int i;
    const wchar_t src[] = L"Hello";   /* 5 个宽字符 */

    fp = tmpfile();
    assert(fp != NULL);

    /* 以宽字符方式写入 */
    for (i = 0; src[i] != L'\0'; ++i) {
        assert(fputwc(src[i], fp) != WEOF);
    }
    /* 回到文件开头 */
    assert(fseek(fp, 0, SEEK_SET) == 0);

    /* [2] 逐个读取，验证取得正确的宽字符，且位置指示器推进 */
    for (i = 0; src[i] != L'\0'; ++i) {
        wc = fgetwc(fp);
        assert(wc != WEOF);
        assert((wchar_t)wc == src[i]);   /* wchar_t 转换为 wint_t 后读回 */
        /* 位置指示器应已推进：ftell 递增 */
        assert(ftell(fp) == (long)(i + 1));
    }

    /* [3] 到达文件尾：返回 WEOF，并置 EOF 指示器 */
    wc = fgetwc(fp);
    assert(wc == WEOF);
    assert(feof(fp) != 0);      /* EOF 指示器被置位 */
    assert(ferror(fp) == 0);    /* 不是读错误 */

    /* [3] EOF 指示器已置时再次调用，仍返回 WEOF */
    wc = fgetwc(fp);
    assert(wc == WEOF);
    assert(feof(fp) != 0);

    fclose(fp);
}

/* [3] 空文件：首次读取即处于文件尾，返回 WEOF 并置 EOF 指示器 */
static void test_empty_file(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    assert(fgetwc(fp) == WEOF);
    assert(feof(fp) != 0);
    assert(ferror(fp) == 0);

    fclose(fp);
}

/* [3] + Footnote 292：读错误时置错误指示器并返回 WEOF，
 *      且 feof/ferror 可区分 EOF 与读错误 */
static void test_read_error(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    /* 以只写方式打开同一底层文件不可行；改用只读打开后关闭底层描述符
     * 制造读错误。这里用更可移植的方式：对只写流调用 fgetwc。 */
    fclose(fp);

    fp = tmpfile();
    assert(fp != NULL);
    /* 关闭底层文件描述符以制造读错误（POSIX 环境常见做法） */
    {
        int fd = fileno(fp);
        if (fd >= 0) {
            /* 关闭 fd 后，后续读操作会失败 */
            close(fd);
        }
    }
    {
        wint_t wc = fgetwc(fp);
        /* 读错误：返回 WEOF 且置错误指示器 */
        assert(wc == WEOF);
        assert(ferror(fp) != 0);
        /* Footnote 292：可用 ferror 区分读错误与 EOF */
    }
    fclose(fp);
}

/* [3] + Footnote 292：编码错误时 errno 置 EILSEQ 并返回 WEOF。
 *      构造非法多字节序列（在 UTF-8 环境下写入非法字节）。 */
static void test_encoding_error(void)
{
    FILE *fp;
    const unsigned char bad[] = { 0xFF, 0xFE, 0xFD }; /* 非法 UTF-8 序列 */

    /* 尝试设置 UTF-8 locale；若不可用则跳过该测试 */
    if (setlocale(LC_ALL, "en_US.UTF-8") == NULL &&
        setlocale(LC_ALL, "C.UTF-8") == NULL) {
        return; /* 环境不支持，跳过 */
    }

    fp = tmpfile();
    assert(fp != NULL);

    /* 以字节方式写入非法序列 */
    assert(fwrite(bad, 1, sizeof bad, fp) == sizeof bad);
    assert(fseek(fp, 0, SEEK_SET) == 0);

    errno = 0;
    {
        wint_t wc = fgetwc(fp);
        if (wc == WEOF) {
            /* 若发生编码错误，errno 应为 EILSEQ（Footnote 292） */
            assert(errno == EILSEQ);
        }
    }
    fclose(fp);
}

/* [2] 混合读写：验证 fgetwc 与 fputwc 交替使用时的位置推进 */
static void test_position_advance(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    assert(fputwc(L'A', fp) != WEOF);
    assert(fputwc(L'B', fp) != WEOF);
    assert(fseek(fp, 0, SEEK_SET) == 0);

    assert((wchar_t)fgetwc(fp) == L'A');
    assert(ftell(fp) == 1L);
    assert((wchar_t)fgetwc(fp) == L'B');
    assert(ftell(fp) == 2L);

    fclose(fp);
}

int main(void)
{
    test_prototype();
    test_normal_read();
    test_empty_file();
    test_read_error();
    test_encoding_error();
    test_position_advance();

    printf("All positive tests for C99 7.24.3.1 (fgetwc) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fgetwc 的参数必须为 FILE * 类型」：
 * 传入 int 而非 FILE *，gcc -std=c99 应报错
 * （incompatible type for argument / passing argument 1 makes pointer from integer）。 */
void neg_wrong_arg_type(void)
{
    int x = 0;
    fgetwc(x);          /* 错误：参数应为 FILE * */
}

/* 违反约束「fgetwc 需要恰好一个参数」：
 * 无参数调用，gcc -std=c99 应报错（too few arguments to function 'fgetwc'）。 */
void neg_too_few_args(void)
{
    fgetwc();           /* 错误：缺少参数 */
}

/* 违反约束「fgetwc 需要恰好一个参数」：
 * 多传参数，gcc -std=c99 应报错（too many arguments to function 'fgetwc'）。 */
void neg_too_many_args(FILE *fp)
{
    fgetwc(fp, fp);     /* 错误：参数过多 */
}

/* 违反约束「fgetwc 的返回类型为 wint_t，不可作为左值赋值」：
 * 对函数调用结果赋值，gcc -std=c99 应报错（lvalue required as left operand of assignment）。 */
void neg_assign_to_result(FILE *fp)
{
    fgetwc(fp) = 0;     /* 错误：函数调用结果不是左值 */
}

/* 违反约束「fgetwc 的返回类型为 wint_t，不可取地址」：
 * 对函数调用结果取地址，gcc -std=c99 应报错（lvalue required as unary '&' operand）。 */
void neg_address_of_result(FILE *fp)
{
    wint_t *p = &fgetwc(fp);   /* 错误：函数调用结果不是左值 */
    (void)p;
}

/* 违反约束「fgetwc 的返回类型为 wint_t，不可自增」：
 * 对函数调用结果使用 ++，gcc -std=c99 应报错（lvalue required as increment operand）。 */
void neg_increment_result(FILE *fp)
{
    ++fgetwc(fp);       /* 错误：函数调用结果不是左值 */
}

/* 违反约束「fgetwc 的返回类型为 wint_t，不可作为赋值目标」：
 * 复合赋值同样要求左值，gcc -std=c99 应报错。 */
void neg_compound_assign(FILE *fp)
{
    fgetwc(fp) += 1;    /* 错误：函数调用结果不是左值 */
}

#endif /* 负向测试结束 */