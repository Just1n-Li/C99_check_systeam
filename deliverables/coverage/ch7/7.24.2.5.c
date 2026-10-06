/*
 * 测试条款：C99 7.24.2.5  The vfwprintf function
 *
 * 预期行为：
 *   正向测试：以下使用 vfwprintf 的代码应能编译并正确运行，
 *             返回值应为已传输的宽字符数（非负），出错时为负值。
 *   负向测试：违反约束的代码（如参数类型不匹配、缺少头文件声明等）
 *             应导致编译报错。所有负向片段放在 #if 0 ... #endif 中，
 *             保证本文件整体仍可编译运行。
 *
 * 覆盖段落：
 *   [1] Synopsis：需要 <stdarg.h> <stdio.h> <wchar.h>，原型
 *       int vfwprintf(FILE * restrict stream,
 *                     const wchar_t * restrict format,
 *                     va_list arg);
 *   [2] Description：等价于 fwprintf，但可变参数列表由 arg 替换；
 *       arg 必须由 va_start 初始化（可能经过若干 va_arg 调用）；
 *       vfwprintf 不调用 va_end。
 *   [3] Returns：返回传输的宽字符数；输出或编码错误时返回负值。
 *   [4] EXAMPLE：通用错误报告例程。
 *   Footnote 291：调用后 arg 的值不确定（不测试其值，仅说明）。
 */

#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [4] EXAMPLE 中的通用错误报告例程（稍作调整以便测试） */
static int error_report(FILE *stream, const char *function_name,
                        const wchar_t *format, ...)
{
    va_list args;
    int n1, n2;

    va_start(args, format);
    /* 打印出错函数名 */
    n1 = fwprintf(stream, L"ERROR in %s: ", function_name);
    /* 打印消息剩余部分 */
    n2 = vfwprintf(stream, format, args);
    va_end(args);

    if (n1 < 0 || n2 < 0)
        return -1;
    return n1 + n2;
}

/* [2] 验证 vfwprintf 等价于 fwprintf：用同一格式与参数分别输出到
 *     两个内存流，比较结果字符串是否一致。 */
static void test_equivalence(void)
{
    wchar_t buf1[256];
    wchar_t buf2[256];
    FILE *f1 = NULL, *f2 = NULL;
    va_list ap;
    int r1, r2;

    /* 使用 tmpfile 作为可写流 */
    f1 = tmpfile();
    f2 = tmpfile();
    assert(f1 != NULL && f2 != NULL);

    /* 用 fwprintf 直接输出 */
    r1 = fwprintf(f1, L"%d %s %c %ls", 42, "hello", L'X', L"world");
    assert(r1 >= 0);

    /* 用 vfwprintf 通过 va_list 输出相同内容 */
    {
        /* 构造一个辅助函数来调用 vfwprintf */
        /* 这里直接内联：需要 va_start，故放在一个可变参数函数中 */
    }

    /* 由于 va_start 必须在可变参数函数内使用，这里用一个辅助函数 */
    /* 见下方 helper_vfwprintf */

    /* 关闭并重新打开以读取 */
    fflush(f1);
    fflush(f2);

    /* 读取 f1 内容 */
    rewind(f1);
    {
        size_t i = 0;
        wint_t wc;
        while ((wc = fgetwc(f1)) != WEOF && i < 255)
            buf1[i++] = (wchar_t)wc;
        buf1[i] = L'\0';
    }

    /* 用 vfwprintf 写入 f2 */
    r2 = helper_vfwprintf(f2, L"%d %s %c %ls", 42, "hello", L'X', L"world");
    assert(r2 >= 0);
    assert(r2 == r1);  /* [3] 返回值应相同 */

    fflush(f2);
    rewind(f2);
    {
        size_t i = 0;
        wint_t wc;
        while ((wc = fgetwc(f2)) != WEOF && i < 255)
            buf2[i++] = (wchar_t)wc;
        buf2[i] = L'\0';
    }

    /* [2] 等价性：输出内容应一致 */
    assert(wcscmp(buf1, buf2) == 0);

    fclose(f1);
    fclose(f2);
}

/* 辅助函数：在可变参数函数内使用 va_start 后调用 vfwprintf */
static int helper_vfwprintf(FILE *stream, const wchar_t *format, ...)
{
    va_list ap;
    int r;
    va_start(ap, format);
    r = vfwprintf(stream, format, ap);
    /* [2] vfwprintf 不调用 va_end，由调用者负责 */
    va_end(ap);
    return r;
}

/* [3] 验证返回值：正常输出时返回非负值 */
static void test_return_value(void)
{
    FILE *f = tmpfile();
    int r;
    assert(f != NULL);

    r = helper_vfwprintf(f, L"abc");
    assert(r == 3);  /* 3 个宽字符 */

    r = helper_vfwprintf(f, L"%d", 12345);
    assert(r == 5);  /* "12345" 5 个宽字符 */

    r = helper_vfwprintf(f, L"");
    assert(r == 0);  /* 空格式串，0 个宽字符 */

    fclose(f);
}

/* [2] 验证 arg 可由 va_start 初始化并经过若干 va_arg 调用后传入 */
static int test_va_arg_then_vfwprintf(FILE *stream, const wchar_t *fmt, ...)
{
    va_list ap;
    int first;
    int r;

    va_start(ap, fmt);
    /* 先取一个参数 */
    first = va_arg(ap, int);
    /* 再用剩余的 arg 调用 vfwprintf */
    r = vfwprintf(stream, fmt, ap);
    va_end(ap);

    return (first == 100) ? r : -1;
}

static void test_va_arg_usage(void)
{
    FILE *f = tmpfile();
    int r;
    assert(f != NULL);

    /* 第一个参数 100 被 va_arg 取走，剩余参数用于 vfwprintf */
    r = test_va_arg_then_vfwprintf(f, L"%d %d", 100, 7, 8);
    assert(r == 3);  /* "7 8" 共 3 个宽字符 */

    fclose(f);
}

/* [4] 测试 EXAMPLE 中的 error 函数 */
static void test_example(void)
{
    FILE *f = tmpfile();
    int r;
    assert(f != NULL);

    r = error_report(f, "my_func", L"value=%d, name=%ls", 99, L"test");
    assert(r > 0);

    fclose(f);
}

int main(void)
{
    /* 设置 locale 以支持宽字符输出 */
    setlocale(LC_ALL, "");

    test_equivalence();
    test_return_value();
    test_va_arg_usage();
    test_example();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「vfwprintf 的第三个参数类型必须为 va_list」：
 * 传入 int 而非 va_list，gcc -std=c99 应报错
 * （类型不兼容，参数类型错误）。 */
void bad_arg_type(void)
{
    FILE *f = tmpfile();
    int x = 0;
    vfwprintf(f, L"%d", x);  /* 错误：第三个参数应为 va_list */
}

/* 违反约束「vfwprintf 的第二个参数类型必须为 const wchar_t *」：
 * 传入 char * 而非 wchar_t *，gcc -std=c99 应报错
 * （指针类型不兼容）。 */
void bad_format_type(void)
{
    FILE *f = tmpfile();
    va_list ap;
    va_start(ap, f);
    vfwprintf(f, "hello", ap);  /* 错误：格式串应为 wchar_t * */
    va_end(ap);
}

/* 违反约束「vfwprintf 的第一个参数类型必须为 FILE *」：
 * 传入 int，gcc -std=c99 应报错。 */
void bad_stream_type(void)
{
    va_list ap;
    int x = 0;
    va_start(ap, x);
    vfwprintf(0, L"hello", ap);  /* 错误：第一个参数应为 FILE * */
    va_end(ap);
}

/* 违反约束「调用 vfwprintf 前必须包含 <wchar.h> 声明」：
 * 若未包含 <wchar.h>，vfwprintf 未声明，C99 下调用未声明函数
 * 为约束违反（C99 6.5.2.2），gcc -std=c99 应报错或警告。 */
/* 注：此处已在文件顶部包含 <wchar.h>，故用注释说明；
 * 实际测试时若移除 #include <wchar.h> 则编译报错。 */

/* 违反约束「vfwprintf 的返回类型为 int，不能赋值给不兼容类型」：
 * 将返回值赋给 struct 类型，gcc -std=c99 应报错。 */
struct S { int x; };
void bad_return_assign(void)
{
    FILE *f = tmpfile();
    va_list ap;
    struct S s;
    va_start(ap, f);
    s = vfwprintf(f, L"x", ap);  /* 错误：int 不能赋给 struct S */
    va_end(ap);
}

#endif /* 负向测试结束 */