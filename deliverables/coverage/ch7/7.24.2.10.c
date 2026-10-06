/*
 * 测试条款：C99 7.24.2.10  The vwscanf function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 头文件 <stdarg.h> <wchar.h> 与原型
 *   [2] 等价于 wscanf，但可变实参列表由 va_list arg 取代；
 *       arg 必须由 va_start 初始化（可能经过若干 va_arg 调用）；
 *       vwscanf 不调用 va_end。
 *   [3] 返回值：输入失败且在任何转换之前发生 -> EOF；
 *       否则返回成功赋值的输入项数，可能少于提供项数，甚至为 0。
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 头文件与原型：包含 <stdarg.h> 与 <wchar.h> 后，
 *     vwscanf 应被声明为：
 *     int vwscanf(const wchar_t * restrict format, va_list arg);
 *     这里通过取函数指针来静态验证原型签名。 */
static int (*vwscanf_proto_check)(const wchar_t * restrict, va_list) = vwscanf;

/* 辅助函数：把可变实参转发给 vwscanf。
 * 这是 vwscanf 的典型用法：调用者用 va_start 初始化 arg，
 * 被调用者（此处为 wrapper）把 arg 传给 vwscanf。
 * [2] vwscanf 本身不调用 va_end，由 wrapper 负责调用 va_end。 */
static int call_vwscanf(const wchar_t * restrict fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);          /* [2] arg 必须由 va_start 初始化 */
    ret = vwscanf(fmt, ap);     /* [2] 用 va_list 取代可变实参列表 */
    va_end(ap);                 /* [2] vwscanf 不调用 va_end，由调用者调用 */

    return ret;
}

/* 辅助函数：演示 arg 在传给 vwscanf 之前经过若干 va_arg 调用。
 * 第一个可变实参是“前缀”整数，被 va_arg 取走；
 * 剩下的实参列表再交给 vwscanf。 */
static int call_vwscanf_after_va_arg(int prefix, const wchar_t * restrict fmt, ...)
{
    va_list ap;
    int got_prefix;
    int ret;

    va_start(ap, fmt);
    got_prefix = va_arg(ap, int);   /* [2] 可能经过若干 va_arg 调用 */
    assert(got_prefix == prefix);
    ret = vwscanf(fmt, ap);         /* 剩余列表交给 vwscanf */
    va_end(ap);

    return ret;
}

/* 用临时文件把给定内容作为 stdin 重定向，然后调用 fn。
 * 返回 fn 的返回值。 */
static int with_stdin(const char *content, int (*fn)(void))
{
    FILE *fp;
    int ret;

    fp = tmpfile();
    assert(fp != NULL);
    if (content != NULL) {
        assert(fputs(content, fp) >= 0);
    }
    rewind(fp);

    {
        FILE *saved = stdin;
        stdin = fp;
        ret = fn();
        stdin = saved;
    }

    fclose(fp);
    return ret;
}

/* ---- 正向用例 1：基本读取，验证返回值 = 成功赋值项数 [3] ---- */
static int read_two_ints(void)
{
    int a = 0, b = 0;
    int n;

    /* 输入 "12 34" -> 应成功赋值 2 项 */
    n = call_vwscanf(L"%d %d", &a, &b);
    assert(n == 2);
    assert(a == 12);
    assert(b == 34);

    return n;
}

/* ---- 正向用例 2：提前匹配失败，返回值可为 0 [3] ---- */
static int read_zero_items(void)
{
    int a = -1;
    int n;

    /* 输入 "abc" 对 "%d" 是匹配失败，且发生在任何转换之前 -> 返回 0 */
    n = call_vwscanf(L"%d", &a);
    assert(n == 0);
    assert(a == -1);   /* 未赋值，保持原值 */

    return n;
}

/* ---- 正向用例 3：部分匹配，返回值少于提供项数 [3] ---- */
static int read_partial(void)
{
    int a = -1, b = -1;
    int n;

    /* 输入 "7 xyz"：第一个 %d 成功，第二个 %d 匹配失败 -> 返回 1 */
    n = call_vwscanf(L"%d %d", &a, &b);
    assert(n == 1);
    assert(a == 7);
    assert(b == -1);

    return n;
}

/* ---- 正向用例 4：输入失败（空输入）在任何转换之前 -> EOF [3] ---- */
static int read_eof(void)
{
    int a = -1;
    int n;

    /* 空输入：在任何转换之前发生输入失败 -> 返回 EOF */
    n = call_vwscanf(L"%d", &a);
    assert(n == EOF);
    assert(a == -1);

    return n;
}

/* ---- 正向用例 5：arg 在传给 vwscanf 前经过 va_arg 调用 [2] ---- */
static int read_after_va_arg(void)
{
    int a = 0;
    int n;

    /* 前缀 99 被 va_arg 取走，剩余 "%d" 交给 vwscanf 读取 5 */
    n = call_vwscanf_after_va_arg(99, L"%d", &a);
    assert(n == 1);
    assert(a == 5);

    return n;
}

/* ---- 正向用例 6：宽字符字符串读取，验证 vwscanf 与 wscanf 等价 [2] ---- */
static int read_wide_string(void)
{
    wchar_t buf[32];
    int n;

    memset(buf, 0, sizeof(buf));
    n = call_vwscanf(L"%ls", buf);
    assert(n == 1);
    assert(wcscmp(buf, L"hello") == 0);

    return n;
}

/* ---- 正向用例 7：验证 vwscanf 不调用 va_end [2] ----
 * 在 wrapper 中，vwscanf 返回后我们仍然可以继续使用 ap
 * （这里仅演示 va_end 由 wrapper 调用；若 vwscanf 内部调用了
 *  va_end，则后续 va_end 行为未定义，但本测试只验证正常路径）。 */
static int read_then_va_end(void)
{
    va_list ap;
    int a = 0;
    int n;

    va_start(ap, a);   /* 占位，实际用不到 a */
    n = vwscanf(L"%d", ap);
    va_end(ap);        /* [2] 由调用者调用 va_end */

    return n;
}

int main(void)
{
    int r;

    /* [1] 原型签名检查：若原型不匹配，编译期即报错 */
    assert(vwscanf_proto_check == vwscanf);

    /* [3] 基本读取 */
    r = with_stdin("12 34", read_two_ints);
    assert(r == 2);

    /* [3] 提前匹配失败 -> 0 */
    r = with_stdin("abc", read_zero_items);
    assert(r == 0);

    /* [3] 部分匹配 -> 少于提供项数 */
    r = with_stdin("7 xyz", read_partial);
    assert(r == 1);

    /* [3] 输入失败 -> EOF */
    r = with_stdin("", read_eof);
    assert(r == EOF);

    /* [2] arg 经过 va_arg 调用后再传给 vwscanf */
    r = with_stdin("5", read_after_va_arg);
    assert(r == 1);

    /* [2] 宽字符字符串读取 */
    r = with_stdin("hello", read_wide_string);
    assert(r == 1);

    /* [2] vwscanf 不调用 va_end，由调用者调用 */
    r = with_stdin("42", read_then_va_end);
    assert(r == 1);

    printf("C99 7.24.2.10 vwscanf: all positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束 [1]：vwscanf 的第一个参数类型为 const wchar_t * restrict。
 * 传入 int * 类型不兼容，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 1 ...）。 */
void neg_wrong_first_arg_type(void)
{
    int *p = 0;
    va_list ap;
    vwscanf(p, ap);   /* 错误：第一个实参应为 const wchar_t * */
}

/* 违反约束 [1]：vwscanf 的第二个参数类型为 va_list。
 * 传入 int 类型不兼容，gcc -std=c99 应报错。 */
void neg_wrong_second_arg_type(void)
{
    vwscanf(L"%d", 42);   /* 错误：第二个实参应为 va_list */
}

/* 违反约束 [1]：vwscanf 需要两个实参。
 * 只传一个实参，gcc -std=c99 应报错（too few arguments）。 */
void neg_too_few_args(void)
{
    vwscanf(L"%d");   /* 错误：缺少 va_list 实参 */
}

/* 违反约束 [1]：vwscanf 需要两个实参。
 * 传三个实参，gcc -std=c99 应报错（too many arguments）。 */
void neg_too_many_args(void)
{
    va_list ap;
    vwscanf(L"%d", ap, 0);   /* 错误：实参过多 */
}

/* 违反约束 [1]：vwscanf 返回 int，不能当作结构体使用。
 * 对返回值做成员访问，gcc -std=c99 应报错。 */
void neg_return_not_struct(void)
{
    va_list ap;
    int x = vwscanf(L"%d", ap).field;   /* 错误：int 无成员 */
}

/* 违反约束 [1]：vwscanf 的返回值不是左值，不能赋值。
 * gcc -std=c99 应报错（lvalue required as left operand of assignment）。 */
void neg_return_not_lvalue(void)
{
    va_list ap;
    vwscanf(L"%d", ap) = 0;   /* 错误：函数返回值不是左值 */
}

/* 违反约束 [1]：vwscanf 的返回值不是左值，不能取地址。
 * gcc -std=c99 应报错（lvalue required as unary '&' operand）。 */
void neg_return_addr(void)
{
    va_list ap;
    int *p = &vwscanf(L"%d", ap);   /* 错误：不能对返回值取地址 */
    (void)p;
}

/* 违反约束 [1]：vwscanf 的返回值不是左值，不能自增。
 * gcc -std=c99 应报错（lvalue required as increment operand）。 */
void neg_return_increment(void)
{
    va_list ap;
    vwscanf(L"%d", ap)++;   /* 错误：返回值不是左值 */
}

#endif /* 负向测试结束 */