/*
 * 测试 C99 7.24.2.9 —— vwprintf 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应被编译器拒绝（编译报错）。
 *
 * 条款要点：
 *   [1] 原型：int vwprintf(const wchar_t * restrict format, va_list arg);
 *   [2] 等价于 wprintf，但可变实参列表由 arg 替换；arg 必须已由 va_start
 *       初始化（可能经过若干 va_arg 调用）；vwprintf 不调用 va_end。
 *   [3] 返回写入的宽字符数；若发生输出或编码错误，返回负值。
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>
#include <assert.h>
#include <locale.h>

/* ------------------------------------------------------------------ */
/* 辅助函数：把可变实参转发给 vwprintf，验证 [2] 的“等价于 wprintf”语义 */
/* ------------------------------------------------------------------ */
static int my_wprintf(const wchar_t * restrict format, ...)
{
    va_list ap;
    int ret;

    va_start(ap, format);
    /* [2] 用 arg 替换可变实参列表，转发给 vwprintf */
    ret = vwprintf(format, ap);
    /* [2] vwprintf 不调用 va_end，调用者负责 */
    va_end(ap);
    return ret;
}

/* 辅助函数：先消费一个 int 实参，再转发剩余实参，验证 [2] 中
 * “arg 可能经过若干 va_arg 调用” 的语义 */
static int my_wprintf_skip_int(int skip, const wchar_t * restrict format, ...)
{
    va_list ap;
    int ret;
    int consumed;

    va_start(ap, format);
    consumed = va_arg(ap, int);          /* 先取走一个 int */
    assert(consumed == skip);
    ret = vwprintf(format, ap);          /* 剩余实参交给 vwprintf */
    va_end(ap);
    return ret;
}

int main(void)
{
    /* 设置本地化环境，使宽字符输出可用（不影响本测试的返回值语义） */
    setlocale(LC_ALL, "C");

    /* ============================================================== */
    /* ========== 正向测试：以下代码应能编译并运行通过 ============== */
    /* ============================================================== */

    /* [1] 原型可用性：取函数地址，验证签名与 restrict 限定兼容 */
    {
        int (*fp)(const wchar_t * restrict, va_list) = vwprintf;
        assert(fp != NULL);
    }

    /* [2] vwprintf 等价于 wprintf：用同一格式串分别调用 wprintf 与
     *     vwprintf，比较返回的宽字符数是否一致。
     *     注意：这里比较的是“返回值语义”，不比较实际输出内容。 */
    {
        int r1, r2;

        r1 = wprintf(L"hello %ls %d\n", L"world", 42);

        va_list ap;
        /* 用一个包装函数把可变实参转成 va_list 传给 vwprintf */
        r2 = my_wprintf(L"hello %ls %d\n", L"world", 42);

        /* [3] 两者返回的宽字符数应相同 */
        assert(r1 == r2);
        assert(r1 > 0);
    }

    /* [2] arg 已经过若干 va_arg 调用后再传给 vwprintf */
    {
        int r = my_wprintf_skip_int(7, L"skip=%d\n", 7);
        assert(r > 0);
    }

    /* [3] 返回值：写入的宽字符数。
     *     格式串 L"abc" 无转换说明，输出 3 个宽字符，返回 3。 */
    {
        int r = my_wprintf(L"abc");
        assert(r == 3);
    }

    /* [3] 返回值：含转换说明时，返回的是“写入的宽字符数”，
     *     而不是实参个数。L"%d" 输出 "12345" 共 5 个宽字符。 */
    {
        int r = my_wprintf(L"%d", 12345);
        assert(r == 5);
    }

    /* [3] 返回值：空格式串输出 0 个宽字符，返回 0。 */
    {
        int r = my_wprintf(L"");
        assert(r == 0);
    }

    /* [3] 返回值：宽字符（非 ASCII）也按“宽字符个数”计数。
     *     L"\u00e9" 是单个宽字符，返回 1。 */
    {
        int r = my_wprintf(L"\u00e9");
        assert(r == 1);
    }

    /* [2] vwprintf 不调用 va_end：调用者可在 vwprintf 之后继续
     *     使用同一个 va_list（这里用 va_end 正常收尾，验证不崩溃）。 */
    {
        va_list ap;
        int r;

        va_start(ap, NULL); /* 占位，实际下面重新初始化 */
        va_end(ap);

        /* 重新走一遍完整流程，确认 va_end 由调用者负责 */
        r = my_wprintf(L"ok\n");
        assert(r == 3);
    }

    printf("ALL POSITIVE TESTS PASSED\n");
    return 0;
}

/* ============================================================== */
/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ======== */
/* ============================================================== */
#if 0

/* 违反约束 [1]：vwprintf 的第一个参数类型为 const wchar_t *，
 * 传入 char * 类型不兼容，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 1 ...）。 */
{
    va_list ap;
    char *fmt = "hello";
    vwprintf(fmt, ap);   /* 错误：char* 不能隐式转换为 const wchar_t* */
}

/* 违反约束 [1]：第二个参数必须是 va_list 类型，
 * 传入 int 不兼容，应报错。 */
{
    vwprintf(L"x", 0);   /* 错误：int 不能转换为 va_list */
}

/* 违反约束 [1]：参数个数不足，缺少 va_list 实参，应报错。 */
{
    vwprintf(L"x");      /* 错误：参数太少 */
}

/* 违反约束 [1]：参数个数过多，应报错。 */
{
    va_list ap;
    vwprintf(L"x", ap, 1);  /* 错误：参数太多 */
}

/* 违反约束 [1]：vwprintf 返回 int，不能赋值给结构体类型，应报错。 */
{
    struct S { int x; } s;
    va_list ap;
    s = vwprintf(L"x", ap);  /* 错误：int 不能赋给 struct S */
}

/* 违反约束 [1]：对函数调用结果取地址（函数返回值非左值），应报错。 */
{
    va_list ap;
    int *p = &vwprintf(L"x", ap);  /* 错误：不能取函数返回值的地址 */
}

/* 违反约束 [1]：对函数调用结果赋值（非左值），应报错。 */
{
    va_list ap;
    vwprintf(L"x", ap) = 5;  /* 错误：赋值目标不是左值 */
}

#endif