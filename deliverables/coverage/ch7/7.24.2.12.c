/*
 * 测试条款：C99 7.24.2.12  The wscanf function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int wscanf(const wchar_t * restrict format, ...);
 *   [2] 语义：等价于 fwscanf(stdin, format, ...)
 *   [3] 返回值：输入失败且未发生任何转换时返回 EOF；
 *       否则返回成功赋值的输入项个数（可能少于提供的项数，甚至为 0）。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 原型检查：wscanf 的声明必须与标准一致。
 *     通过取函数指针并赋值来静态验证签名：
 *     int (*)(const wchar_t * restrict, ...)
 *     若原型不符，此处会编译报错或警告。 */
static int (*wscanf_proto_check)(const wchar_t * restrict, ...) = wscanf;

/* 辅助：把宽字符串写入临时文件，然后重定向 stdin 读取它。
 * 由于 wscanf 固定从 stdin 读取，我们用 freopen 把 stdin 指向文件。 */
static FILE *make_stdin_from_wide(const char *fname, const char *content)
{
    FILE *fp = fopen(fname, "w");
    assert(fp != NULL);
    fputs(content, fp);
    fclose(fp);
    /* 将 stdin 重定向到该文件 */
    FILE *in = freopen(fname, "r", stdin);
    assert(in != NULL);
    return in;
}

int main(void)
{
    /* 设置区域，使宽字符 I/O 正常工作 */
    setlocale(LC_ALL, "C");

    /* 引用原型检查变量，避免未使用警告 */
    assert(wscanf_proto_check == wscanf);

    /* ------------------------------------------------------------
     * [2] 语义：wscanf 等价于 fwscanf(stdin, format, ...)
     *     用同一输入分别通过 wscanf 与 fwscanf(stdin,...) 读取，
     *     验证二者行为一致（读取相同的值）。
     * ------------------------------------------------------------ */

    /* 准备输入文件内容："42 3.5 hello" */
    const char *fname = "wscanf_test_input.txt";

    /* --- 用 wscanf 读取 --- */
    make_stdin_from_wide(fname, "42 3.5 hello");
    {
        int    i1 = 0;
        double d1 = 0.0;
        wchar_t s1[32];
        memset(s1, 0, sizeof(s1));

        /* [1] 使用宽字符格式串，可变参数 */
        int n1 = wscanf(L"%d %lf %ls", &i1, &d1, s1);

        /* [3] 成功赋值 3 项，返回 3 */
        assert(n1 == 3);
        assert(i1 == 42);
        assert(d1 == 3.5);
        assert(wcscmp(s1, L"hello") == 0);
    }

    /* --- 用 fwscanf(stdin, ...) 读取相同内容，验证等价性 --- */
    make_stdin_from_wide(fname, "42 3.5 hello");
    {
        int    i2 = 0;
        double d2 = 0.0;
        wchar_t s2[32];
        memset(s2, 0, sizeof(s2));

        int n2 = fwscanf(stdin, L"%d %lf %ls", &i2, &d2, s2);

        assert(n2 == 3);
        assert(i2 == 42);
        assert(d2 == 3.5);
        assert(wcscmp(s2, L"hello") == 0);
    }

    /* ------------------------------------------------------------
     * [3] 返回值：成功赋值的项数
     * ------------------------------------------------------------ */

    /* 情形 A：全部匹配，返回项数 = 提供的项数 */
    make_stdin_from_wide(fname, "7 8");
    {
        int a = 0, b = 0;
        int n = wscanf(L"%d %d", &a, &b);
        assert(n == 2);
        assert(a == 7 && b == 8);
    }

    /* 情形 B：早期匹配失败，返回 0（未赋值任何项） */
    make_stdin_from_wide(fname, "abc");
    {
        int a = -1;
        int n = wscanf(L"%d", &a);
        /* 输入 "abc" 无法匹配 %d，早期匹配失败，返回 0 */
        assert(n == 0);
        /* a 未被赋值，保持原值 */
        assert(a == -1);
    }

    /* 情形 C：部分匹配，返回已赋值的项数（少于提供的项数） */
    make_stdin_from_wide(fname, "10 xyz");
    {
        int a = 0, b = 0;
        int n = wscanf(L"%d %d", &a, &b);
        /* 第一个 %d 成功，第二个 %d 遇到 "xyz" 失败 */
        assert(n == 1);
        assert(a == 10);
        assert(b == 0); /* b 未被赋值 */
    }

    /* 情形 D：输入失败（空输入 / 立即 EOF），返回 EOF */
    make_stdin_from_wide(fname, "");
    {
        int a = 0;
        int n = wscanf(L"%d", &a);
        /* 在任何转换之前发生输入失败，返回 EOF */
        assert(n == EOF);
    }

    /* 情形 E：格式串无转换说明符，返回 0（无项被赋值） */
    make_stdin_from_wide(fname, "anything");
    {
        int n = wscanf(L"literal");
        /* 没有转换说明符，成功赋值的项数为 0 */
        assert(n == 0);
    }

    /* 清理临时文件 */
    remove(fname);

    printf("All positive tests for C99 7.24.2.12 (wscanf) passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「wscanf 的第一个参数类型必须是 const wchar_t *」：
 * 传入 char* 而非 wchar_t*，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 1 ...）。 */
#include <wchar.h>
void bad1(void) {
    char fmt[] = "%d";
    int x;
    wscanf(fmt, &x);   /* 错误：应为 const wchar_t * */
}

/* 违反约束「wscanf 声明于 <wchar.h>，且返回 int」：
 * 若把返回值当作指针使用，类型不匹配，应报错。 */
#include <wchar.h>
void bad2(void) {
    wchar_t *p = wscanf(L"%d");  /* 错误：int 不能初始化 wchar_t* */
    (void)p;
}

/* 违反约束「wscanf 是函数，需要可调用的声明」：
 * 未包含 <wchar.h> 时调用 wscanf，C99 中隐式函数声明为错误
 * （gcc -std=c99 -Werror=implicit-function-declaration 应报错）。 */
void bad3(void) {
    int x;
    wscanf(L"%d", &x);  /* 错误：无 wscanf 声明 */
}

/* 违反约束「格式串参数必须为 const wchar_t *」：
 * 传入整数常量作为格式串，类型不兼容，应报错。 */
#include <wchar.h>
void bad4(void) {
    int x;
    wscanf(123, &x);   /* 错误：int 不能转换为 const wchar_t * */
}

#endif /* 负向测试结束 */