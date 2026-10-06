/*
 * 测试条款：C99 7.15.1.4  The va_start macro
 *
 * 预期行为：
 *   正向测试：以下使用 va_start 的代码应能编译并正确运行（assert 通过）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型 #include <stdarg.h>  void va_start(va_list ap, parmN);
 *   [2] va_start 必须在访问任何无名实参之前调用
 *   [3] va_start 初始化 ap 供 va_arg/va_end 使用；未 va_end 前不得重新初始化
 *   [4] parmN 是 ... 前最右参数；register/函数/数组类型或与默认实参提升后
 *       类型不兼容时行为未定义（UB，不作为负向测试）
 *   [5] va_start 不返回值
 *   [6] EXAMPLE 1：f1 收集指针列表传给 f2
 *   [7] EXAMPLE 2：f3 用 va_copy 保存列表状态
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [6] EXAMPLE 1 的复现：f1 收集 n_ptrs 个 char* 到数组，再传给 f2 */
#define MAXARGS 31

static char *g_f2_array[MAXARGS];
static int   g_f2_n = -1;

static void f2(int n, char *array[])
{
    int i;
    g_f2_n = n;
    for (i = 0; i < n; i++)
        g_f2_array[i] = array[i];
}

static void f1(int n_ptrs, ...)
{
    va_list ap;
    char *array[MAXARGS];
    int ptr_no = 0;

    if (n_ptrs > MAXARGS)
        n_ptrs = MAXARGS;

    /* [2] 在访问无名实参之前调用 va_start */
    /* [4] parmN = n_ptrs，是 ... 前最右参数，类型 int，与默认提升后兼容 */
    va_start(ap, n_ptrs);

    while (ptr_no < n_ptrs)
        array[ptr_no++] = va_arg(ap, char *);

    /* [3] 使用完毕后 va_end */
    va_end(ap);

    f2(n_ptrs, array);
}

/* [7] EXAMPLE 2 的复现：f3 用 va_copy 保存列表状态 */
static char *g_f4_array[MAXARGS];
static int   g_f4_n = -1;

static void f4(int n, char *array[])
{
    int i;
    g_f4_n = n;
    for (i = 0; i < n; i++)
        g_f4_array[i] = array[i];
}

static void f3(int n_ptrs, int f4_after, ...)
{
    va_list ap, ap_save;
    char *array[MAXARGS];
    int ptr_no = 0;

    if (n_ptrs > MAXARGS)
        n_ptrs = MAXARGS;

    va_start(ap, f4_after);

    while (ptr_no < n_ptrs) {
        array[ptr_no++] = va_arg(ap, char *);
        if (ptr_no == f4_after)
            va_copy(ap_save, ap);
    }
    va_end(ap);

    f2(n_ptrs, array);

    /* 处理保存的副本 */
    n_ptrs -= f4_after;
    ptr_no = 0;
    while (ptr_no < n_ptrs)
        array[ptr_no++] = va_arg(ap_save, char *);
    va_end(ap_save);

    f4(n_ptrs, array);
}

/* [5] va_start 不返回值：验证它可作为表达式语句使用（无返回值） */
static int test_no_return_value(int first, ...)
{
    va_list ap;
    int sum = first;
    int v;

    /* 作为语句调用，不接收返回值 */
    va_start(ap, first);
    while ((v = va_arg(ap, int)) != 0)
        sum += v;
    va_end(ap);
    return sum;
}

/* [2] 验证 va_start 在访问无名实参之前调用即可正常工作 */
static int test_before_access(int n, ...)
{
    va_list ap;
    int i, sum = 0;

    va_start(ap, n);          /* 先初始化 */
    for (i = 0; i < n; i++)   /* 再访问无名实参 */
        sum += va_arg(ap, int);
    va_end(ap);
    return sum;
}

/* [3] 验证 va_start 初始化后 ap 可被 va_arg/va_end 正常使用 */
static int test_init_for_va_arg(int a, ...)
{
    va_list ap;
    int r;
    va_start(ap, a);
    r = va_arg(ap, int);
    va_end(ap);
    return r;
}

/* [4] parmN 为 int（默认提升后仍为 int，兼容）——正常使用 */
static int test_parmN_int(int parmN, ...)
{
    va_list ap;
    int r;
    va_start(ap, parmN);
    r = va_arg(ap, int);
    va_end(ap);
    return r;
}

/* [4] parmN 为 double（默认提升后仍为 double，兼容）——正常使用 */
static double test_parmN_double(double parmN, ...)
{
    va_list ap;
    double r;
    va_start(ap, parmN);
    r = va_arg(ap, double);
    va_end(ap);
    return r;
}

/* [4] parmN 为指针类型（默认提升后仍为指针，兼容）——正常使用 */
static const char *test_parmN_ptr(const char *parmN, ...)
{
    va_list ap;
    const char *r;
    va_start(ap, parmN);
    r = va_arg(ap, const char *);
    va_end(ap);
    return r;
}

int main(void)
{
    /* ---- [6] EXAMPLE 1 ---- */
    f1(3, "alpha", "beta", "gamma");
    assert(g_f2_n == 3);
    assert(strcmp(g_f2_array[0], "alpha") == 0);
    assert(strcmp(g_f2_array[1], "beta")  == 0);
    assert(strcmp(g_f2_array[2], "gamma") == 0);

    /* n_ptrs 超过 MAXARGS 时被截断 */
    f1(0);
    assert(g_f2_n == 0);

    /* ---- [7] EXAMPLE 2 ---- */
    /* 传入 5 个指针，f4_after = 2：前 2 个给 f2，后 3 个给 f4 */
    f3(5, 2, "p0", "p1", "p2", "p3", "p4");
    assert(g_f2_n == 5);
    assert(strcmp(g_f2_array[0], "p0") == 0);
    assert(strcmp(g_f2_array[4], "p4") == 0);
    assert(g_f4_n == 3);
    assert(strcmp(g_f4_array[0], "p2") == 0);
    assert(strcmp(g_f4_array[1], "p3") == 0);
    assert(strcmp(g_f4_array[2], "p4") == 0);

    /* ---- [5] va_start 不返回值 ---- */
    assert(test_no_return_value(1, 2, 3, 4, 0) == 10);

    /* ---- [2] 先 va_start 再访问无名实参 ---- */
    assert(test_before_access(4, 10, 20, 30, 40) == 100);

    /* ---- [3] va_start 初始化 ap 供 va_arg/va_end 使用 ---- */
    assert(test_init_for_va_arg(7, 99) == 99);

    /* ---- [4] parmN 类型与默认提升后兼容 ---- */
    assert(test_parmN_int(1, 42) == 42);
    assert(test_parmN_double(1.5, 2.5) == 2.5);
    assert(strcmp(test_parmN_ptr("first", "second"), "second") == 0);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 违反约束 [1]：va_start 需要 <stdarg.h> 中声明的 va_list 类型。
 * 若未包含 <stdarg.h>，va_list 未声明，编译应报错。
 * 期望：error: unknown type name 'va_list'
 */
void bad_no_stdarg(void)
{
    va_list ap;          /* va_list 未声明 */
    va_start(ap, x);
    va_end(ap);
}

/*
 * 违反约束 [1]：va_start 是宏，需要两个实参 (ap, parmN)。
 * 只给一个实参应报错。
 * 期望：error: macro "va_start" requires 2 arguments, but only 1 given
 */
void bad_too_few_args(int parmN, ...)
{
    va_list ap;
    va_start(ap);        /* 缺少 parmN 实参 */
    va_end(ap);
}

/*
 * 违反约束 [1]：va_start 需要两个实参，给三个应报错。
 * 期望：error: macro "va_start" passed 3 arguments, but takes just 2
 */
void bad_too_many_args(int parmN, ...)
{
    va_list ap;
    va_start(ap, parmN, extra);   /* 多余实参 */
    va_end(ap);
}

/*
 * 违反约束 [1]：va_start 的第一个实参必须是 va_list 类型。
 * 传入 int 应报错（va_start 展开后对非 va_list 类型操作）。
 * 期望：编译报错（类型不匹配 / 宏展开错误）
 */
void bad_first_arg_not_va_list(int parmN, ...)
{
    int not_a_va_list;
    va_start(not_a_va_list, parmN);   /* 第一个实参类型错误 */
    va_end(not_a_va_list);
}

/*
 * 违反约束 [1]：va_start 的第二个实参 parmN 必须是标识符（参数名）。
 * 传入表达式（如 parmN + 1）应报错。
 * 期望：编译报错（宏展开后取地址/标识符失败）
 */
void bad_parmN_not_identifier(int parmN, ...)
{
    va_list ap;
    va_start(ap, parmN + 1);   /* parmN 不是标识符 */
    va_end(ap);
}

/*
 * 违反约束 [1]：va_start 的第二个实参 parmN 必须是 ... 前最右参数。
 * 传入不存在的标识符应报错。
 * 期望：error: 'nonexistent' undeclared
 */
void bad_parmN_undeclared(int parmN, ...)
{
    va_list ap;
    va_start(ap, nonexistent);   /* 未声明的标识符 */
    va_end(ap);
}

/*
 * 违反约束 [1]：va_start 的第二个实参 parmN 必须是 ... 前最右参数。
 * 传入非最右参数（如第一个参数）在语义上错误，但编译器通常不报错，
 * 属于 [4] 的 UB 范畴，故此处不作为负向测试。
 */

#endif /* 负向测试结束 */