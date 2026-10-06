/*
 * 测试 C99 6.7 Declarations
 *
 * 预期行为：
 *   正向测试：以下代码应能编译（gcc -std=c99 -Wall）并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，每个片段违反 6.7 的某条约束，编译器应报错。
 *             这些片段被 #if 0 屏蔽，保证本文件整体仍可编译运行。
 *
 * 覆盖段落：[1] 语法、[2] 至少声明一个 declarator/tag/枚举成员、
 *           [3] 无链接标识符同作用域同名字空间只能声明一次、
 *           [4] 同作用域同对象/函数声明类型必须兼容、
 *           [5] 声明 vs 定义、[6] 声明说明符与 init-declarator-list、
 *           [7] 无链接对象类型须完整。
 */

#include <assert.h>
#include <stdio.h>
#include <stddef.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 语法：declaration-specifiers init-declarator-list ;
 * [6] 声明说明符序列 + 逗号分隔的 init-declarator-list */
static int g_a = 1, g_b = 2, g_c;   /* 多个 declarator，部分带初始化器 */

/* [1] storage-class-specifier + type-specifier + type-qualifier 组合 */
static const int g_const = 42;
static volatile int g_vol = 7;

/* [1] function-specifier 出现在声明说明符中 */
inline static int add_one(int x) { return x + 1; }

/* [5] 定义：为对象保留存储 / 为函数包含函数体 */
int g_defined_object = 100;         /* 对象定义 */
int g_defined_func(void) { return 5; } /* 函数定义 */

/* [5] typedef 名：其（唯一）声明即为定义 */
typedef int myint_t;

/* [5] 枚举常量：其（唯一）声明即为定义 */
enum Color { RED = 1, GREEN = 2, BLUE = 3 };

/* [2] 只声明 tag（不声明 declarator）是合法的 */
struct OnlyTag { int x; };
union  OnlyUnion { int i; float f; };
enum   OnlyEnum { E0, E1 };

/* [2] 只声明枚举成员（枚举说明符本身）也是合法的 */
enum { ANON_A = 10, ANON_B = 20 };

/* [3] 无链接标识符：块作用域内只声明一次 */
static void test_no_linkage_once(void)
{
    int local = 3;      /* 块作用域，无链接，仅一次声明 */
    assert(local == 3);
}

/* [4] 同作用域同对象/函数的声明类型必须兼容：
 *     先声明后定义，类型一致 */
int compat_func(int);          /* 声明 */
int compat_func(int x) { return x * 2; }  /* 定义，类型兼容 */

/* [7] 无链接对象类型须在 declarator 结束前完整 */
static void test_complete_type(void)
{
    struct Complete { int a; int b; };
    struct Complete obj = { 1, 2 };   /* 类型在 declarator 结束前已完整 */
    assert(obj.a == 1 && obj.b == 2);
}

/* [7] 函数参数（含原型）要求调整后的类型完整 */
static int param_complete(struct Complete2 { int v; } p)  /* 参数类型完整 */
{
    return p.v;
}

/* [6] declarator 可携带额外类型信息（指针、数组、函数） */
static int  arr[3] = { 1, 2, 3 };
static int *ptr = arr;
static int (*fp)(int) = add_one;

int main(void)
{
    /* [6] init-declarator-list 中每个 declarator 可带初始化器 */
    assert(g_a == 1 && g_b == 2);
    g_c = g_a + g_b;
    assert(g_c == 3);

    /* [1] type-qualifier 语义 */
    assert(g_const == 42);
    g_vol = 8;
    assert(g_vol == 8);

    /* [1] function-specifier */
    assert(add_one(1) == 2);

    /* [5] 定义语义 */
    assert(g_defined_object == 100);
    assert(g_defined_func() == 5);

    /* [5] typedef 名与枚举常量 */
    myint_t m = 9;
    assert(m == 9);
    assert(RED == 1 && GREEN == 2 && BLUE == 3);

    /* [2] 仅 tag 声明 */
    struct OnlyTag t = { 5 };
    union  OnlyUnion u; u.i = 6;
    enum   OnlyEnum e = E1;
    assert(t.x == 5 && u.i == 6 && e == E1);

    /* [2] 匿名枚举成员 */
    assert(ANON_A == 10 && ANON_B == 20);

    /* [3] 无链接标识符单次声明 */
    test_no_linkage_once();

    /* [4] 兼容类型声明 */
    assert(compat_func(21) == 42);

    /* [7] 完整类型 */
    test_complete_type();

    /* [7] 参数调整后类型完整 */
    {
        struct Complete2 c2 = { 77 };
        assert(param_complete(c2) == 77);
    }

    /* [6] declarator 携带额外类型信息 */
    assert(arr[0] == 1 && arr[2] == 3);
    assert(*ptr == 1);
    assert(fp(10) == 11);

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 6.7 约束，应编译报错
 * ============================================================ */
#if 0

/* [2] 违反约束「声明至少声明一个 declarator、tag 或枚举成员」：
 *     仅有声明说明符而无 declarator/tag/枚举成员。
 *     期望：gcc -std=c99 报错（如 "useless type name in empty declaration"）。 */
int ;

/* [2] 同上：仅有类型限定符，无 declarator/tag。 */
const ;

/* [3] 违反约束「无链接标识符同作用域同名字空间只能声明一次」：
 *     块作用域内同名局部变量重复声明。
 *     期望：gcc -std=c99 报错（redeclaration of 'x'）。 */
void f_redecl(void)
{
    int x;
    int x;   /* 错误：同一作用域、同一名字空间重复声明 */
}

/* [3] 同上：同一作用域内 typedef 名重复声明。 */
void f_redecl_typedef(void)
{
    typedef int T;
    typedef int T;   /* 错误：重复声明 */
}

/* [4] 违反约束「同作用域同对象/函数声明类型必须兼容」：
 *     同一函数先声明为 int(int)，后声明为 double(double)。
 *     期望：gcc -std=c99 报错（conflicting types for 'g'）。 */
int g_conflict(int);
double g_conflict(double);   /* 错误：类型不兼容 */

/* [4] 同上：同一对象先声明为 int，后声明为 float。 */
extern int  obj_conflict;
extern float obj_conflict;   /* 错误：类型不兼容 */

/* [7] 违反约束「无链接对象类型须在 declarator 结束前完整」：
 *     块作用域内声明不完整类型的对象（无链接）。
 *     期望：gcc -std=c99 报错（storage size of 'inc' isn't known）。 */
void f_incomplete(void)
{
    struct Incomplete;          /* 仅声明 tag，类型不完整 */
    struct Incomplete inc;      /* 错误：无链接对象类型不完整 */
}

/* [7] 同上：带初始化器时，类型须在 init-declarator 结束前完整。 */
void f_incomplete_init(void)
{
    struct Incomplete2;
    struct Incomplete2 inc2 = { 0 };  /* 错误：类型不完整 */
}

#endif /* 负向测试结束 */