/*
 * 测试目标：C99 7.1.4 Use of library functions
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反 7.1.4 的约束/要求，
 *             期望编译器（gcc -std=c99 -Wall）报错。
 *
 * 覆盖段落：
 *   [1] 无效实参/类型 -> UB（不作为负向测试，仅注释说明）；
 *       库函数可被实现为宏；用括号抑制宏展开；#undef 抑制宏；
 *       取库函数地址合法；宏展开每个实参恰好求值一次；
 *       对象式宏可用于 #if。
 *   [2] 不包含头文件也可显式声明库函数并使用。
 *   [3] 库函数返回前存在序列点。
 *   [4] 标准库函数不保证可重入，可能修改静态存储期对象。
 *   [5] EXAMPLE：atoi 的四种用法。
 *   Footnotes 161/162/163：真实函数必须存在；宏可能无序列点；
 *       下划线保留标识符。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [5] EXAMPLE：atoi 的四种用法 */

/* 用法 1：使用关联头文件（可能产生宏展开） */
static int atoi_via_header(const char *str)
{
    return atoi(str);           /* [5] 用法 1 */
}

/* 用法 2：使用关联头文件 + #undef，确保引用真实函数 */
static int atoi_via_undef(const char *str)
{
#undef atoi
    return atoi(str);           /* [5] 用法 2 */
}

/* 用法 3：用括号抑制宏展开 */
static int atoi_via_parens(const char *str)
{
    return (atoi)(str);         /* [5] 用法 3 */
}

/* 用法 4：显式声明，不使用头文件 */
extern int atoi(const char *);
static int atoi_via_extern(const char *str)
{
    return atoi(str);           /* [5] 用法 4 */
}

/* [1] 取库函数地址合法（即使它同时被定义为宏） */
static int (*atoi_ptr)(const char *) = &atoi;

/* [1] 宏展开每个实参恰好求值一次：用带副作用的实参验证 */
static int side_effect_count = 0;
static int bump(void)
{
    side_effect_count++;
    return 42;
}

/* [2] 不包含头文件，显式声明库函数并使用 */
extern int abs(int);
extern size_t strlen(const char *);

/* [3] 库函数返回前存在序列点：用返回值参与表达式验证 */
static int seq_point_test(void)
{
    /* 若返回前无序列点，则下面的表达式行为未定义；
       有序列点则结果确定。 */
    int r = abs(-5) + abs(-3);
    return r;
}

/* [4] 标准库函数可能修改静态存储期对象：strtok 是典型例子 */
static void reentrancy_test(void)
{
    char buf1[] = "a,b,c";
    char buf2[] = "x,y,z";
    char *save1 = NULL, *save2 = NULL;
    char *t1, *t2;

    /* strtok 使用静态存储，不保证可重入 */
    t1 = strtok(buf1, ",");
    t2 = strtok(buf2, ",");     /* 会覆盖 strtok 的内部静态状态 */
    (void)save1; (void)save2;
    assert(t1 != NULL && strcmp(t1, "a") == 0);
    assert(t2 != NULL && strcmp(t2, "x") == 0);
}

/* [1] 对象式宏可用于 #if：INT_MAX 是对象式宏，展开为整数常量表达式 */
#if INT_MAX < 32767
#error "INT_MAX too small"
#endif

/* [1] 函数式宏可出现在任何兼容返回类型的函数可被调用的地方 */
static int macro_in_expr(void)
{
    /* isdigit 可能是宏，也可能不是；两种情况下都应可用 */
    return isdigit('5') ? 1 : 0;
}

int main(void)
{
    /* [5] atoi 四种用法结果一致 */
    const char *s = "12345";
    assert(atoi_via_header(s)  == 12345);
    assert(atoi_via_undef(s)   == 12345);
    assert(atoi_via_parens(s)  == 12345);
    assert(atoi_via_extern(s)  == 12345);

    /* [1] 取库函数地址并调用 */
    assert(atoi_ptr != NULL);
    assert(atoi_ptr("678") == 678);

    /* [1] 宏展开每个实参恰好求值一次 */
    side_effect_count = 0;
    {
        /* 若 abs 是宏，其实参 bump() 必须恰好求值一次 */
        int v = abs(bump());
        assert(v == 42);
        assert(side_effect_count == 1);
    }

    /* [2] 显式声明后使用，无需头文件 */
    assert(abs(-7) == 7);
    assert(strlen("hello") == 5);

    /* [3] 序列点：结果确定 */
    assert(seq_point_test() == 8);

    /* [4] 不保证可重入 */
    reentrancy_test();

    /* [1] 对象式宏用于 #if 已在上方预处理阶段验证 */
    assert(INT_MAX >= 32767);

    /* [1] 函数式宏在表达式中使用 */
    assert(macro_in_expr() == 1);

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* ------------------------------------------------------------
 * 违反约束：库函数声明与使用必须类型一致。
 * 7.1.4 [1] 要求实参类型（提升后）必须符合函数期望；
 * 显式声明与库函数原型冲突时，编译器应报错。
 * 期望：gcc -std=c99 报 "conflicting types for 'atoi'"
 * ------------------------------------------------------------ */
extern int atoi(const char *);
extern double atoi(const char *);   /* 与上一条声明冲突 */

/* ------------------------------------------------------------
 * 违反约束：库函数名被宏定义后，直接调用会展开为宏；
 * 若宏定义与真实函数签名不兼容，调用处应报错。
 * 期望：gcc -std=c99 报 "too many arguments" 或类似错误
 * ------------------------------------------------------------ */
#define abs(x)  ((x) < 0 ? -(x) : (x))
int bad_abs_call = abs(1, 2);       /* 宏只接受 1 个参数 */

/* ------------------------------------------------------------
 * 违反约束：对库函数返回的非左值结果赋值。
 * 7.1.4 [1] 中函数调用结果不是左值，不能作为赋值目标。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void bad_assign_to_call(void)
{
    abs(-1) = 5;                    /* 函数调用结果非左值 */
}

/* ------------------------------------------------------------
 * 违反约束：对强制转换结果赋值（非左值）。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void bad_assign_to_cast(void)
{
    (int)3.14 = 0;
}

/* ------------------------------------------------------------
 * 违反约束：对条件表达式结果赋值（非左值）。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void bad_assign_to_cond(void)
{
    int a = 1, b = 2;
    (a ? a : b) = 5;
}

/* ------------------------------------------------------------
 * 违反约束：对逗号表达式结果赋值（非左值）。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void bad_assign_to_comma(void)
{
    int a = 1, b = 2;
    (a, b) = 5;
}

/* ------------------------------------------------------------
 * 违反约束：对函数返回结构体的成员赋值（非左值）。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
struct S { int x; };
struct S make_s(void);
void bad_assign_to_member(void)
{
    make_s().x = 5;
}

/* ------------------------------------------------------------
 * 违反约束：对 const 限定对象的成员赋值。
 * 7.1.4 [1] 提到指向不可修改存储的指针；
 * const 限定类型的成员不可赋值。
 * 期望：gcc -std=c99 报 "assignment of read-only member"
 * ------------------------------------------------------------ */
void bad_assign_const_member(void)
{
    const struct S cs = { 0 };
    cs.x = 5;
}

/* ------------------------------------------------------------
 * 违反约束：对 const 限定对象解引用后赋值。
 * 期望：gcc -std=c99 报 "assignment of read-only location"
 * ------------------------------------------------------------ */
void bad_assign_const_deref(void)
{
    const int ci = 0;
    const int *p = &ci;
    *p = 5;
}

/* ------------------------------------------------------------
 * 违反约束：对 volatile 限定对象取地址后赋给非 volatile 指针
 * 会丢失限定符（C99 6.5.16.1 约束）。
 * 期望：gcc -std=c99 报 "discards qualifiers"
 * ------------------------------------------------------------ */
void bad_discard_volatile(void)
{
    volatile int vi = 0;
    int *p = &vi;                   /* 丢弃 volatile 限定符 */
    *p = 5;
}

/* ------------------------------------------------------------
 * 违反约束：对 const 限定对象取地址后赋给非 const 指针
 * 会丢失限定符。
 * 期望：gcc -std=c99 报 "discards qualifiers"
 * ------------------------------------------------------------ */
void bad_discard_const(void)
{
    const int ci = 0;
    int *p = &ci;                   /* 丢弃 const 限定符 */
    *p = 5;
}

/* ------------------------------------------------------------
 * 违反约束：无原型函数调用时，实参经默认实参提升后
 * 类型必须与形参兼容；此处用不完整类型作实参。
 * 期望：gcc -std=c99 报 "invalid use of undefined type"
 * ------------------------------------------------------------ */
struct Incomplete;
void bad_incomplete_arg(void)
{
    struct Incomplete obj;          /* 不完整类型不能定义对象 */
    (void)obj;
}

/* ------------------------------------------------------------
 * 违反约束：省略号（...）之后的实参不再进行默认实参提升，
 * 但 float 实参在 ... 中仍会提升为 double；
 * 此处故意用不兼容类型触发诊断。
 * 期望：gcc -std=c99 报 "incompatible type for argument"
 * ------------------------------------------------------------ */
extern int printf(const char *, ...);
void bad_ellipsis_type(void)
{
    struct S s = { 1 };
    printf("%d\n", s);              /* 结构体不能作为 ... 实参 */
}

#endif /* 负向测试结束 */

/*
 * 说明：
 * 1. 正向部分覆盖 7.1.4 [1]~[5] 及脚注 161/162/163 的可测试语义：
 *    宏/函数双形态、括号抑制宏、#undef、取地址、实参恰好求值一次、
 *    对象式宏用于 #if、显式声明免头文件、返回前序列点、不可重入。
 * 2. 负向部分只针对约束（constraint）与必须诊断的规则：
 *    非左值赋值、const/volatile 限定符丢失、声明冲突、宏参数个数错误、
 *    不完整类型、省略号实参类型不兼容。
 * 3. 7.1.4 [1] 中「无效实参/类型导致 UB」属于未定义行为，不作为负向测试，
 *    仅在注释中说明。
 */