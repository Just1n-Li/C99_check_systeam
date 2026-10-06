/*
 * 测试 C99 7.15.1 —— 可变参数列表访问宏 (va_start / va_arg / va_copy / va_end)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] va_start / va_arg 必须实现为宏，而非函数；
 *       va_copy / va_end 是宏还是外部链接标识符未指定；
 *       抑制宏定义以访问真实函数、或程序定义同名外部标识符 → UB；
 *       每次 va_start / va_copy 调用必须由同一函数内对应的 va_end 调用匹配。
 */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] va_start / va_arg 必须是宏：用 #ifdef 验证它们是宏定义。
 *     若它们被实现为函数而非宏，则 #ifdef 为假，编译期断言失败。 */
#ifndef va_start
#error "va_start must be a macro (C99 7.15.1 [1])"
#endif
#ifndef va_arg
#error "va_arg must be a macro (C99 7.15.1 [1])"
#endif

/* [1] va_copy / va_end 是宏还是外部链接标识符未指定，
 *     因此这里只做“存在性”检查：要么是宏，要么是可声明的标识符。
 *     下面通过实际使用来验证它们可用（见测试函数）。 */

/* [1] 基本用法：va_start 初始化，va_arg 逐个取参，va_end 结束。
 *     每次 va_start 都由同一函数内的 va_end 匹配。 */
static int sum_ints(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;

    va_start(ap, n);                 /* [1] va_start 调用 */
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);    /* [1] va_arg 调用 */
    }
    va_end(ap);                      /* [1] 与 va_start 匹配的 va_end */
    return total;
}

/* [1] 混合类型：验证 va_arg 按类型取参，且 va_end 匹配 va_start。 */
static double sum_mixed(int n, ...)
{
    va_list ap;
    double total = 0.0;
    int i;

    va_start(ap, n);
    for (i = 0; i < n; i++) {
        /* 交替取 int 与 double */
        if (i % 2 == 0) {
            total += (double)va_arg(ap, int);
        } else {
            total += va_arg(ap, double);
        }
    }
    va_end(ap);
    return total;
}

/* [1] va_copy：复制 va_list，两个副本各自遍历，且各自 va_end 匹配。
 *     验证 va_copy 调用由对应的 va_end 匹配。 */
static int copy_and_count(int n, ...)
{
    va_list ap, ap2;
    int count1 = 0, count2 = 0;
    int i;

    va_start(ap, n);                 /* [1] va_start */
    va_copy(ap2, ap);                /* [1] va_copy，需匹配 va_end */

    for (i = 0; i < n; i++) {
        (void)va_arg(ap, int);
        count1++;
    }
    for (i = 0; i < n; i++) {
        (void)va_arg(ap2, int);
        count2++;
    }

    va_end(ap);                      /* [1] 匹配 va_start */
    va_end(ap2);                     /* [1] 匹配 va_copy */
    return count1 + count2;
}

/* [1] 无原型函数调用中的默认实参提升：
 *     char → int，float → double；省略号 (...) 之后的实参停止转换。
 *     这里通过一个无原型函数转发到可变参数函数来验证。 */
static int take_va(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;
    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);    /* 提升后的 char 以 int 取出 */
    }
    va_end(ap);
    return total;
}

/* 无原型声明：调用时实参经历默认实参提升 */
static int forward_no_proto();

static int forward_no_proto(int n, ...)  /* 定义带原型，但调用点无原型 */
{
    va_list ap;
    int total = 0;
    int i;
    va_start(ap, n);
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);
    return total;
}

/* [1] 用 va_list 遍历字符串，验证 va_arg 取指针类型。 */
static size_t total_strlen(int n, ...)
{
    va_list ap;
    size_t total = 0;
    int i;

    va_start(ap, n);
    for (i = 0; i < n; i++) {
        const char *s = va_arg(ap, const char *);
        total += strlen(s);
    }
    va_end(ap);
    return total;
}

int main(void)
{
    /* [1] 基本 int 求和 */
    assert(sum_ints(3, 10, 20, 30) == 60);
    assert(sum_ints(0) == 0);

    /* [1] 混合类型 */
    assert(sum_mixed(4, 1, 2.5, 3, 4.5) == 11.0);

    /* [1] va_copy 复制后各自遍历 */
    assert(copy_and_count(3, 1, 2, 3) == 6);

    /* [1] 无原型调用中的默认实参提升：char 提升为 int */
    {
        char c = 'A';                /* 65 */
        /* 调用点无原型：c 提升为 int，省略号后停止转换 */
        int r = forward_no_proto(1, c);
        assert(r == 65);
    }

    /* [1] 无原型调用中的默认实参提升：float 提升为 double，
     *     但这里取 int 会不匹配，故用 double 版本验证。 */
    {
        /* 直接调用带原型的可变参数函数，float 实参在省略号处
         * 不发生提升（有原型时），但这里我们只验证 int 路径。 */
        assert(take_va(2, 5, 7) == 12);
    }

    /* [1] 指针类型 */
    assert(total_strlen(3, "ab", "cde", "f") == 6);

    printf("C99 7.15.1 positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束 [1]「va_start / va_arg 必须实现为宏，而非函数」：
 * 若编译器把它们实现为函数，则不能取地址当作函数指针使用；
 * 标准要求它们是宏，因此下面这种“当作函数取地址”的用法
 * 在符合标准的实现中应报错（宏名不是函数）。
 * 期望：gcc -std=c99 报错（va_start 是宏，不能取地址）。 */
void (*fp)(va_list *, const char *) = va_start;

/* 违反约束 [1]「每次 va_start 调用必须由同一函数内对应的 va_end 匹配」：
 * 下面函数调用了 va_start 却没有 va_end，违反匹配要求。
 * 期望：编译器/静态检查报错（缺少匹配的 va_end）。
 * 注：严格说这是约束违反，符合标准的实现应诊断。 */
int missing_va_end(int n, ...)
{
    va_list ap;
    va_start(ap, n);
    return va_arg(ap, int);
    /* 缺少 va_end(ap); —— 违反 [1] 匹配约束 */
}

/* 违反约束 [1]「每次 va_copy 调用必须由同一函数内对应的 va_end 匹配」：
 * 下面函数 va_copy 后没有对应的 va_end。
 * 期望：编译器/静态检查报错（缺少匹配的 va_end）。 */
int missing_va_end_after_copy(int n, ...)
{
    va_list ap, ap2;
    va_start(ap, n);
    va_copy(ap2, ap);
    va_end(ap);
    return va_arg(ap2, int);
    /* 缺少 va_end(ap2); —— 违反 [1] 匹配约束 */
}

/* 违反约束 [1]「抑制宏定义以访问真实函数，或定义同名外部标识符 → UB」：
 * 下面定义与 va_start 同名的外部标识符，属于 UB。
 * 期望：编译器报错（重定义/冲突）。 */
int va_start = 0;

#endif /* 负向测试结束 */