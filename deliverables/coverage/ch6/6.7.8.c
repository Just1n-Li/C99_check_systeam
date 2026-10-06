/*
 * 测试 C99 6.7.8 Initialization
 *
 * 预期行为：
 *   正向测试：以下代码应能编译（gcc -std=c99 -Wall）并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，每个片段违反 6.7.8 的某条约束，编译器应报错。
 *             这些片段被 #if 0 屏蔽，保证整个文件仍可编译运行。
 *
 * 覆盖段落：[1] 语法、[2]-[7] Constraints、[8]-[21] Semantics、EXAMPLE。
 */

#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <wchar.h>
#include <string.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [10] 静态存储期未显式初始化：指针->空指针；算术->0；聚合递归；联合首成员 */
static int    g_int;              /* 算术 -> 0 */
static int   *g_ptr;              /* 指针 -> null */
static int    g_arr[4];           /* 聚合 -> 全 0 */
static struct { int a; double b; } g_st;  /* 聚合 -> 全 0 */
static union  { int i; double d; } g_un;  /* 联合 -> 首成员 i = 0 */

/* [4] 静态存储期初始化器必须是常量表达式或字符串字面量 */
static int    g_const = 3 + 4;    /* 常量表达式 */
static char   g_str[] = "hi";     /* 字符串字面量 */
static int    g_arr2[3] = { 1, 2, 3 };

/* [11] 标量初始化：单个表达式，可选花括号 */
static int    g_scalar1 = 42;
static int    g_scalar2 = { 42 };

/* [14] 字符数组由字符串字面量初始化 */
static char   g_char_arr[5] = "abcd";   /* 含终止符，正好放满 */
static char   g_char_arr2[] = "abcd";   /* 未知大小 -> 5 个元素 */

/* [15] wchar_t 数组由宽字符串字面量初始化 */
static wchar_t g_warr[5] = L"abcd";
static wchar_t g_warr2[] = L"abcd";

/* [17][18][19] 指定初始化器 */
static int    g_desig[10] = { [2] = 20, [5] = 50, [2] = 22 }; /* 覆盖 [2] */
static struct { int x, y, z; } g_desig_st = { .z = 3, .x = 1 };

/* [20] 嵌套聚合：花括号 vs 无花括号 */
static int    g_nest1[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
static int    g_nest2[2][3] = { 1, 2, 3, 4, 5, 6 };  /* 无内层花括号 */
static struct { int a[2]; int b; } g_nest3 = { { 1, 2 }, 3 };
static struct { int a[2]; int b; } g_nest4 = { 1, 2, 3 }; /* 无内层花括号 */

/* [13] 自动存储期结构体可由同类型表达式初始化 */
struct Point { int x, y; };

static void test_static_storage(void)
{
    /* [10] */
    assert(g_int == 0);
    assert(g_ptr == NULL);
    assert(g_arr[0] == 0 && g_arr[1] == 0 && g_arr[2] == 0 && g_arr[3] == 0);
    assert(g_st.a == 0 && g_st.b == 0.0);
    assert(g_un.i == 0);

    /* [4] */
    assert(g_const == 7);
    assert(strcmp(g_str, "hi") == 0);
    assert(g_arr2[0] == 1 && g_arr2[1] == 2 && g_arr2[2] == 3);

    /* [11] */
    assert(g_scalar1 == 42);
    assert(g_scalar2 == 42);

    /* [14] */
    assert(sizeof g_char_arr == 5);
    assert(g_char_arr[0] == 'a' && g_char_arr[3] == 'd' && g_char_arr[4] == '\0');
    assert(sizeof g_char_arr2 == 5);
    assert(g_char_arr2[4] == '\0');

    /* [15] */
    assert(sizeof g_warr == 5 * sizeof(wchar_t));
    assert(g_warr[0] == L'a' && g_warr[4] == L'\0');
    assert(sizeof g_warr2 == 5 * sizeof(wchar_t));

    /* [17][18][19] */
    assert(g_desig[0] == 0 && g_desig[1] == 0);
    assert(g_desig[2] == 22);   /* 后写覆盖前写 */
    assert(g_desig[3] == 0 && g_desig[4] == 0);
    assert(g_desig[5] == 50);
    assert(g_desig[6] == 0 && g_desig[9] == 0);
    assert(g_desig_st.x == 1 && g_desig_st.y == 0 && g_desig_st.z == 3);

    /* [20] */
    assert(g_nest1[0][0] == 1 && g_nest1[1][2] == 6);
    assert(g_nest2[0][0] == 1 && g_nest2[1][2] == 6);
    assert(g_nest3.a[0] == 1 && g_nest3.a[1] == 2 && g_nest3.b == 3);
    assert(g_nest4.a[0] == 1 && g_nest4.a[1] == 2 && g_nest4.b == 3);
}

static void test_auto_storage(void)
{
    /* [10] 自动存储期未初始化 -> 不确定值，不测试其值，只测试可声明 */
    int uninit;
    (void)uninit;

    /* [11] 标量初始化，可选花括号 */
    int s1 = 7;
    int s2 = { 7 };
    assert(s1 == 7 && s2 == 7);

    /* [13] 自动存储期结构体由同类型表达式初始化 */
    struct Point p1 = { 1, 2 };
    struct Point p2 = p1;          /* 单表达式，兼容结构体类型 */
    assert(p2.x == 1 && p2.y == 2);

    /* [14] 自动存储期字符数组 */
    char ca[4] = "abc";
    assert(ca[0] == 'a' && ca[2] == 'c' && ca[3] == '\0');

    /* [16] 聚合的花括号列表 */
    int arr[3] = { 10, 20, 30 };
    assert(arr[0] == 10 && arr[1] == 20 && arr[2] == 30);

    /* [17] 无指定时按顺序初始化 */
    struct { int a, b, c; } st = { 1, 2, 3 };
    assert(st.a == 1 && st.b == 2 && st.c == 3);

    /* [18] 嵌套指定初始化器 */
    struct { int a[3]; int b; } nst = { .a[1] = 5, .b = 9 };
    assert(nst.a[0] == 0 && nst.a[1] == 5 && nst.a[2] == 0 && nst.b == 9);

    /* [19] 覆盖：后写覆盖前写 */
    int ov[3] = { 1, 2, 3 };
    int ov2[3] = { [0] = 1, [1] = 2, [0] = 100 };
    assert(ov[0] == 1 && ov[1] == 2 && ov[2] == 3);
    assert(ov2[0] == 100 && ov2[1] == 2 && ov2[2] == 0);

    /* [19] 未显式初始化的子对象隐式初始化为静态存储期规则（0） */
    int partial[5] = { 1, 2 };
    assert(partial[0] == 1 && partial[1] == 2);
    assert(partial[2] == 0 && partial[3] == 0 && partial[4] == 0);

    /* [20] 嵌套聚合，内层花括号 */
    int nest[2][2] = { { 1, 2 }, { 3, 4 } };
    assert(nest[0][0] == 1 && nest[1][1] == 4);

    /* [20] 嵌套聚合，无内层花括号：按顺序填满 */
    int nest2[2][2] = { 1, 2, 3, 4 };
    assert(nest2[0][0] == 1 && nest2[0][1] == 2);
    assert(nest2[1][0] == 3 && nest2[1][1] == 4);

    /* [20] 联合：无花括号时只取足够初始化首成员 */
    union { int i; double d; } u = { 5 };
    assert(u.i == 5);

    /* [20] 结构体含联合：无花括号时只初始化联合首成员 */
    struct { int a; union { int i; double d; } u; int b; } su = { 1, 2, 3 };
    assert(su.a == 1 && su.u.i == 2 && su.b == 3);

    /* [17] 联合：无指定时初始化第一个命名成员 */
    union { int i; double d; } u2 = { 42 };
    assert(u2.i == 42);

    /* [11] 标量类型转换：double -> int */
    int conv = 3.9;
    assert(conv == 3);

    /* [11] 标量类型转换：int -> double */
    double conv2 = 5;
    assert(conv2 == 5.0);
}

/* [9] 匿名成员不参与初始化（C99 无匿名成员，此处用普通成员验证 [9] 的
 * “未命名成员不参与初始化”语义在 C99 中不适用；C99 结构体成员均有名）。
 * 因此 [9] 在 C99 中无实际可测对象，仅验证命名成员正常初始化。 */

/* [21] 初始化器数量少于元素数量：剩余隐式初始化为 0 */
static void test_fewer_initializers(void)
{
    int a[5] = { 1, 2 };
    assert(a[0] == 1 && a[1] == 2);
    assert(a[2] == 0 && a[3] == 0 && a[4] == 0);

    struct { int x, y, z; } s = { 1 };
    assert(s.x == 1 && s.y == 0 && s.z == 0);

    /* [14] 字符串字面量比数组短：剩余补 0 */
    char c[6] = "ab";
    assert(c[0] == 'a' && c[1] == 'b');
    assert(c[2] == 0 && c[3] == 0 && c[4] == 0 && c[5] == 0);
}

/* [6] 未知大小数组 + 指定初始化器：非负值有效 */
static int g_unknown[] = { [3] = 30, [0] = 1 };
/* 数组大小由最大下标决定 -> 4 个元素 */

static void test_unknown_size(void)
{
    assert(sizeof g_unknown / sizeof g_unknown[0] == 4);
    assert(g_unknown[0] == 1);
    assert(g_unknown[1] == 0 && g_unknown[2] == 0);
    assert(g_unknown[3] == 30);
}

int main(void)
{
    test_static_storage();
    test_auto_storage();
    test_fewer_initializers();
    test_unknown_size();
    printf("C99 6.7.8 positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 6.7.8 约束，应编译报错
 * ============================================================ */
#if 0

/* [2] 违反约束：初始化器为不存在的对象提供值
 * 期望：gcc -std=c99 报错（excess elements in array initializer） */
int bad_excess[2] = { 1, 2, 3 };

/* [2] 违反约束：结构体初始化器提供过多成员
 * 期望：gcc -std=c99 报错（excess elements in struct initializer） */
struct S2 { int a; int b; };
struct S2 bad_st_excess = { 1, 2, 3 };

/* [3] 违反约束：VLA 不能有初始化器
 * 期望：gcc -std=c99 报错（variable-sized object may not be initialized） */
void bad_vla(int n)
{
    int vla[n] = { 0 };
}

/* [4] 违反约束：静态存储期对象的初始化器不是常量表达式
 * 期望：gcc -std=c99 报错（initializer element is not constant） */
int bad_static_nonconst(void)
{
    int local = 5;
    static int s = local;   /* local 不是常量表达式 */
    return s;
}

/* [5] 违反约束：块作用域且具外部/内部链接的标识符不得有初始化器
 * 期望：gcc -std=c99 报错（declaration of block-scope variable with
 *       external linkage has initializer） */
void bad_block_linkage(void)
{
    extern int ext_var = 5;   /* 块作用域 extern 带初始化器 */
}

/* [6] 违反约束：designator [constant-expression] 要求当前对象为数组类型
 * 期望：gcc -std=c99 报错（array index in non-array initializer） */
struct S3 { int a; int b; };
struct S3 bad_desig_not_array = { [0] = 1 };

/* [6] 违反约束：designator 的表达式必须是整数常量表达式
 * 期望：gcc -std=c99 报错（array index in initializer is not an integer
 *       constant expression） */
int bad_desig_nonconst(void)
{
    int i = 1;
    int a[3] = { [i] = 5 };   /* i 不是整数常量表达式 */
    return a[0];
}

/* [7] 违反约束：designator .identifier 要求当前对象为结构体或联合体
 * 期望：gcc -std=c99 报错（field name not in record or union initializer） */
int bad_desig_not_struct[3] = { .x = 1 };

/* [7] 违反约束：.identifier 的 identifier 必须是该类型的成员
 * 期望：gcc -std=c99 报错（'S4' has no member named 'z'） */
struct S4 { int a; int b; };
struct S4 bad_desig_no_member = { .z = 1 };

/* [11] 违反约束：标量初始化器必须是单个表达式（不能是逗号分隔的多个）
 * 期望：gcc -std=c99 报错（excess elements in scalar initializer） */
int bad_scalar_multi = { 1, 2 };

/* [11] 违反约束：标量初始化器类型不兼容（简单赋值约束）
 * 期望：gcc -std=c99 报错（incompatible types in assignment） */
struct S5 { int a; };
int bad_scalar_incompat = (struct S5){ 1 };

/* [13] 违反约束：自动存储期结构体初始化器类型不兼容
 * 期望：gcc -std=c99 报错（incompatible types when initializing） */
void bad_struct_incompat(void)
{
    struct A { int x; };
    struct B { int y; };
    struct A a = (struct B){ 1 };
}

/* [14] 违反约束：字符数组由字符串字面量初始化时，字面量过长（无终止符空间）
 * 期望：gcc -std=c99 报错（initializer-string for array of chars is too long） */
char bad_str_too_long[3] = "abcd";

/* [15] 违反约束：wchar_t 数组由宽字符串字面量初始化时，字面量过长
 * 期望：gcc -std=c99 报错（initializer-string for array of wide chars is too long） */
wchar_t bad_wstr_too_long[3] = L"abcd";

/* [16] 违反约束：聚合初始化器必须是花括号列表（或 [13] 的单表达式）
 * 期望：gcc -std=c99 报错（invalid initializer） */
struct S6 { int a; int b; };
struct S6 bad_agg_no_brace = 1, 2;

/* [17] 违反约束：联合初始化器提供过多成员
 * 期望：gcc -std=c99 报错（excess elements in union initializer） */
union U1 { int i; double d; };
union U1 bad_union_excess = { 1, 2 };

/* [18] 违反约束：designator 链中成员不存在
 * 期望：gcc -std=c99 报错（'S7' has no member named 'y'） */
struct S7 { int x; };
struct S7 bad_desig_chain = { .y = 1 };

/* [19] 违反约束：同一子对象重复初始化在 C99 中允许（后写覆盖前写），
 * 因此不作为负向测试。此处仅注释说明。 */

/* [20] 违反约束：嵌套聚合初始化器元素过多
 * 期望：gcc -std=c99 报错（excess elements in array initializer） */
int bad_nest_excess[2][2] = { { 1, 2, 3 }, { 4, 5, 6 } };

/* [21] 违反约束：初始化器数量超过元素数量（同 [2]）
 * 期望：gcc -std=c99 报错（excess elements in array initializer） */
int bad_too_many[2] = { 1, 2, 3 };

#endif /* 负向测试结束 */

/*
 * 说明：
 * 1. 正向部分覆盖了 6.7.8 的语法 [1]、约束 [2]-[7] 的合法情形、
 *    语义 [8]-[21] 的可测行为，包括静态/自动存储期、标量/聚合/联合、
 *    字符串/宽字符串、指定初始化器、嵌套聚合、覆盖与隐式零初始化。
 * 2. 负向部分每个片段违反一条约束，放在 #if 0 中，编译器应报错；
 *    实际验证时可将对应片段移出 #if 0 单独编译。
 * 3. [9] 关于匿名成员的语义在 C99 中无匿名成员语法，故无实际可测对象；
 *    [19] 的重复初始化在 C99 中是合法覆盖行为，不作为负向测试。
 */