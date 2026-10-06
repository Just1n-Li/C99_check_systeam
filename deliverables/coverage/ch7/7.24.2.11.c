/*
 * 测试条款：C99 7.24.2.11  The wprintf function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int wprintf(const wchar_t * restrict format, ...);
 *   [2] 语义：等价于 fwprintf(stdout, format, ...)
 *   [3] 返回值：成功返回已传输的宽字符数；出错返回负值。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与声明完全一致。
 *     若 wprintf 的声明与标准不符（例如缺少 restrict 或返回类型不同），
 *     此处赋值会触发不兼容指针类型的诊断。 */
static int (*wprintf_proto_check)(const wchar_t * restrict, ...) = wprintf;

int main(void)
{
    /* 设置本地化环境，使宽字符输出可用（不影响条款语义，仅保证可运行）。 */
    setlocale(LC_ALL, "C");

    /* [1] 原型：接受 const wchar_t * 格式串与可变参数。 */
    assert(wprintf_proto_check == wprintf);

    /* [2] 语义：wprintf(fmt, ...) 等价于 fwprintf(stdout, fmt, ...)。
     *     用同一格式串分别调用两者，比较返回值应一致。 */
    {
        int r1, r2;
        fflush(stdout);
        r1 = wprintf(L"[wprintf] plain text\n");
        fflush(stdout);
        r2 = fwprintf(stdout, L"[fwprintf] plain text\n");
        fflush(stdout);
        /* 两次输出内容长度相同（仅前缀不同，长度差为常量），
         * 这里只验证返回值均为非负，且与 fwprintf 行为一致（都成功）。 */
        assert(r1 >= 0);
        assert(r2 >= 0);
    }

    /* [2] 语义：带可变参数与转换说明，验证返回值等于实际传输的宽字符数。 */
    {
        int r;
        fflush(stdout);
        /* 字符串 "abc" 共 3 个宽字符，无换行。 */
        r = wprintf(L"%ls", L"abc");
        fflush(stdout);
        assert(r == 3);
    }

    /* [2] 语义：整数转换说明，验证返回值计数正确。 */
    {
        int r;
        fflush(stdout);
        /* "%d" 输出 "42"，共 2 个宽字符。 */
        r = wprintf(L"%d", 42);
        fflush(stdout);
        assert(r == 2);
    }

    /* [2] 语义：混合文本与转换说明，验证总宽字符数。 */
    {
        int r;
        fflush(stdout);
        /* "x=7" 共 3 个宽字符。 */
        r = wprintf(L"x=%d", 7);
        fflush(stdout);
        assert(r == 3);
    }

    /* [2] 语义：空格式串，传输 0 个宽字符，返回 0。 */
    {
        int r;
        fflush(stdout);
        r = wprintf(L"");
        fflush(stdout);
        assert(r == 0);
    }

    /* [3] 返回值：成功时返回非负值（已传输的宽字符数）。 */
    {
        int r;
        fflush(stdout);
        r = wprintf(L"ok\n");
        fflush(stdout);
        assert(r >= 0);
    }

    /* [3] 返回值：出错时返回负值。
     *     通过关闭 stdout 制造输出错误，验证返回负值。
     *     注意：此操作会破坏 stdout，故放在最后执行。 */
    {
        int r;
        fflush(stdout);
        fclose(stdout);
        r = wprintf(L"should fail\n");
        /* 输出错误时应返回负值。 */
        assert(r < 0);
    }

    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wprintf 的第一个参数类型必须为 const wchar_t *」：
 * 传入窄字符指针（char *），与原型不兼容。
 * 期望：gcc -std=c99 报 "passing argument 1 of 'wprintf' from incompatible
 *       pointer type" 或类似错误。 */
void bad_arg_type(void)
{
    char *s = "hello";
    wprintf(s);            /* 错误：应为 const wchar_t * */
}

/* 违反约束「wprintf 的第一个参数类型必须为 const wchar_t *」：
 * 传入整数，类型完全不匹配。
 * 期望：编译报错（参数类型不兼容）。 */
void bad_arg_int(void)
{
    wprintf(42);           /* 错误：应为 const wchar_t * */
}

/* 违反约束「wprintf 的第一个参数类型必须为 const wchar_t *」：
 * 传入宽字符（wchar_t），而非指向宽字符的指针。
 * 期望：编译报错（参数类型不兼容）。 */
void bad_arg_wchar(void)
{
    wchar_t c = L'A';
    wprintf(c);            /* 错误：应为 const wchar_t * */
}

/* 违反约束「wprintf 至少需要一个参数（格式串）」：
 * 无参数调用，与原型 int wprintf(const wchar_t *, ...) 不符。
 * 期望：编译报错（参数太少）。 */
void bad_no_arg(void)
{
    wprintf();             /* 错误：缺少格式串参数 */
}

/* 违反约束「wprintf 返回 int，不能作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值违反赋值运算符约束。
 * 期望：编译报错（lvalue required as left operand of assignment）。 */
void bad_assign_result(void)
{
    wprintf(L"x") = 5;     /* 错误：函数调用结果非左值 */
}

/* 违反约束「wprintf 的返回类型为 int，不能用于需要结构体等不兼容类型之处」：
 * 将返回值赋给不兼容的指针类型（int 到指针无隐式转换）。
 * 期望：编译报错（赋值类型不兼容）。 */
void bad_return_type(void)
{
    int *p = wprintf(L"x"); /* 错误：int 不能隐式转换为 int * */
    (void)p;
}

#endif /* 负向测试结束 */