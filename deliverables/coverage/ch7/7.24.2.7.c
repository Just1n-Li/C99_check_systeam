/*
 * 测试条款：C99 7.24.2.7  The vswprintf function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdarg.h> <wchar.h>
 *   [2] 语义：等价于 swprintf，变参列表由 va_list arg 取代；
 *       arg 必须由 va_start 初始化（可能经过若干 va_arg）；
 *       vswprintf 不调用 va_end。
 *   [3] 返回值：写入的宽字符数（不含结尾空宽字符）；
 *       若发生编码错误，或请求生成 n 个及以上宽字符，则返回负值。
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>
#include <assert.h>
#include <locale.h>

/* ------------------------------------------------------------------ */
/* 辅助函数：把 vswprintf 包装起来，验证 [2] 中“变参列表由 va_list 取代” */
/* 以及“arg 由 va_start 初始化”的用法。                                  */
/* ------------------------------------------------------------------ */
static int
wrap_vswprintf(wchar_t *restrict s, size_t n,
               const wchar_t *restrict format, ...)
{
    va_list ap;
    int ret;

    va_start(ap, format);          /* [2] arg 由 va_start 初始化 */
    ret = vswprintf(s, n, format, ap); /* [1][2] 调用 vswprintf */
    /* [2] vswprintf 不调用 va_end，因此这里由调用者负责 */
    va_end(ap);
    return ret;
}

/* 再包一层：在 va_start 之后先做若干 va_arg，再传给 vswprintf，
 * 验证 [2] 中“possibly subsequent va_arg calls”的语义。 */
static int
wrap_vswprintf_after_vaarg(wchar_t *restrict s, size_t n,
                           const wchar_t *restrict format, ...)
{
    va_list ap;
    int ret;
    int first;

    va_start(ap, format);
    first = va_arg(ap, int);       /* [2] 先做一次 va_arg */
    ret = vswprintf(s, n, format, ap); /* 剩余参数交给 vswprintf */
    va_end(ap);
    (void)first;
    return ret;
}

int
main(void)
{
    /* 设置区域，使宽字符 I/O 与转换正常工作 */
    setlocale(LC_ALL, "C");

    /* ============================================================== */
    /* ========== 正向测试：以下代码应能编译并运行通过 ============== */
    /* ============================================================== */

    /* [1] 头文件与原型：直接声明一个函数指针，检查签名兼容性。
     *     若 <wchar.h> 未按 C99 提供该原型，此处会因类型不匹配而告警/报错。 */
    {
        int (*fp)(wchar_t *restrict, size_t, const wchar_t *restrict, va_list)
            = vswprintf;
        assert(fp != NULL);
    }

    /* [2][3] 基本用法：格式化写入，返回值 = 写入的宽字符数（不含结尾 L'\0'） */
    {
        wchar_t buf[64];
        int ret;

        ret = wrap_vswprintf(buf, 64, L"hello %ls %d", L"world", 42);
        /* "hello world 42" 共 14 个宽字符 */
        assert(ret == 14);
        assert(wcscmp(buf, L"hello world 42") == 0);
        /* 结尾空宽字符存在 */
        assert(buf[14] == L'\0');
    }

    /* [2] 在 va_start 之后先做 va_arg，再把剩余参数交给 vswprintf */
    {
        wchar_t buf[64];
        int ret;

        /* 第一个参数 7 被 va_arg 取走，剩余 "x=%d" 与 7 交给 vswprintf */
        ret = wrap_vswprintf_after_vaarg(buf, 64, L"x=%d", 7, 7);
        assert(ret == 3);              /* "x=7" */
        assert(wcscmp(buf, L"x=7") == 0);
    }

    /* [3] 返回值不含结尾空宽字符：空格式串写入 0 个字符 */
    {
        wchar_t buf[8];
        int ret;

        ret = wrap_vswprintf(buf, 8, L"");
        assert(ret == 0);
        assert(buf[0] == L'\0');
    }

    /* [3] 恰好写满 n-1 个字符（留出结尾空宽字符）应成功 */
    {
        wchar_t buf[6];                /* 可容纳 5 个字符 + L'\0' */
        int ret;

        ret = wrap_vswprintf(buf, 6, L"%ls", L"abcde");
        assert(ret == 5);
        assert(wcscmp(buf, L"abcde") == 0);
        assert(buf[5] == L'\0');
    }

    /* [3] 请求生成 n 个及以上宽字符时返回负值（缓冲区不足） */
    {
        wchar_t buf[4];                /* 只能容纳 3 个字符 + L'\0' */
        int ret;

        /* 需要写入 5 个字符，n=4，应返回负值 */
        ret = wrap_vswprintf(buf, 4, L"%ls", L"abcde");
        assert(ret < 0);
    }

    /* [3] n == 0 时，任何非空输出都请求了 >= n 个字符，应返回负值 */
    {
        wchar_t buf[1];
        int ret;

        ret = wrap_vswprintf(buf, 0, L"%ls", L"abc");
        assert(ret < 0);
    }

    /* [3] 编码错误：在 "C" 区域下，宽字符 > 0xFF 无法转换为多字节，
     *     使用 %ls 转换宽字符串时可能触发编码错误，返回负值。
     *     这里用 %lc 输出一个超出当前区域可表示范围的宽字符。 */
    {
        wchar_t buf[16];
        int ret;

        /* 0x110000 超出 Unicode 范围，属于无效宽字符，转换应失败 */
        ret = wrap_vswprintf(buf, 16, L"%lc", (wint_t)0x110000);
        assert(ret < 0);
    }

    /* [2] vswprintf 不调用 va_end：调用者可在 vswprintf 之后继续使用 ap
     *     （此处仅验证调用者自行 va_end 后程序正常结束，无崩溃）。 */
    {
        wchar_t buf[32];
        va_list ap;
        int ret;

        va_start(ap, buf);             /* 占位，实际用不到 format 参数 */
        va_end(ap);

        ret = wrap_vswprintf(buf, 32, L"%d-%d", 1, 2);
        assert(ret == 3);
        assert(wcscmp(buf, L"1-2") == 0);
    }

    printf("All positive tests passed.\n");

    /* ============================================================== */
    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ======== */
    /* ============================================================== */
#if 0

    /* 违反约束 [1]：vswprintf 的第 4 个参数类型必须是 va_list。
     * 传入 int 应编译报错（参数类型不兼容）。 */
    {
        wchar_t buf[16];
        int ret = vswprintf(buf, 16, L"%d", 42); /* 缺少 va_list，类型错误 */
        (void)ret;
    }

    /* 违反约束 [1]：第 1 个参数必须是 wchar_t *（restrict 限定）。
     * 传入 char * 应编译报错（指针类型不兼容）。 */
    {
        char cbuf[16];
        va_list ap;
        int ret = vswprintf(cbuf, 16, L"%d", ap); /* char* 与 wchar_t* 不兼容 */
        (void)ret;
    }

    /* 违反约束 [1]：第 3 个参数必须是 const wchar_t *。
     * 传入 char * 应编译报错。 */
    {
        wchar_t buf[16];
        va_list ap;
        int ret = vswprintf(buf, 16, "plain", ap); /* char* 与 wchar_t* 不兼容 */
        (void)ret;
    }

    /* 违反约束 [1]：第 2 个参数必须是 size_t。
     * 传入结构体类型应编译报错。 */
    {
        struct S { int x; } s;
        wchar_t buf[16];
        va_list ap;
        int ret = vswprintf(buf, s, L"%d", ap); /* 结构体不能转换为 size_t */
        (void)ret;
    }

    /* 违反约束 [1]：vswprintf 返回 int，不能赋值给结构体。 */
    {
        struct S { int x; } s;
        wchar_t buf[16];
        va_list ap;
        s = vswprintf(buf, 16, L"%d", ap); /* 类型不匹配 */
    }

#endif /* 负向测试结束 */

    return 0;
}