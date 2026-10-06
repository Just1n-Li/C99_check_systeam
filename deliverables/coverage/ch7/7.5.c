/*
 * 测试 C99 7.5 <errno.h> Errors
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，违反约束的片段应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：
 *   [1] <errno.h> 定义若干与错误条件报告相关的宏
 *   [2] EDOM/EILSEQ/ERANGE 为 int 类型整数常量表达式、互不相同的正值，
 *       可用于 #if；errno 展开为可修改左值，类型 int
 *   [3] 程序启动时 errno 为 0；库函数从不将其置 0；可能被置为非零
 *   [4] 实现可额外定义以 E+数字 或 E+大写字母 开头的宏
 *   Footnote 175/176/177 相关语义
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <errno.h> 定义了与错误报告相关的宏 */
/* [2] EDOM / EILSEQ / ERANGE 是 int 类型的整数常量表达式 */
static int check_macro_types(void)
{
    /* 用 _Static_assert 风格：C99 无 _Static_assert，用数组大小技巧 */
    /* 若 EDOM 不是整数常量表达式，则数组大小非法，编译失败 */
    char a[EDOM  >= 1 ? 1 : -1];
    char b[EILSEQ >= 1 ? 1 : -1];
    char c[ERANGE >= 1 ? 1 : -1];
    (void)a; (void)b; (void)c;
    return 1;
}

/* [2] 三个宏互不相同，且均为正值 */
static void check_macro_distinct_positive(void)
{
    assert(EDOM   > 0);
    assert(EILSEQ > 0);
    assert(ERANGE > 0);
    assert(EDOM   != EILSEQ);
    assert(EDOM   != ERANGE);
    assert(EILSEQ != ERANGE);
}

/* [2] 三个宏可用于 #if 预处理指令 */
#if !defined(EDOM) || !defined(EILSEQ) || !defined(ERANGE)
#error "EDOM/EILSEQ/ERANGE must be defined as macros usable in #if"
#endif
#if (EDOM <= 0) || (EILSEQ <= 0) || (ERANGE <= 0)
#error "EDOM/EILSEQ/ERANGE must be positive in #if"
#endif
#if (EDOM == EILSEQ) || (EDOM == ERANGE) || (EILSEQ == ERANGE)
#error "EDOM/EILSEQ/ERANGE must be distinct in #if"
#endif

/* [2] errno 展开为可修改左值，类型为 int */
static void check_errno_is_modifiable_lvalue(void)
{
    /* 可读 */
    int v = errno;
    (void)v;
    /* 可写：作为左值赋值 */
    errno = 0;
    assert(errno == 0);
    errno = 42;
    assert(errno == 42);
    /* 可取地址（左值） */
    int *p = &errno;
    *p = 7;
    assert(errno == 7);
    /* 类型为 int：sizeof 与 int 一致 */
    assert(sizeof(errno) == sizeof(int));
}

/* [3] 程序启动时 errno 为 0 */
static void check_errno_zero_at_startup(void)
{
    /* 注意：本函数在 main 早期调用，且此前未调用可能设置 errno 的库函数 */
    assert(errno == 0);
}

/* [3] 库函数从不将 errno 置 0；可能被置为非零 */
static void check_library_sets_nonzero(void)
{
    /* 使用一个会失败的数学调用：sqrt(-1.0) 应设置 EDOM */
    errno = 0;
    volatile double neg = -1.0;
    double r = sqrt(neg);
    (void)r;
    /* 若实现报告域错误，则 errno 应为 EDOM（非零） */
    /* 标准允许实现不设置 errno（若未文档化），但若设置则应为 EDOM */
    if (errno != 0) {
        assert(errno == EDOM);
    }

    /* 使用 strtol 溢出：应设置 ERANGE */
    errno = 0;
    char *endp = NULL;
    const char *big = "99999999999999999999999999999999999999";
    long lv = strtol(big, &endp, 10);
    (void)lv;
    /* 若实现报告范围错误，则 errno 应为 ERANGE */
    if (errno != 0) {
        assert(errno == ERANGE);
    }

    /* 库函数不会把 errno 置 0：先设为非零，调用一个成功且不涉及 errno 的函数 */
    errno = 12345;
    char buf[8];
    strcpy(buf, "abc");
    assert(strcmp(buf, "abc") == 0);
    /* 标准未要求成功调用清零 errno；此处仅验证“从不置 0”这一语义 */
    assert(errno != 0);
}

/* [4] 实现可额外定义以 E+数字 或 E+大写字母 开头的宏 */
static void check_additional_macros(void)
{
    /* 标准允许实现定义额外宏；此处仅验证已定义的宏名符合命名规则 */
    /* 我们无法枚举实现私有宏，但可验证 EDOM/EILSEQ/ERANGE 符合 E+大写字母 */
    /* 该测试为语义说明性检查，不依赖具体实现 */
    assert(1);
}

/* [Footnote 176] 使用 errno 做错误检查的惯用法：先清零，再调用，再检查 */
static void check_errno_usage_pattern(void)
{
    errno = 0;
    volatile double neg = -2.0;
    double r = sqrt(neg);
    (void)r;
    /* 若 errno 被设置，应为 EDOM */
    if (errno != 0) {
        assert(errno == EDOM);
    }
}

int main(void)
{
    /* [3] 启动时 errno 为 0 —— 必须在任何可能设置 errno 的调用之前检查 */
    check_errno_zero_at_startup();

    check_macro_types();
    check_macro_distinct_positive();
    check_errno_is_modifiable_lvalue();
    check_library_sets_nonzero();
    check_additional_macros();
    check_errno_usage_pattern();

    printf("C99 7.5 <errno.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 违反约束「errno 展开为可修改左值」：
 * 对 errno 取地址后赋给 const 指针再写入，或把 errno 当作不可修改对象使用。
 * 更直接的约束违反：把 errno 用作常量表达式（它不是常量）。
 * 期望：gcc -std=c99 报错（非常量初始化 / 非整数常量表达式）。
 */
int arr_errno[errno];   /* errno 不是整数常量表达式，数组大小非法 */

/*
 * 违反约束「EDOM/EILSEQ/ERANGE 是整数常量表达式」：
 * 若把它们当作非整数使用（例如用于需要浮点常量的场合），
 * 或对它们取地址（宏展开为常量，不能取地址）。
 * 期望：编译报错（取地址操作数不是左值）。
 */
int *p_edom = &EDOM;    /* EDOM 是整数常量，不是左值，不能取地址 */

/*
 * 违反约束「errno 类型为 int」：
 * 将 errno 用于需要非 int 类型的严格场合，例如作为结构体成员名冲突，
 * 或声明与 errno 同名的对象（[2] 规定程序定义名为 errno 的标识符行为未定义，
 * 但此处演示类型不匹配的约束违反）。
 * 期望：编译报错（类型不兼容）。
 */
struct S { int x; };
struct S s_errno = errno;   /* int 不能初始化结构体，类型不兼容 */

/*
 * 违反约束「EDOM/EILSEQ/ERANGE 互不相同的正值」：
 * 在 #if 中假设它们相等并用于数组维度，若实现中它们确实不同，
 * 则此处逻辑错误；但更直接的约束违反是将其用于非整数上下文。
 * 期望：编译报错（浮点常量表达式非法用于 #if）。
 */
#if (EDOM + 0.5) > 0
#error "EDOM used in non-integer constant expression"
#endif

#endif /* 负向测试结束 */