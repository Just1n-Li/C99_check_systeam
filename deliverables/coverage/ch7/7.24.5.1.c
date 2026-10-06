/*
 * 测试条款：C99 7.24.5.1  wcsftime 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <time.h> <wchar.h> / restrict 限定
 *   [2] 语义：等价于 strftime，但 s 为宽字符数组、maxsize 为宽字符数、
 *       format 为宽字符串、返回值为宽字符数
 *   [3] 返回值：结果（含结尾空宽字符）<= maxsize 时返回写入的宽字符数
 *       （不含结尾空宽字符）；否则返回 0，数组内容不确定
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型一致。
 *     注意 restrict 限定符在函数类型中不参与兼容性判断，
 *     但参数类型/返回类型必须匹配。 */
static size_t (*fp_wcsftime)(wchar_t * restrict,
                             size_t,
                             const wchar_t * restrict,
                             const struct tm * restrict) = wcsftime;

/* [2] 语义测试：wcsftime 等价于 strftime，但操作宽字符。
 *     用同一 struct tm 分别调用 strftime 与 wcsftime，
 *     比较两者产生的字符序列（逐字符相等）。 */
static void test_equivalence_with_strftime(void)
{
    struct tm tmv;
    char      mbuf[256];
    wchar_t   wbuf[256];
    size_t    n1, n2;
    size_t    i;

    memset(&tmv, 0, sizeof tmv);
    tmv.tm_year = 2001 - 1900;   /* 2001 年 */
    tmv.tm_mon  = 0;             /* 1 月 */
    tmv.tm_mday = 2;             /* 2 日 */
    tmv.tm_hour = 3;
    tmv.tm_min  = 4;
    tmv.tm_sec  = 5;
    tmv.tm_wday = 2;             /* 星期二 */
    tmv.tm_yday = 1;

    /* 使用一个确定性的格式串，避免依赖本地化差异 */
    n1 = strftime(mbuf, sizeof mbuf, "%Y-%m-%d %H:%M:%S", &tmv);
    n2 = wcsftime(wbuf, sizeof wbuf / sizeof wbuf[0],
                  L"%Y-%m-%d %H:%M:%S", &tmv);

    /* [2] 返回值指示宽字符数，应与 strftime 的字节数一致（ASCII 情形） */
    assert(n1 == n2);
    assert(n1 == 19);            /* "2001-01-02 03:04:05" 共 19 个字符 */

    /* [2] 逐字符比较：wcsftime 输出应与 strftime 输出相同 */
    for (i = 0; i < n1; i++) {
        assert((unsigned char)mbuf[i] == (unsigned char)wbuf[i]);
    }
    /* [3] 结尾空宽字符必须存在 */
    assert(wbuf[n1] == L'\0');
}

/* [3] 返回值测试：结果（含结尾空宽字符）<= maxsize 时，
 *     返回写入的宽字符数（不含结尾空宽字符）。 */
static void test_return_value_fits(void)
{
    struct tm tmv;
    wchar_t   buf[64];
    size_t    n;

    memset(&tmv, 0, sizeof tmv);
    tmv.tm_year = 1999 - 1900;
    tmv.tm_mon  = 11;            /* 12 月 */
    tmv.tm_mday = 31;
    tmv.tm_hour = 23;
    tmv.tm_min  = 59;
    tmv.tm_sec  = 58;

    /* 格式串 "%Y" 产生 4 个宽字符 + 结尾空宽字符 = 5 <= 64 */
    n = wcsftime(buf, 64, L"%Y", &tmv);
    assert(n == 4);              /* 不含结尾空宽字符 */
    assert(buf[0] == L'1');
    assert(buf[1] == L'9');
    assert(buf[2] == L'9');
    assert(buf[3] == L'9');
    assert(buf[4] == L'\0');     /* 结尾空宽字符 */

    /* 恰好放得下：maxsize == 结果长度 + 1 */
    n = wcsftime(buf, 5, L"%Y", &tmv);
    assert(n == 4);
    assert(buf[4] == L'\0');
}

/* [3] 返回值测试：结果（含结尾空宽字符）> maxsize 时返回 0，
 *     且数组内容不确定（此处只检查返回值，不检查内容）。 */
static void test_return_value_too_small(void)
{
    struct tm tmv;
    wchar_t   buf[64];
    size_t    n;

    memset(&tmv, 0, sizeof tmv);
    tmv.tm_year = 1999 - 1900;
    tmv.tm_mon  = 11;
    tmv.tm_mday = 31;

    /* "%Y" 需要 4 + 1 = 5 个宽字符，maxsize = 4 放不下 */
    n = wcsftime(buf, 4, L"%Y", &tmv);
    assert(n == 0);

    /* maxsize = 0 也放不下（连结尾空宽字符都放不下） */
    n = wcsftime(buf, 0, L"%Y", &tmv);
    assert(n == 0);

    /* maxsize = 1 仍放不下（需要 5） */
    n = wcsftime(buf, 1, L"%Y", &tmv);
    assert(n == 0);
}

/* [2] 空格式串：结果只有结尾空宽字符，长度 1 <= maxsize，
 *     返回 0（不含结尾空宽字符的宽字符数）。 */
static void test_empty_format(void)
{
    struct tm tmv;
    wchar_t   buf[8];
    size_t    n;

    memset(&tmv, 0, sizeof tmv);
    n = wcsftime(buf, 8, L"", &tmv);
    assert(n == 0);
    assert(buf[0] == L'\0');
}

/* [2] 宽字符输出：使用包含非 ASCII 宽字符的格式串，
 *     验证输出确实是宽字符序列（而非窄字节序列）。 */
static void test_wide_characters(void)
{
    struct tm tmv;
    wchar_t   buf[32];
    size_t    n;

    memset(&tmv, 0, sizeof tmv);
    tmv.tm_year = 2000 - 1900;
    tmv.tm_mon  = 0;
    tmv.tm_mday = 1;

    /* 格式串中直接嵌入宽字符 L'年'、L'月'、L'日' */
    n = wcsftime(buf, 32, L"%Y年%m月%d日", &tmv);
    assert(n == 11);             /* "2000年01月01日" 共 11 个宽字符 */
    assert(buf[4] == L'年');
    assert(buf[7] == L'月');
    assert(buf[10] == L'日');
    assert(buf[11] == L'\0');
}

/* [1] restrict 限定：s 与 format 指向不同对象，符合 restrict 语义。
 *     这里仅验证正常调用不触发问题。 */
static void test_restrict_distinct_objects(void)
{
    struct tm tmv;
    wchar_t   buf[32];
    const wchar_t *fmt = L"%Y";
    size_t    n;

    memset(&tmv, 0, sizeof tmv);
    tmv.tm_year = 2020 - 1900;

    n = wcsftime(buf, 32, fmt, &tmv);
    assert(n == 4);
    assert(buf[0] == L'2');
    assert(buf[1] == L'0');
    assert(buf[2] == L'2');
    assert(buf[3] == L'0');
}

int main(void)
{
    /* 使用 "C" 区域设置，保证 strftime/wcsftime 输出可预测 */
    setlocale(LC_ALL, "C");

    /* [1] 原型检查 */
    assert(fp_wcsftime == wcsftime);

    /* [2] 与 strftime 等价性 */
    test_equivalence_with_strftime();

    /* [3] 返回值：放得下 */
    test_return_value_fits();

    /* [3] 返回值：放不下 */
    test_return_value_too_small();

    /* [2] 空格式串 */
    test_empty_format();

    /* [2] 宽字符输出 */
    test_wide_characters();

    /* [1] restrict 语义 */
    test_restrict_distinct_objects();

    printf("wcsftime (C99 7.24.5.1) positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcsftime 的第一个参数类型为 wchar_t *」：
 * 传入 char * 而非 wchar_t *，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 1 ...）。 */
void neg_wrong_first_arg_type(void)
{
    char buf[64];
    struct tm tmv;
    wcsftime(buf, 64, L"%Y", &tmv);   /* 错误：buf 应为 wchar_t * */
}

/* 违反约束「wcsftime 的第三个参数类型为 const wchar_t *」：
 * 传入窄字符串字面量（char *），gcc -std=c99 应报错。 */
void neg_wrong_format_type(void)
{
    wchar_t buf[64];
    struct tm tmv;
    wcsftime(buf, 64, "%Y", &tmv);    /* 错误：格式串应为宽字符串 */
}

/* 违反约束「wcsftime 的第四个参数类型为 const struct tm *」：
 * 传入 int *，gcc -std=c99 应报错。 */
void neg_wrong_timeptr_type(void)
{
    wchar_t buf[64];
    int x = 0;
    wcsftime(buf, 64, L"%Y", &x);     /* 错误：&x 应为 const struct tm * */
}

/* 违反约束「wcsftime 的第二个参数类型为 size_t」：
 * 传入指针，gcc -std=c99 应报错。 */
void neg_wrong_maxsize_type(void)
{
    wchar_t buf[64];
    struct tm tmv;
    wcsftime(buf, buf, L"%Y", &tmv);  /* 错误：maxsize 应为 size_t */
}

/* 违反约束「wcsftime 返回 size_t」：
 * 把返回值赋给不兼容的指针类型，gcc -std=c99 应报错。 */
void neg_wrong_return_use(void)
{
    wchar_t buf[64];
    struct tm tmv;
    int *p = wcsftime(buf, 64, L"%Y", &tmv);  /* 错误：size_t 不能初始化 int * */
    (void)p;
}

/* 违反约束「调用 wcsftime 必须提供 4 个实参」：
 * 实参个数不足，gcc -std=c99 应报错。 */
void neg_too_few_args(void)
{
    wchar_t buf[64];
    struct tm tmv;
    wcsftime(buf, 64, L"%Y");         /* 错误：缺少第 4 个实参 */
    (void)tmv;
}

/* 违反约束「调用 wcsftime 必须提供 4 个实参」：
 * 实参个数过多，gcc -std=c99 应报错。 */
void neg_too_many_args(void)
{
    wchar_t buf[64];
    struct tm tmv;
    wcsftime(buf, 64, L"%Y", &tmv, 0); /* 错误：实参过多 */
}

#endif /* 负向测试结束 */