/*
 * 测试条款：C99 7.15.1.3  The va_end macro
 *
 * 预期行为：
 *   正向测试：以下使用 va_start / va_copy / va_end 的代码应能编译并正确运行。
 *   负向测试：违反约束的片段（如把 va_end 当函数取地址、把 va_end 用于返回值、
 *             参数个数/类型错误等）应导致编译报错。
 *
 * 条款要点：
 *   [1] 原型：void va_end(va_list ap);  需要 #include <stdarg.h>
 *   [2] va_end 使被 va_start / va_copy 初始化的 va_list 正常结束使用；
 *       va_end 可能修改 ap 使其不再可用（除非重新用 va_start/va_copy 初始化）；
 *       若没有对应的 va_start/va_copy，或返回前未调用 va_end，行为未定义。
 *   [3] va_end 不返回值（void）。
 */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1][2][3] 基本用法：va_start 初始化，va_end 正常结束 */
static int sum_ints(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;

    va_start(ap, n);                 /* [2] 初始化 ap */
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);                      /* [2] 正常返回前调用 va_end；[3] 无返回值 */
    return total;
}

/* [2] va_copy 初始化另一个 va_list，各自用 va_end 结束 */
static int sum_twice(int n, ...)
{
    va_list ap1, ap2;
    int total = 0;
    int i;

    va_start(ap1, n);
    va_copy(ap2, ap1);               /* [2] ap2 由 va_copy 初始化 */

    for (i = 0; i < n; i++) {
        total += va_arg(ap1, int);
    }
    for (i = 0; i < n; i++) {
        total += va_arg(ap2, int);   /* ap2 独立遍历 */
    }

    va_end(ap1);                     /* [2] 结束 ap1 */
    va_end(ap2);                     /* [2] 结束 ap2 */
    return total;
}

/* [2] va_end 之后可重新用 va_start 初始化同一个 va_list 再次使用 */
static int reuse_va_list(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;

    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);                      /* [2] 第一次结束 */

    /* [2] 重新初始化后 ap 再次可用 */
    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);                      /* [2] 第二次结束 */
    return total;
}

/* [2] 无原型函数调用中的默认实参提升：char -> int, float -> double */
static int sum_promoted(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;

    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);    /* char 提升为 int */
    }
    va_end(ap);
    return total;
}

static double sum_doubles(int n, ...)
{
    va_list ap;
    double total = 0.0;
    int i;

    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, double); /* float 提升为 double */
    }
    va_end(ap);
    return total;
}

/* [2] 省略号 (...) 之后的实参不再做默认提升之外的转换 */
static long long sum_ll(int n, ...)
{
    va_list ap;
    long long total = 0;
    int i;

    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, long long);
    }
    va_end(ap);
    return total;
}

/* [2] 嵌套：内层函数用 va_end，外层继续使用自己的 va_list */
static int outer_with_inner(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;

    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);
    }
    /* 调用另一个使用可变参数的函数，其内部自行 va_end */
    total += sum_ints(2, 100, 200);
    va_end(ap);
    return total;
}

int main(void)
{
    /* [1][2][3] 基本用法 */
    assert(sum_ints(3, 1, 2, 3) == 6);
    assert(sum_ints(0) == 0);

    /* [2] va_copy 与 va_end */
    assert(sum_twice(3, 1, 2, 3) == 12);   /* (1+2+3)*2 */

    /* [2] va_end 后重新 va_start */
    assert(reuse_va_list(3, 1, 2, 3) == 12);

    /* [2] 默认实参提升：char -> int */
    {
        char c1 = 10, c2 = 20;
        assert(sum_promoted(2, c1, c2) == 30);
    }

    /* [2] 默认实参提升：float -> double */
    {
        float f1 = 1.5f, f2 = 2.5f;
        assert(sum_doubles(2, f1, f2) == 4.0);
    }

    /* [2] 省略号后 long long 不做额外转换 */
    assert(sum_ll(2, 1LL, 2LL) == 3LL);

    /* [2] 嵌套调用 */
    assert(outer_with_inner(2, 1, 2) == 303);

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束 [1]：va_end 是宏，不是函数，不能取地址。
 * 期望报错：gcc -std=c99 报 "expected ..." 或 "lvalue required" 类错误。 */
#include <stdarg.h>
void (*fp)(va_list) = va_end;

/* 违反约束 [1]：va_end 返回 void，不能用于需要值的上下文。
 * 期望报错：void 值不能用于赋值/算术。 */
void bad_use_value(va_list ap)
{
    int x = va_end(ap);   /* 期望报错：void 值不能赋给 int */
    (void)x;
}

/* 违反约束 [1]：参数类型不匹配，va_end 需要 va_list。
 * 期望报错：类型不兼容。 */
void bad_arg_type(void)
{
    int n = 0;
    va_end(n);            /* 期望报错：int 不能传给 va_list 参数 */
}

/* 违反约束 [1]：参数个数错误，va_end 需要恰好一个参数。
 * 期望报错：宏参数个数不匹配。 */
void bad_arg_count(va_list ap)
{
    va_end();             /* 期望报错：参数太少 */
    va_end(ap, ap);       /* 期望报错：参数太多 */
}

/* 违反约束 [1]：未包含 <stdarg.h> 时使用 va_end。
 * 期望报错：va_end 未声明。 */
void bad_no_include(void)
{
    /* 假设此处没有 #include <stdarg.h> */
    va_list ap;
    va_end(ap);           /* 期望报错：va_list/va_end 未声明 */
}

#endif /* 负向测试结束 */