/*
 * 测试 C99 7.24.2.3 —— swprintf 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明：int swprintf(wchar_t * restrict s, size_t n,
 *                              const wchar_t * restrict format, ...);
 *   [2] 语义：等价于 fwprintf，但输出写入宽字符数组 s；
 *            最多写入 n 个宽字符（含结尾空宽字符，n 为 0 时除外）。
 *   [3] 返回值：写入的宽字符数（不含结尾空宽字符）；
 *            若发生编码错误，或请求写入的宽字符数 >= n，则返回负值。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证签名可用 */
static int (*swprintf_proto)(wchar_t * restrict, size_t,
                             const wchar_t * restrict, ...) = swprintf;

/* [2] 基本写入：n 足够大，应写入完整字符串 + 结尾空宽字符 */
static void test_basic_write(void)
{
    wchar_t buf[64];
    int ret;

    /* 先填充非零值，验证结尾空宽字符确实被写入 */
    wmemset(buf, L'X', 64);

    ret = swprintf(buf, 64, L"hello %ls %d", L"world", 42);

    /* [3] 返回值 = 写入的宽字符数（不含结尾空宽字符） */
    assert(ret == (int)wcslen(L"hello world 42"));
    assert(ret == 14);

    /* [2] 内容正确 */
    assert(wcscmp(buf, L"hello world 42") == 0);

    /* [2] 结尾空宽字符被添加 */
    assert(buf[ret] == L'\0');
}

/* [2] n 恰好等于所需宽字符数（含结尾空宽字符）时应成功 */
static void test_exact_fit(void)
{
    wchar_t buf[6]; /* "abcde" + L'\0' = 6 */
    int ret;

    ret = swprintf(buf, 6, L"abcde");
    assert(ret == 5);
    assert(wcscmp(buf, L"abcde") == 0);
    assert(buf[5] == L'\0');
}

/* [3] 请求写入的宽字符数 >= n 时返回负值 */
static void test_truncation_returns_negative(void)
{
    wchar_t buf[4];
    int ret;

    /* 需要 6 个宽字符（含结尾），但 n = 4，应返回负值 */
    ret = swprintf(buf, 4, L"abcde");
    assert(ret < 0);
}

/* [3] n 为 0 时：不写入任何宽字符（包括结尾空宽字符），返回负值 */
static void test_n_zero(void)
{
    wchar_t buf[8];
    int ret;

    wmemset(buf, L'Z', 8);

    ret = swprintf(buf, 0, L"abc");

    /* [3] 请求写入的宽字符数 >= n(=0)，返回负值 */
    assert(ret < 0);

    /* [2] n 为 0 时不写入任何内容（含结尾空宽字符） */
    assert(buf[0] == L'Z');
    assert(buf[1] == L'Z');
}

/* [2] 空格式串：只写入结尾空宽字符，返回 0 */
static void test_empty_format(void)
{
    wchar_t buf[8];
    int ret;

    wmemset(buf, L'Q', 8);

    ret = swprintf(buf, 8, L"");

    assert(ret == 0);
    assert(buf[0] == L'\0');
}

/* [2] 等价于 fwprintf 的格式化能力：宽度、精度、十六进制等 */
static void test_formatting_equivalence(void)
{
    wchar_t buf[128];
    int ret;

    ret = swprintf(buf, 128, L"[%5d][%-5d][%05d][%x][%.3f]",
                   42, 42, 42, 255, 3.14159);

    assert(ret > 0);
    assert(wcscmp(buf, L"[   42][42   ][00042][ff][3.142]") == 0);
    assert(buf[ret] == L'\0');
}

/* [2] 宽字符参数 %lc / %ls 的处理 */
static void test_wide_args(void)
{
    wchar_t buf[64];
    int ret;

    ret = swprintf(buf, 64, L"%lc-%ls", L'A', L"wide");

    assert(ret == 6);
    assert(wcscmp(buf, L"A-wide") == 0);
}

/* [2] 返回值与 wcslen 一致（不含结尾空宽字符） */
static void test_return_matches_length(void)
{
    wchar_t buf[32];
    int ret;

    ret = swprintf(buf, 32, L"%d%d%d", 1, 2, 3);
    assert(ret == 3);
    assert((size_t)ret == wcslen(buf));
}

int main(void)
{
    /* 使用 C locale 即可，测试不依赖本地化字符集 */
    setlocale(LC_ALL, "C");

    /* [1] 原型检查 */
    assert(swprintf_proto == swprintf);

    test_basic_write();
    test_exact_fit();
    test_truncation_returns_negative();
    test_n_zero();
    test_empty_format();
    test_formatting_equivalence();
    test_wide_args();
    test_return_matches_length();

    printf("C99 7.24.2.3 swprintf: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「swprintf 的第一个参数类型为 wchar_t *」：
 * 传入 char * 而非 wchar_t *，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 1 ...）。 */
void neg_wrong_first_arg_type(void)
{
    char buf[64];
    swprintf(buf, 64, L"abc");   /* 期望：编译报错 */
}

/* 违反约束「swprintf 的第三个参数类型为 const wchar_t *」：
 * 传入 char * 格式串，gcc -std=c99 应报错。 */
void neg_wrong_format_type(void)
{
    wchar_t buf[64];
    swprintf(buf, 64, "abc");    /* 期望：编译报错 */
}

/* 违反约束「swprintf 的第二个参数类型为 size_t」：
 * 传入指针类型，gcc -std=c99 应报错（或至少产生诊断）。 */
void neg_wrong_n_type(void)
{
    wchar_t buf[64];
    int *p = 0;
    swprintf(buf, p, L"abc");    /* 期望：编译报错 */
}

/* 违反约束「swprintf 至少需要 3 个实参」：
 * 只传 2 个实参，gcc -std=c99 应报错（too few arguments）。 */
void neg_too_few_args(void)
{
    wchar_t buf[64];
    swprintf(buf, 64);           /* 期望：编译报错 */
}

/* 违反约束「swprintf 的返回类型为 int」：
 * 把返回值赋给结构体类型，gcc -std=c99 应报错。 */
struct S { int x; };
void neg_wrong_return_use(void)
{
    wchar_t buf[64];
    struct S s;
    s = swprintf(buf, 64, L"abc");  /* 期望：编译报错 */
}

#endif /* 负向测试结束 */