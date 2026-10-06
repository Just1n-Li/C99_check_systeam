/*
 * 测试条款：C99 7.24.2.8  The vswscanf function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int vswscanf(const wchar_t * restrict s,
 *                         const wchar_t * restrict format, va_list arg);
 *   [2] 语义：等价于 swscanf，但可变实参列表由 arg 取代；
 *             arg 应由 va_start 初始化（可能经过若干 va_arg 调用）；
 *             vswscanf 不调用 va_end。
 *   [3] 返回值：任何转换前发生输入失败返回 EOF；
 *             否则返回被赋值的输入项个数，可能少于提供数，甚至为 0。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <assert.h>
#include <locale.h>

/* ============================================================
 * 辅助函数：把 vswscanf 包装起来，以便用可变实参调用它。
 * 这正是 [2] 描述的典型用法：va_start 初始化 arg，
 * 传给 vswscanf，然后由调用者（此处）负责 va_end。
 * ============================================================ */
static int call_vswscanf(const wchar_t *s, const wchar_t *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);          /* [2] arg 由 va_start 初始化 */
    ret = vswscanf(s, fmt, ap); /* [1][2] 调用 vswscanf */
    va_end(ap);                 /* [2] va_end 由调用者执行，vswscanf 不做 */

    return ret;
}

/* 演示 [2] 中“可能经过若干 va_arg 调用”的情形：
 * 先取走一个 int，再把剩余的 arg 交给 vswscanf。 */
static int call_vswscanf_after_va_arg(const wchar_t *s, const wchar_t *fmt,
                                      int first, ...)
{
    va_list ap;
    int got_first;
    int ret;

    va_start(ap, first);
    got_first = va_arg(ap, int);   /* [2] 先做一次 va_arg */
    assert(got_first == first);
    ret = vswscanf(s, fmt, ap);    /* [2] 剩余 arg 交给 vswscanf */
    va_end(ap);

    return ret;
}

int main(void)
{
    /* 使用宽字符环境，保证宽字符 I/O 正常工作 */
    setlocale(LC_ALL, "");

    /* ==========================================================
     * 正向测试：以下代码应能编译并运行通过
     * ========================================================== */

    /* ---------- [1] 原型可用性：函数可被取地址、可被调用 ---------- */
    {
        int (*fp)(const wchar_t * restrict, const wchar_t * restrict, va_list);
        fp = vswscanf;              /* [1] 原型匹配，可赋值 */
        assert(fp == vswscanf);
    }

    /* ---------- [3] 正常转换：返回被赋值的输入项个数 ---------- */
    {
        const wchar_t *input = L"42 3.5 hello";
        int i = 0;
        double d = 0.0;
        wchar_t word[32];
        int n;

        memset(word, 0, sizeof word);
        n = call_vswscanf(input, L"%d %lf %ls", &i, &d, word);

        assert(n == 3);             /* [3] 三项均被赋值 */
        assert(i == 42);
        assert(d == 3.5);
        assert(wcscmp(word, L"hello") == 0);
    }

    /* ---------- [3] 提前匹配失败：返回已赋值项数（可为 0） ---------- */
    {
        const wchar_t *input = L"abc 99";
        int i = -1;
        int n;

        n = call_vswscanf(input, L"%d", &i);   /* 首字符非数字，匹配失败 */
        assert(n == 0);             /* [3] 提前匹配失败，返回 0 */
        assert(i == -1);            /* 未赋值，保持原值 */
    }

    /* ---------- [3] 部分匹配：返回少于提供数的项数 ---------- */
    {
        const wchar_t *input = L"7 xyz";
        int i = 0;
        int j = -1;
        int n;

        n = call_vswscanf(input, L"%d %d", &i, &j);
        assert(n == 1);             /* [3] 只赋值了第一项 */
        assert(i == 7);
        assert(j == -1);
    }

    /* ---------- [3] 输入失败：返回 EOF ---------- */
    {
        const wchar_t *input = L"";   /* 空输入，任何转换前即失败 */
        int i = 0;
        int n;

        n = call_vswscanf(input, L"%d", &i);
        assert(n == EOF);           /* [3] 输入失败返回 EOF */
    }

    /* ---------- [2] arg 可先经 va_arg 再传给 vswscanf ---------- */
    {
        const wchar_t *input = L"123 456";
        int a = 0, b = 0;
        int n;

        n = call_vswscanf_after_va_arg(input, L"%d", 111, &a);
        assert(n == 1);
        assert(a == 123);
        (void)b;
    }

    /* ---------- [2] 与 swscanf 等价性：相同输入/格式得到相同结果 ---------- */
    {
        const wchar_t *input = L"10 20";
        int a1 = 0, b1 = 0, a2 = 0, b2 = 0;
        int n1, n2;

        n1 = call_vswscanf(input, L"%d %d", &a1, &b1);
        n2 = swscanf(input, L"%d %d", &a2, &b2);

        assert(n1 == n2);           /* [2] 等价于 swscanf */
        assert(a1 == a2);
        assert(b1 == b2);
    }

    /* ---------- [2] vswscanf 不调用 va_end：调用者仍可继续使用 ap ---------- */
    {
        const wchar_t *input = L"5";
        int i = 0;
        int n;

        n = call_vswscanf(input, L"%d", &i);
        assert(n == 1);
        assert(i == 5);
        /* 若 vswscanf 内部错误地调用了 va_end，上面的包装函数
         * 再调用 va_end 将导致未定义行为；此处正常返回即说明
         * 责任划分符合 [2]。 */
    }

    printf("vswscanf: all positive tests passed.\n");

    /* ==========================================================
     * 负向测试：以下代码违反 C99 约束，应编译报错
     * ========================================================== */
#if 0

    /* 违反约束 [1]：vswscanf 的第三个参数类型为 va_list，
     * 传入 int* 不匹配原型，gcc -std=c99 应报错
     * （incompatible type for argument 3 / too few arguments 等）。 */
    {
        const wchar_t *s = L"1";
        const wchar_t *f = L"%d";
        int x = 0;
        vswscanf(s, f, &x);         /* 第三个实参应为 va_list，而非 int* */
    }

    /* 违反约束 [1]：参数个数不足，缺少 va_list 实参，
     * gcc -std=c99 应报错（too few arguments to function 'vswscanf'）。 */
    {
        const wchar_t *s = L"1";
        const wchar_t *f = L"%d";
        vswscanf(s, f);
    }

    /* 违反约束 [1]：第一个参数应为 const wchar_t*，
     * 传入窄字符指针 char* 不匹配，gcc -std=c99 应报错
     * （incompatible type for argument 1）。 */
    {
        const char *s = "1";
        const wchar_t *f = L"%d";
        va_list ap;
        vswscanf(s, f, ap);
    }

    /* 违反约束 [1]：第二个参数应为 const wchar_t*，
     * 传入窄字符格式串 char* 不匹配，gcc -std=c99 应报错。 */
    {
        const wchar_t *s = L"1";
        const char *f = "%d";
        va_list ap;
        vswscanf(s, f, ap);
    }

    /* 违反约束 [1]：vswscanf 返回 int，不能作为结构体使用，
     * 对返回值做成员访问应报错。 */
    {
        const wchar_t *s = L"1";
        const wchar_t *f = L"%d";
        va_list ap;
        int y = vswscanf(s, f, ap).x;   /* int 无成员 x，应报错 */
        (void)y;
    }

    /* 违反约束 [1]：vswscanf 的返回值是非左值，
     * 对其赋值应报错（lvalue required as left operand of assignment）。 */
    {
        const wchar_t *s = L"1";
        const wchar_t *f = L"%d";
        va_list ap;
        vswscanf(s, f, ap) = 0;         /* 非左值赋值，应报错 */
    }

#endif /* 负向测试结束 */

    return 0;
}