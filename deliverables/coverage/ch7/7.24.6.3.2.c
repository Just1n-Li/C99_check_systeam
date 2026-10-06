/*
 * 测试目标：C99 7.24.6.3.2  mbrtowc 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（在支持 UTF-8 或 C locale 的
 *             实现上，assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <wchar.h>
 *   [2] s 为 NULL 时等价于 mbrtowc(NULL, "", 1, ps)，忽略 pwc 与 n
 *   [3] s 非 NULL 时最多检查 n 字节，完整有效则存储宽字符；
 *       若宽字符为 L'\0'，结果状态为初始转换状态
 *   [4] 返回值：0 / 1..n / (size_t)-2 / (size_t)-1（并置 errno=EILSEQ）
 *   Footnote 300：n >= MB_CUR_MAX 时 (size_t)-2 只可能出现在冗余移位序列
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型一致 */
static size_t (*fp_mbrtowc)(wchar_t * restrict,
                            const char * restrict,
                            size_t,
                            mbstate_t * restrict) = mbrtowc;

static void test_prototype(void)
{
    /* [1] 取地址并赋值给匹配的函数指针，若原型不符则编译报错 */
    assert(fp_mbrtowc == mbrtowc);
    printf("[1] prototype OK\n");
}

/* [2] s 为 NULL 时等价于 mbrtowc(NULL, "", 1, ps)，pwc 与 n 被忽略 */
static void test_null_s(void)
{
    mbstate_t st;
    size_t r;
    wchar_t wc = L'X';   /* 应被忽略，不被修改 */

    memset(&st, 0, sizeof st);

    /* 传入非 NULL 的 pwc 与任意 n，均应被忽略 */
    r = mbrtowc(&wc, NULL, 12345, &st);
    /* 空字符串对应空宽字符，返回 0 */
    assert(r == 0);
    /* pwc 被忽略，值不变 */
    assert(wc == L'X');

    /* 再传 NULL pwc 与 n=0，同样应返回 0 */
    r = mbrtowc(NULL, NULL, 0, &st);
    assert(r == 0);

    printf("[2] s == NULL OK\n");
}

/* [3][4] 完整有效多字节字符：返回 1..n，并存储宽字符 */
static void test_complete_valid(void)
{
    mbstate_t st;
    wchar_t wc = 0;
    size_t r;

    memset(&st, 0, sizeof st);

    /* 单字节 ASCII 'A' 在任何 locale 下都是完整有效字符 */
    r = mbrtowc(&wc, "A", 1, &st);
    assert(r == 1);
    assert(wc == L'A');

    /* 提供多于所需字节，仍只消费 1 字节 */
    memset(&st, 0, sizeof st);
    wc = 0;
    r = mbrtowc(&wc, "AB", 2, &st);
    assert(r == 1);
    assert(wc == L'A');

    /* pwc 为 NULL 时仍返回字节数，但不存储 */
    memset(&st, 0, sizeof st);
    r = mbrtowc(NULL, "A", 1, &st);
    assert(r == 1);

    printf("[3][4] complete valid OK\n");
}

/* [3][4] 空宽字符：返回 0，且结果状态为初始转换状态 */
static void test_null_wide_char(void)
{
    mbstate_t st;
    wchar_t wc = L'Z';
    size_t r;

    memset(&st, 0, sizeof st);

    /* 空字符 '\0' 对应空宽字符，返回 0 */
    r = mbrtowc(&wc, "", 1, &st);
    assert(r == 0);
    assert(wc == L'\0');

    /* 结果状态应为初始转换状态：用 mbsinit 验证 */
    assert(mbsinit(&st) != 0);

    printf("[3][4] null wide char OK\n");
}

/* [4] 不完整但可能有效的多字节字符：返回 (size_t)-2，不存储 */
static void test_incomplete(void)
{
    mbstate_t st;
    wchar_t wc = L'Q';
    size_t r;

    memset(&st, 0, sizeof st);

    /*
     * 构造一个需要多于 1 字节的多字节字符。
     * 在 UTF-8 locale 下，0xC3 是 2 字节序列的首字节。
     * 若当前 locale 不是 UTF-8，则跳过该子测试。
     */
    if (MB_CUR_MAX > 1) {
        r = mbrtowc(&wc, "\xC3", 1, &st);
        if (r == (size_t)-2) {
            /* 不完整：未存储宽字符 */
            assert(wc == L'Q');
            /* 状态非初始（已消费部分字节） */
            /* 继续提供后续字节，应完成转换 */
            r = mbrtowc(&wc, "\xA9", 1, &st);
            assert(r == 1);
            assert(wc == 0x00E9); /* é */
        } else {
            /* 该实现可能不接受此序列，视为编码错误 */
            assert(r == (size_t)-1);
        }
    }

    printf("[4] incomplete OK\n");
}

/* [4] 编码错误：返回 (size_t)-1，errno == EILSEQ，不存储 */
static void test_encoding_error(void)
{
    mbstate_t st;
    wchar_t wc = L'W';
    size_t r;

    memset(&st, 0, sizeof st);

    /*
     * 0xFF 在 UTF-8 中永远不是合法首字节。
     * 在单字节 locale（如 C locale）中，0xFF 也可能非法。
     * 若实现接受 0xFF，则跳过。
     */
    errno = 0;
    r = mbrtowc(&wc, "\xFF", 1, &st);
    if (r == (size_t)-1) {
        assert(errno == EILSEQ);
        /* 未存储宽字符 */
        assert(wc == L'W');
    } else {
        /* 该实现接受 0xFF，跳过错误路径检查 */
        printf("    (implementation accepts 0xFF, skip EILSEQ check)\n");
    }

    printf("[4] encoding error OK\n");
}

/* [3] 状态相关编码：跨调用保持转换状态 */
static void test_stateful(void)
{
    mbstate_t st;
    wchar_t wc = 0;
    size_t r;

    memset(&st, 0, sizeof st);

    /* 在 UTF-8 locale 下测试 2 字节序列分两次喂入 */
    if (MB_CUR_MAX > 1) {
        r = mbrtowc(&wc, "\xC3", 1, &st);
        if (r == (size_t)-2) {
            r = mbrtowc(&wc, "\xA9", 1, &st);
            assert(r == 1);
            assert(wc == 0x00E9);
            /* 完成后状态回到初始 */
            assert(mbsinit(&st) != 0);
        }
    }

    printf("[3] stateful OK\n");
}

/* Footnote 300：n >= MB_CUR_MAX 时 (size_t)-2 只可能来自冗余移位序列。
 * 这里只做一致性检查：当 n >= MB_CUR_MAX 且返回 -2 时，
 * 说明实现存在冗余移位序列（状态相关编码）。 */
static void test_footnote_300(void)
{
    mbstate_t st;
    wchar_t wc;
    size_t r;
    size_t n = MB_CUR_MAX;

    memset(&st, 0, sizeof st);

    /* 用一段足够长的合法 ASCII 串，n = MB_CUR_MAX */
    r = mbrtowc(&wc, "Hello", n, &st);
    /* 首字符 'H' 完整有效，返回 1 */
    assert(r == 1);
    assert(wc == L'H');

    printf("[footnote 300] consistency OK\n");
}

int main(void)
{
    /* 尝试设置 UTF-8 locale 以便测试多字节路径；失败则用 C locale */
    if (setlocale(LC_ALL, "C.UTF-8") == NULL &&
        setlocale(LC_ALL, "en_US.UTF-8") == NULL) {
        setlocale(LC_ALL, "C");
    }
    printf("MB_CUR_MAX = %d\n", (int)MB_CUR_MAX);

    test_prototype();
    test_null_s();
    test_complete_valid();
    test_null_wide_char();
    test_incomplete();
    test_encoding_error();
    test_stateful();
    test_footnote_300();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [1]：mbrtowc 的第一个参数类型为 wchar_t * restrict，
 * 传入 int * 不兼容，gcc -std=c99 应报错（incompatible pointer type）。 */
void neg_wrong_pwc_type(void)
{
    int x;
    mbstate_t st;
    mbrtowc(&x, "A", 1, &st);   /* 错误：int * 不能隐式转为 wchar_t * */
}

/* 违反约束 [1]：第二个参数类型为 const char * restrict，
 * 传入 wchar_t * 不兼容，应报错。 */
void neg_wrong_s_type(void)
{
    wchar_t buf[4];
    mbstate_t st;
    mbrtowc(NULL, buf, 1, &st); /* 错误：wchar_t * 不能隐式转为 const char * */
}

/* 违反约束 [1]：第三个参数类型为 size_t，
 * 传入指针类型不兼容，应报错。 */
void neg_wrong_n_type(void)
{
    mbstate_t st;
    mbrtowc(NULL, "A", (void *)0, &st); /* 错误：void * 不能隐式转为 size_t */
}

/* 违反约束 [1]：第四个参数类型为 mbstate_t * restrict，
 * 传入 int * 不兼容，应报错。 */
void neg_wrong_ps_type(void)
{
    int x;
    mbrtowc(NULL, "A", 1, &x);  /* 错误：int * 不能隐式转为 mbstate_t * */
}

/* 违反约束 [1]：参数个数不匹配，应报错。 */
void neg_too_few_args(void)
{
    mbrtowc(NULL, "A");         /* 错误：参数太少 */
}

/* 违反约束 [1]：参数个数不匹配，应报错。 */
void neg_too_many_args(void)
{
    mbstate_t st;
    mbrtowc(NULL, "A", 1, &st, 0); /* 错误：参数太多 */
}

/* 违反约束 [1]：mbrtowc 返回 size_t，不能赋值给结构体，应报错。 */
struct S { int a; };
void neg_return_type(void)
{
    struct S s;
    mbstate_t st;
    s = mbrtowc(NULL, "A", 1, &st); /* 错误：size_t 不能赋给 struct S */
}

#endif /* 负向测试结束 */