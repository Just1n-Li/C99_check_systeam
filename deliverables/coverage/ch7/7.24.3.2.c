/*
 * 测试目标：C99 7.24.3.2  The fgetws function
 *
 * 预期行为：
 *   - 正向测试：以下使用 fgetws 的代码应能编译并运行通过（assert 全部成立）。
 *   - 负向测试：违反约束的代码片段应被编译器拒绝（编译报错），
 *               这些片段统一放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <wchar.h>（fgetws 声明）
 *   [2] 语义：最多读 n-1 个宽字符；遇换行宽字符（保留）或 EOF 停止；
 *       在最后读入的宽字符之后立即写入一个空宽字符 L'\0'。
 *   [3] 返回值：成功返回 s；EOF 且未读入任何字符时数组内容不变并返回 NULL；
 *       读/编码错误时数组内容不确定并返回 NULL。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* 辅助：把宽字符串写入一个临时文件，返回文件名（静态缓冲） */
static const char *make_wfile(const wchar_t *ws)
{
    static char name[L_tmpnam];
    FILE *fp;
    tmpnam(name);
    fp = fopen(name, "w");
    assert(fp != NULL);
    /* 用 fputws 写入宽字符内容 */
    assert(fputws(ws, fp) >= 0);
    fclose(fp);
    return name;
}

/* [1] 原型与头文件：确认 fgetws 可用，且签名与标准一致 */
static void test_prototype(void)
{
    /* 若 <wchar.h> 未声明 fgetws，此处会因隐式声明而告警/报错 */
    wchar_t *(*fp)(wchar_t * restrict, int, FILE * restrict) = fgetws;
    assert(fp != NULL);
}

/* [2] 基本读取：读入一行（含换行），并在末尾写入 L'\0' */
static void test_basic_read(void)
{
    const char *name = make_wfile(L"hello\nworld\n");
    FILE *fp = fopen(name, "r");
    wchar_t buf[64];
    wchar_t *r;

    assert(fp != NULL);
    /* 缓冲区先填满非零值，验证 fgetws 会写入终止空宽字符 */
    wmemset(buf, L'X', 64);

    r = fgetws(buf, 64, fp);
    assert(r == buf);                 /* [3] 成功返回 s */
    /* [2] 换行宽字符被保留，且其后立即写入 L'\0' */
    assert(wcscmp(buf, L"hello\n") == 0);
    assert(buf[6] == L'\0');          /* 终止空宽字符位置正确 */

    /* 再读一行 */
    r = fgetws(buf, 64, fp);
    assert(r == buf);
    assert(wcscmp(buf, L"world\n") == 0);

    fclose(fp);
    remove(name);
}

/* [2] n 的限制：最多读入 n-1 个宽字符，第 n 个位置写 L'\0' */
static void test_n_limit(void)
{
    const char *name = make_wfile(L"abcdefghij\n");
    FILE *fp = fopen(name, "r");
    wchar_t buf[8];
    wchar_t *r;

    assert(fp != NULL);
    wmemset(buf, L'X', 8);

    /* n = 5：最多读 4 个宽字符，buf[4] 写 L'\0' */
    r = fgetws(buf, 5, fp);
    assert(r == buf);
    assert(buf[0] == L'a' && buf[1] == L'b' &&
           buf[2] == L'c' && buf[3] == L'd');
    assert(buf[4] == L'\0');          /* 第 n 个位置（索引 n-1）为空宽字符 */

    /* 继续读剩余部分 */
    r = fgetws(buf, 8, fp);
    assert(r == buf);
    assert(wcscmp(buf, L"efghij\n") == 0);

    fclose(fp);
    remove(name);
}

/* [2] 遇换行停止：即使缓冲区足够大，也只读到换行（含换行）为止 */
static void test_stop_at_newline(void)
{
    const char *name = make_wfile(L"ab\ncd\n");
    FILE *fp = fopen(name, "r");
    wchar_t buf[64];
    wchar_t *r;

    assert(fp != NULL);
    r = fgetws(buf, 64, fp);
    assert(r == buf);
    assert(wcscmp(buf, L"ab\n") == 0);   /* 换行后不再继续读 */

    r = fgetws(buf, 64, fp);
    assert(r == buf);
    assert(wcscmp(buf, L"cd\n") == 0);

    fclose(fp);
    remove(name);
}

/* [2] 无换行的最后一行：读到 EOF 停止，仍写入 L'\0' */
static void test_no_trailing_newline(void)
{
    const char *name = make_wfile(L"xyz");
    FILE *fp = fopen(name, "r");
    wchar_t buf[64];
    wchar_t *r;

    assert(fp != NULL);
    r = fgetws(buf, 64, fp);
    assert(r == buf);
    assert(wcscmp(buf, L"xyz") == 0);    /* 无换行，读到 EOF 停止 */
    assert(buf[3] == L'\0');

    fclose(fp);
    remove(name);
}

/* [3] EOF 且未读入任何字符：数组内容不变，返回 NULL */
static void test_eof_no_chars(void)
{
    const char *name = make_wfile(L"");
    FILE *fp = fopen(name, "r");
    wchar_t buf[16];
    wchar_t *r;
    int i;

    assert(fp != NULL);
    /* 预置可识别的哨兵值 */
    for (i = 0; i < 16; i++)
        buf[i] = (wchar_t)(0x1000 + i);

    r = fgetws(buf, 16, fp);
    assert(r == NULL);                   /* [3] 返回空指针 */
    /* [3] 数组内容保持不变 */
    for (i = 0; i < 16; i++)
        assert(buf[i] == (wchar_t)(0x1000 + i));

    fclose(fp);
    remove(name);
}

/* [3] 读到 EOF 之前已读入字符：返回 s（不是 NULL） */
static void test_eof_after_chars(void)
{
    const char *name = make_wfile(L"abc");
    FILE *fp = fopen(name, "r");
    wchar_t buf[64];
    wchar_t *r;

    assert(fp != NULL);
    r = fgetws(buf, 64, fp);
    assert(r == buf);                    /* 读到了字符，成功返回 s */
    assert(wcscmp(buf, L"abc") == 0);

    /* 再次调用：此时已到 EOF 且未读入字符，应返回 NULL */
    r = fgetws(buf, 64, fp);
    assert(r == NULL);

    fclose(fp);
    remove(name);
}

/* [2] 空宽字符 L'\0' 作为数据：fgetws 会把它当作普通宽字符读入，
 *      并在其后写入终止空宽字符（因此缓冲区中出现两个连续 L'\0'） */
static void test_embedded_nul(void)
{
    const char *name = make_wfile(L"a\0b\n");
    FILE *fp = fopen(name, "r");
    wchar_t buf[64];
    wchar_t *r;

    assert(fp != NULL);
    wmemset(buf, L'X', 64);
    r = fgetws(buf, 64, fp);
    assert(r == buf);
    /* 读入 'a'、L'\0'、'b'、'\n'，然后写终止 L'\0' */
    assert(buf[0] == L'a');
    assert(buf[1] == L'\0');
    assert(buf[2] == L'b');
    assert(buf[3] == L'\n');
    assert(buf[4] == L'\0');

    fclose(fp);
    remove(name);
}

/* [2] n == 1：最多读 0 个宽字符，只写终止空宽字符，返回 s */
static void test_n_one(void)
{
    const char *name = make_wfile(L"abc\n");
    FILE *fp = fopen(name, "r");
    wchar_t buf[4];
    wchar_t *r;

    assert(fp != NULL);
    wmemset(buf, L'X', 4);
    r = fgetws(buf, 1, fp);
    assert(r == buf);                    /* 成功返回 s */
    assert(buf[0] == L'\0');             /* 只写了终止空宽字符 */

    fclose(fp);
    remove(name);
}

int main(void)
{
    /* 宽字符 I/O 依赖本地化环境；设为 C locale 即可处理 ASCII 宽字符 */
    setlocale(LC_ALL, "C");

    test_prototype();
    test_basic_read();
    test_n_limit();
    test_stop_at_newline();
    test_no_trailing_newline();
    test_eof_no_chars();
    test_eof_after_chars();
    test_embedded_nul();
    test_n_one();

    printf("All positive tests for C99 7.24.3.2 fgetws passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fgetws 的第一个参数类型为 wchar_t *」：
 * 传入 char * 而非 wchar_t *，gcc -std=c99 应报 incompatible pointer type 错误。 */
void neg_wrong_first_arg(void)
{
    char cbuf[16];
    FILE *fp = stdin;
    fgetws(cbuf, 16, fp);   /* 错误：char* 不能传给 wchar_t* 形参 */
}

/* 违反约束「fgetws 的第二个参数类型为 int」：
 * 传入指针类型，gcc -std=c99 应报 incompatible type 错误。 */
void neg_wrong_second_arg(void)
{
    wchar_t buf[16];
    int *p = 0;
    fgetws(buf, p, stdin);  /* 错误：int* 不能传给 int 形参 */
}

/* 违反约束「fgetws 的第三个参数类型为 FILE *」：
 * 传入 int，gcc -std=c99 应报 incompatible type 错误。 */
void neg_wrong_third_arg(void)
{
    wchar_t buf[16];
    fgetws(buf, 16, 0);     /* 错误：int 不能传给 FILE* 形参（0 除外） */
}

/* 违反约束「fgetws 需要 3 个实参」：
 * 实参个数不足，gcc -std=c99 应报 too few arguments 错误。 */
void neg_too_few_args(void)
{
    wchar_t buf[16];
    fgetws(buf, 16);        /* 错误：缺少第三个实参 */
}

/* 违反约束「fgetws 需要 3 个实参」：
 * 实参个数过多，gcc -std=c99 应报 too many arguments 错误。 */
void neg_too_many_args(void)
{
    wchar_t buf[16];
    fgetws(buf, 16, stdin, stdin);  /* 错误：多了一个实参 */
}

/* 违反约束「fgetws 的返回类型为 wchar_t *」：
 * 把返回值赋给不兼容的指针类型，gcc -std=c99 应报 incompatible type 错误。 */
void neg_wrong_return_use(void)
{
    wchar_t buf[16];
    int *p;
    p = fgetws(buf, 16, stdin);   /* 错误：wchar_t* 赋给 int* */
}

/* 违反约束「fgetws 的返回类型为 wchar_t *」：
 * 把返回值赋给非指针类型，gcc -std=c99 应报 incompatible type 错误。 */
void neg_return_to_int(void)
{
    wchar_t buf[16];
    int x;
    x = fgetws(buf, 16, stdin);   /* 错误：wchar_t* 赋给 int */
}

#endif /* 负向测试结束 */