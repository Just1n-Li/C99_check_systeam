/*
 * 测试 C99 7.17 <stddef.h>：Common definitions
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 ... #endif 中，故意违反 7.17 的约束，
 *             期望编译器（gcc -std=c99 -pedantic-errors）报错。
 *
 * 覆盖段落：
 *   [1] 头文件定义的类型与宏
 *   [2] ptrdiff_t / size_t / wchar_t 的语义
 *   [3] NULL / offsetof 的语义与约束
 *   [4] Recommended practice（size_t/ptrdiff_t 的转换等级）
 *   Forward references: 7.11 localization
 */

#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>   /* 用于 wchar_t 相关测试（7.11 前向引用） */

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <stddef.h> 定义了 ptrdiff_t, size_t, wchar_t, NULL, offsetof */
/* 通过使用这些名字来验证它们确实被定义 */

/* [2] ptrdiff_t 是「两个指针相减结果」的有符号整数类型 */
static void test_ptrdiff_t(void)
{
    int arr[10];
    int *p = &arr[0];
    int *q = &arr[7];
    ptrdiff_t d = q - p;          /* 指针相减，结果类型为 ptrdiff_t */
    assert(d == 7);
    assert(d > 0);                /* 有符号类型，可表示负值 */
    d = p - q;
    assert(d == -7);              /* 有符号：负值可表示 */
    /* ptrdiff_t 是整数类型 */
    assert(sizeof(ptrdiff_t) >= 1);
}

/* [2] size_t 是「sizeof 运算符结果」的无符号整数类型 */
static void test_size_t(void)
{
    size_t s = sizeof(int);
    assert(s == sizeof(int));
    assert(s >= 1);
    /* 无符号：不可能为负 */
    assert((size_t)-1 > 0);       /* 无符号回绕，恒为真 */
    /* size_t 能表示任何对象的大小 */
    size_t a = sizeof(char);
    size_t b = sizeof(long double);
    assert(a <= b);
}

/* [2] wchar_t 是整数类型，其值域能表示所支持 locale 中最大扩展字符集的
 *     所有成员的不同编码；空字符编码值为 0。
 * [2] 若实现未定义 __STDC_MB_MIGHT_NEQ_WC__，则基本字符集每个成员的编码值
 *     等于它作为整数字符常量单独使用时的值。
 */
static void test_wchar_t(void)
{
    /* wchar_t 是整数类型 */
    wchar_t wc = L'A';
    assert(wc == (wchar_t)'A');   /* 基本字符集成员编码值 == 整数字符常量值 */

    /* 空字符编码值为 0 */
    wchar_t nul = L'\0';
    assert(nul == 0);

    /* 基本字符集成员：编码值等于其整数字符常量值 */
    assert(L'a' == (wchar_t)'a');
    assert(L'0' == (wchar_t)'0');
    assert(L' ' == (wchar_t)' ');
    assert(L'\n' == (wchar_t)'\n');

    /* wchar_t 是整数类型：可做算术 */
    wchar_t x = L'A' + 1;
    assert(x == (wchar_t)'B');

    /* 能表示扩展字符（若 locale 支持） */
    (void)wc;
}

/* [3] NULL 展开为实现定义的「空指针常量」 */
static void test_NULL(void)
{
    /* NULL 是空指针常量：可赋给任意指针类型 */
    int *ip = NULL;
    char *cp = NULL;
    void *vp = NULL;
    double *dp = NULL;

    assert(ip == NULL);
    assert(cp == NULL);
    assert(vp == NULL);
    assert(dp == NULL);

    /* 空指针与任何对象/函数指针比较均不相等 */
    int obj = 0;
    assert(&obj != NULL);

    /* NULL 可用作条件判断（空指针常量值为假） */
    if (NULL) {
        assert(0);   /* 不应到达 */
    }
}

/* [3] offsetof(type, member-designator) 展开为 size_t 类型的整数常量表达式，
 *     其值为成员相对结构体起始的字节偏移。
 * [3] 约束：type 与 member-designator 必须使得 &(t.member) 为地址常量。
 */
struct S {
    char  c;
    int   i;
    double d;
    char  tail[8];
};

static void test_offsetof(void)
{
    /* offsetof 结果类型为 size_t */
    size_t off_c = offsetof(struct S, c);
    size_t off_i = offsetof(struct S, i);
    size_t off_d = offsetof(struct S, d);
    size_t off_t = offsetof(struct S, tail);

    /* 第一个成员偏移为 0 */
    assert(off_c == 0);

    /* 偏移单调不减（成员按声明顺序） */
    assert(off_c <= off_i);
    assert(off_i <= off_d);
    assert(off_d <= off_t);

    /* 偏移与真实地址计算一致 */
    struct S s;
    assert(off_c == (size_t)((char *)&s.c - (char *)&s));
    assert(off_i == (size_t)((char *)&s.i - (char *)&s));
    assert(off_d == (size_t)((char *)&s.d - (char *)&s));
    assert(off_t == (size_t)((char *)&s.tail - (char *)&s));

    /* offsetof 是整数常量表达式：可用于数组维度、case 标签等 */
    char buf[offsetof(struct S, tail) + 1];
    assert(sizeof(buf) == offsetof(struct S, tail) + 1);

    /* 可用于静态初始化（常量表达式） */
    static const size_t koff = offsetof(struct S, d);
    assert(koff == off_d);

    /* 嵌套结构体成员 */
    struct Outer { int pad; struct S inner; };
    size_t o = offsetof(struct Outer, inner.i);
    assert(o == offsetof(struct Outer, inner) + offsetof(struct S, i));
}

/* [4] Recommended practice：size_t 与 ptrdiff_t 的整数转换等级
 *     不应大于 signed long int，除非实现需要支持足够大的对象。
 *     这里只做「可观察」的检查：在常见实现上它们不大于 long。
 *     注意：这是推荐做法，不是约束，因此不能作为编译错误测试。
 */
static void test_rank_recommendation(void)
{
    /* 在典型实现上，size_t/ptrdiff_t 的宽度不超过 long 的宽度。
     * 这里仅做信息性检查，不强制断言（推荐做法允许例外）。 */
    printf("sizeof(size_t)=%zu sizeof(ptrdiff_t)=%zu sizeof(long)=%zu\n",
           sizeof(size_t), sizeof(ptrdiff_t), sizeof(long));
    printf("sizeof(wchar_t)=%zu sizeof(int)=%zu\n",
           sizeof(wchar_t), sizeof(int));
    /* 至少保证 size_t 能表示对象大小 */
    assert(sizeof(size_t) >= sizeof(int));
}

/* [1] 这些名字也可在其他头文件中定义（如 <stdio.h> 提供 size_t） */
static void test_other_headers(void)
{
    /* <stdio.h> 也定义 size_t（通过 <stddef.h> 或自身） */
    size_t n = sizeof(int);
    assert(n == sizeof(int));
    /* <string.h> 使用 size_t */
    char buf[16];
    memset(buf, 0, sizeof buf);
    assert(strlen(buf) == 0);
}

int main(void)
{
    test_ptrdiff_t();
    test_size_t();
    test_wchar_t();
    test_NULL();
    test_offsetof();
    test_rank_recommendation();
    test_other_headers();

    printf("C99 7.17 <stddef.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [3]：offsetof 的 member-designator 必须是结构体成员。
 * 下面用非成员名，gcc -std=c99 -pedantic-errors 应报错
 * （error: 'struct S' has no member named 'nonexistent'）。 */
#include <stddef.h>
struct S { int x; };
size_t bad1 = offsetof(struct S, nonexistent);

/* 违反约束 [3]：offsetof 的第一个参数必须是类型（结构体/联合体类型）。
 * 下面传入一个对象而非类型，应报错。 */
struct S s_obj;
size_t bad2 = offsetof(s_obj, x);

/* 违反约束 [3]：offsetof 的第一个参数不能是基本类型（无成员）。
 * 应报错（error: request for member 'x' in something not a structure or union）。 */
size_t bad3 = offsetof(int, x);

/* 违反约束 [3]：offsetof 的第一个参数不能是未定义类型。
 * 应报错（error: invalid application of 'sizeof' to incomplete type）。 */
struct Undefined;
size_t bad4 = offsetof(struct Undefined, x);

/* 违反约束 [3]：offsetof 的 member-designator 不能是位域成员
 * （标准规定：若指定成员是位域，行为未定义——注意这是 UB 而非约束，
 *  因此严格来说不应作为负向测试。此处仅作注释说明，不放入负向块。）
 * 说明：位域情形是「未定义行为」，不是「约束违反」，故不在此测试。
 */

/* 违反约束 [2]：size_t 是无符号整数类型，不能用于需要「有符号」的
 * 严格场景——但 C 不禁止把无符号赋给有符号，故这不是约束违反。
 * 因此不在此测试。
 */

/* 违反约束 [3]：NULL 是空指针常量，不能用作整数常量表达式中的整数值
 * 来初始化整型对象（在严格 C99 中，NULL 可能是 (void*)0，赋给 int 会报错）。
 * 下面把 NULL 赋给 int，gcc -std=c99 -pedantic-errors 应报错
 * （error: initialization makes integer from pointer without a cast）。 */
int bad5 = NULL;

/* 违反约束 [3]：offsetof 结果类型为 size_t，不能直接用于需要
 * 指针类型的上下文（如解引用）。下面把 offsetof 结果当指针解引用，
 * 应报错（error: invalid type argument of unary '*'）。 */
int bad6 = *offsetof(struct S, x);

#endif /* 负向测试结束 */