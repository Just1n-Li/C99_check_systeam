/*
 * 测试条款：C99 7.24.2.2  The fwscanf function
 *
 * 预期行为：
 *   正向测试：以下使用 fwscanf 的代码应能编译并正确运行（assert 通过）。
 *   负向测试：违反约束的代码片段应导致编译报错（放在 #if 0 中，保证本文件仍可编译）。
 *
 * 说明：fwscanf 的许多“约束”实际上是“shall be a pointer to ...”这类要求，
 *       违反它们属于未定义行为（UB），编译器不强制报错，因此不作为负向测试。
 *       本文件负向测试只针对真正的约束（如参数类型、restrict 限定等可诊断项）。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

/* 辅助：把宽字符串写入临时文件，返回 FILE*（已 rewind） */
static FILE *make_wfile(const wchar_t *ws)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);
    /* 用 fputws 写入宽字符 */
    if (fputws(ws, fp) == -1) {
        fclose(fp);
        return NULL;
    }
    rewind(fp);
    return fp;
}

int main(void)
{
    /* 设置 locale 以便宽字符 I/O 正常工作 */
    setlocale(LC_ALL, "");

    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型：int fwscanf(FILE * restrict, const wchar_t * restrict, ...);
     *     验证函数存在且返回 int。 */
    {
        FILE *fp = make_wfile(L"42");
        assert(fp != NULL);
        int x = 0;
        int r = fwscanf(fp, L"%d", &x);   /* [1] 基本调用 */
        assert(r == 1);
        assert(x == 42);
        fclose(fp);
    }

    /* [2] 从 stream 读取，按 format 转换并赋值给后续参数指向的对象。
     *     多余参数被求值但忽略。 */
    {
        FILE *fp = make_wfile(L"7 8");
        assert(fp != NULL);
        int a = 0, b = 0, extra = 0;
        int r = fwscanf(fp, L"%d %d", &a, &b, &extra); /* extra 多余，被忽略 */
        assert(r == 2);
        assert(a == 7 && b == 8);
        fclose(fp);
    }

    /* [3] format 由指令组成：空白、普通宽字符、转换规格。
     *     转换规格：% [*] [宽度] [长度修饰] 转换说明符 */
    {
        FILE *fp = make_wfile(L"abc 123");
        assert(fp != NULL);
        wchar_t buf[16] = {0};
        int n = 0;
        /* 普通宽字符 'a','b','c' + 空白 + %ls + %n */
        int r = fwscanf(fp, L"abc %ls%n", buf, &n);
        assert(r == 1);
        assert(wcscmp(buf, L"123") == 0);
        assert(n > 0);
        fclose(fp);
    }

    /* [3] 赋值抑制 * ：不赋值，不增加返回计数 */
    {
        FILE *fp = make_wfile(L"99 100");
        assert(fp != NULL);
        int y = 0;
        int r = fwscanf(fp, L"%*d %d", &y);  /* 抑制第一个 */
        assert(r == 1);
        assert(y == 100);
        fclose(fp);
    }

    /* [3] 最大字段宽度 */
    {
        FILE *fp = make_wfile(L"12345");
        assert(fp != NULL);
        int v = 0;
        int r = fwscanf(fp, L"%3d", &v);   /* 只读 3 个宽字符 */
        assert(r == 1);
        assert(v == 123);
        fclose(fp);
    }

    /* [5] 空白指令：跳过前导空白，停在第一个非空白（不读走） */
    {
        FILE *fp = make_wfile(L"   \t\n  55");
        assert(fp != NULL);
        int v = 0;
        int r = fwscanf(fp, L"   %d", &v);  /* 空白指令 */
        assert(r == 1);
        assert(v == 55);
        fclose(fp);
    }

    /* [6] 普通宽字符指令：匹配则继续，不匹配则失败且字符保留 */
    {
        FILE *fp = make_wfile(L"X 5");
        assert(fp != NULL);
        int v = 0;
        int r = fwscanf(fp, L"X %d", &v);   /* 'X' 匹配 */
        assert(r == 1);
        assert(v == 5);
        fclose(fp);
    }
    {
        FILE *fp = make_wfile(L"Y 5");
        assert(fp != NULL);
        int v = 0;
        int r = fwscanf(fp, L"X %d", &v);   /* 'X' 不匹配 -> 匹配失败 */
        assert(r == 0);                     /* 返回 0，未赋值 */
        fclose(fp);
    }

    /* [8] 除 [, c, n 外，跳过输入空白 */
    {
        FILE *fp = make_wfile(L"    \t  77");
        assert(fp != NULL);
        int v = 0;
        int r = fwscanf(fp, L"%d", &v);     /* %d 前自动跳过空白 */
        assert(r == 1);
        assert(v == 77);
        fclose(fp);
    }

    /* [8] %c 不跳过空白 */
    {
        FILE *fp = make_wfile(L" A");
        assert(fp != NULL);
        wchar_t c = 0;
        int r = fwscanf(fp, L"%lc", &c);    /* 读到空格 */
        assert(r == 1);
        assert(c == L' ');
        fclose(fp);
    }

    /* [9] 输入项为最长匹配序列；长度为零 -> 匹配失败 */
    {
        FILE *fp = make_wfile(L"abc");
        assert(fp != NULL);
        int v = 0;
        int r = fwscanf(fp, L"%d", &v);     /* 无数字可读 -> 匹配失败 */
        assert(r == 0);
        fclose(fp);
    }

    /* [9] 输入项后第一个宽字符保留未读 */
    {
        FILE *fp = make_wfile(L"12ab");
        assert(fp != NULL);
        int v = 0;
        wchar_t c = 0;
        int r1 = fwscanf(fp, L"%d", &v);    /* 读 12，'a' 保留 */
        assert(r1 == 1 && v == 12);
        int r2 = fwscanf(fp, L"%lc", &c);   /* 读 'a' */
        assert(r2 == 1 && c == L'a');
        fclose(fp);
    }

    /* [10] %n 不消耗输入，只记录已读宽字符数 */
    {
        FILE *fp = make_wfile(L"123");
        assert(fp != NULL);
        int v = 0, n = -1;
        int r = fwscanf(fp, L"%d%n", &v, &n);
        assert(r == 1);
        assert(v == 123);
        assert(n == 3);
        fclose(fp);
    }

    /* [10] 赋值抑制 * 时结果不写入对象 */
    {
        FILE *fp = make_wfile(L"456");
        assert(fp != NULL);
        int r = fwscanf(fp, L"%*d");
        assert(r == 0);                     /* 抑制，无赋值 */
        fclose(fp);
    }

    /* [11] 长度修饰符 hh / h / l / ll / j / z / t / L */
    {
        FILE *fp = make_wfile(L"1 2 3 4 5 6 7 8");
        assert(fp != NULL);
        signed char   hh = 0;
        short         h  = 0;
        long          l  = 0;
        long long     ll = 0;
        intmax_t      j  = 0;
        size_t        z  = 0;
        ptrdiff_t     t  = 0;
        long double   Ld = 0;
        int r = fwscanf(fp, L"%hhd %hd %ld %lld %jd %zu %td %Lf",
                        &hh, &h, &l, &ll, &j, &z, &t, &Ld);
        assert(r == 8);
        assert(hh == 1 && h == 2 && l == 3 && ll == 4);
        assert(j == 5 && z == 6 && t == 7);
        assert(Ld == 8.0L);
        fclose(fp);
    }

    /* [11] l 修饰符用于 c/s/[ 时，参数为 wchar_t* */
    {
        FILE *fp = make_wfile(L"hello");
        assert(fp != NULL);
        wchar_t wbuf[16] = {0};
        int r = fwscanf(fp, L"%ls", wbuf);
        assert(r == 1);
        assert(wcscmp(wbuf, L"hello") == 0);
        fclose(fp);
    }

    /* [12] 转换说明符 d / i / o / u / x / X */
    {
        FILE *fp = make_wfile(L"-10 0x1F 17 255 ff");
        assert(fp != NULL);
        int d = 0, i = 0, o = 0, u = 0, x = 0, X = 0;
        int r = fwscanf(fp, L"%d %i %o %u %x %X",
                        &d, &i, &o, &u, &x, &X);
        assert(r == 6);
        assert(d == -10);
        assert(i == 0x1F);
        assert(o == 017);
        assert(u == 255);
        assert(x == 0xff);
        assert(X == 0xff);
        fclose(fp);
    }

    /* [12] 转换说明符 f / e / g 等浮点 */
    {
        FILE *fp = make_wfile(L"3.14 2.5e2 1.0");
        assert(fp != NULL);
        float f = 0; double e = 0; double g = 0;
        int r = fwscanf(fp, L"%f %le %lg", &f, &e, &g);
        assert(r == 3);
        assert(f > 3.13f && f < 3.15f);
        assert(e == 250.0);
        assert(g == 1.0);
        fclose(fp);
    }

    /* [12] 转换说明符 c / s / [ */
    {
        FILE *fp = make_wfile(L"AB hello xyz");
        assert(fp != NULL);
        wchar_t c1 = 0, c2 = 0;
        wchar_t s[16] = {0};
        wchar_t set[16] = {0};
        int r = fwscanf(fp, L"%lc%lc %ls %l[xyz]", &c1, &c2, s, set);
        assert(r == 4);
        assert(c1 == L'A' && c2 == L'B');
        assert(wcscmp(s, L"hello") == 0);
        assert(wcscmp(set, L"xyz") == 0);
        fclose(fp);
    }

    /* [12] 转换说明符 p（指针） */
    {
        FILE *fp = make_wfile(L"0x1234");
        assert(fp != NULL);
        void *p = NULL;
        int r = fwscanf(fp, L"%p", &p);
        assert(r == 1);
        assert(p != NULL);
        fclose(fp);
    }

    /* [12] 转换说明符 n */
    {
        FILE *fp = make_wfile(L"abc");
        assert(fp != NULL);
        int n = -1;
        int r = fwscanf(fp, L"%n", &n);
        assert(r == 0);                     /* %n 不增加返回计数 */
        assert(n == 0);
        fclose(fp);
    }

    /* [12] %% 匹配字面 % */
    {
        FILE *fp = make_wfile(L"%");
        assert(fp != NULL);
        int r = fwscanf(fp, L"%%");
        assert(r == 0);                     /* %% 不赋值 */
        fclose(fp);
    }

    /* [4] 指令失败时函数返回：返回已成功赋值的项数 */
    {
        FILE *fp = make_wfile(L"1 x");
        assert(fp != NULL);
        int a = 0, b = 0;
        int r = fwscanf(fp, L"%d %d", &a, &b); /* 第二个失败 */
        assert(r == 1);
        assert(a == 1);
        fclose(fp);
    }

    /* [4] 输入失败（EOF）：返回 EOF */
    {
        FILE *fp = make_wfile(L"");
        assert(fp != NULL);
        int v = 0;
        int r = fwscanf(fp, L"%d", &v);     /* 立即 EOF -> 输入失败 */
        assert(r == EOF);
        fclose(fp);
    }

    /* [2] 格式耗尽而参数多余：多余参数被求值但忽略 */
    {
        FILE *fp = make_wfile(L"5");
        assert(fp != NULL);
        int a = 0, b = 0;
        int r = fwscanf(fp, L"%d", &a, &b); /* b 多余 */
        assert(r == 1);
        assert(a == 5);
        fclose(fp);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束 [1]：fwscanf 第一个参数必须是 FILE*，不能传 int。
     * gcc -std=c99 应报错：incompatible type for argument 1 */
    {
        int not_a_file = 0;
        int x;
        fwscanf(not_a_file, L"%d", &x);
    }

    /* 违反约束 [1]：第二个参数必须是 const wchar_t*，不能传 char*。
     * gcc -std=c99 应报错：incompatible type for argument 2 */
    {
        FILE *fp = tmpfile();
        int x;
        fwscanf(fp, "%d", &x);   /* 传窄字符串字面量 */
    }

    /* 违反约束 [1]：fwscanf 返回 int，不能赋值给结构体。
     * gcc -std=c99 应报错：incompatible types */
    {
        struct S { int a; } s;
        FILE *fp = tmpfile();
        s = fwscanf(fp, L"%d", &s.a);
    }

    /* 违反约束 [1]：fwscanf 需要至少 2 个参数（stream 和 format）。
     * gcc -std=c99 应报错：too few arguments to function 'fwscanf' */
    {
        fwscanf();
    }

    /* 违反约束 [1]：fwscanf 不能只传一个参数。
     * gcc -std=c99 应报错：too few arguments to function 'fwscanf' */
    {
        FILE *fp = tmpfile();
        fwscanf(fp);
    }

#endif

    return 0;
}