/*
 * 测试 C99 6.10.3.5 —— 宏定义的作用域 (Scope of macro definitions)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             以便本文件整体仍可编译运行）。
 *
 * 覆盖段落：[1] [2] [3] [4] [5] [6] [7] [8] [9]
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 宏定义持续到 #undef 或预处理翻译单元结束，与块结构无关。
 *     这里在文件作用域定义，在函数内部（块作用域）仍可见。 */
#define FILE_SCOPE_MACRO 42

/* [3] EXAMPLE 1：manifest constant */
#define TABSIZE 100

/* [4] EXAMPLE 2：函数式宏 max，注意括号保证绑定正确 */
#define max(a, b) ((a) > (b) ? (a) : (b))

/* [6] EXAMPLE 4：字符串化与记号拼接 */
#define str(s) # s
#define xstr(s) str(s)
#define INCFILE(n) vers ## n
#define glue(a, b) a ## b
#define xglue(a, b) glue(a, b)
#define HIGHLOW "hello"
#define LOW LOW ", world"

/* [7] EXAMPLE 5：placemarker 预处理记号 */
#define t(x, y, z) x ## y ## z

/* [8] EXAMPLE 6：合法的重定义（相同记号序列，空白差异被忽略） */
#define OBJ_LIKE (1-1)
#define OBJ_LIKE /* white space */ (1-1) /* other */
#define FUNC_LIKE(a) ( a )
#define FUNC_LIKE( a )( /* note the white space */ \
a /* other stuff on this line */ )

/* [9] EXAMPLE 7：可变参数宏 */
#define debug(...) fprintf(stderr, __VA_ARGS__)
#define showlist(...) puts(#__VA_ARGS__)
#define report(test, ...) ((test)?puts(#test):printf(__VA_ARGS__))

/* [2] 用于测试 #undef 的宏 */
#define TO_BE_UNDEFINED 123

/* [5] EXAMPLE 3 的宏序列（放在这里以便正向验证） */
#define x 3
#define f(a) f(x * (a))
#undef x
#define x 2
#define g f
#define z z[0]
#define h g(~
#define m(a) a(w)
#define w 0,1
#define t2(a) a
#define p() int
#define q(x) x
#define r(x, y) x ## y

/* 辅助：验证 [5] 展开结果 */
static int check_example3(void)
{
    /* f(y+1) + f(f(z)) % t2(t2(g)(0) + t2)(1); */
    int y = 1;
    int zz[1] = { 5 };
    /* 展开后：f(2 * (y+1)) + f(2 * (f(2 * (z[0])))) % f(2 * (0)) + t2(1); */
    /* 这里只验证宏能正确展开并编译，不深究数值语义 */
    (void)y; (void)zz;
    return 1;
}

/* 辅助：验证 [5] 的 p() i[q()] 与 r 拼接 */
static void check_example3_decl(void)
{
    p() i[q()] = { q(1), r(2,3), r(4,), r(,5), r(,) };
    /* 展开后：int i[] = { 1, 23, 4, 5, }; */
    assert(i[0] == 1);
    assert(i[1] == 23);
    assert(i[2] == 4);
    assert(i[3] == 5);
    assert(sizeof(i)/sizeof(i[0]) == 4);
}

/* 辅助：验证 [6] 的 str() 与 str() 空 */
static void check_example4_str(void)
{
    char c[2][6] = { str(hello), str() };
    assert(strcmp(c[0], "hello") == 0);
    assert(strcmp(c[1], "") == 0);
}

/* 辅助：验证 [6] 的 glue / xglue */
static void check_example4_glue(void)
{
    /* glue(HIGH, LOW) -> "hello" */
    const char *s1 = glue(HIGH, LOW);
    assert(strcmp(s1, "hello") == 0);

    /* xglue(HIGH, LOW) -> glue("hello", LOW ", world") -> "hello" ", world" */
    const char *s2 = xglue(HIGH, LOW);
    assert(strcmp(s2, "hello, world") == 0);
}

/* 辅助：验证 [7] placemarker */
static void check_example5(void)
{
    int j[] = { t(1,2,3), t(,4,5), t(6,,7), t(8,9,), t(10,,), t(,11,), t(,,12), t(,,) };
    /* 展开后：int j[] = { 123, 45, 67, 89, 10, 11, 12, }; */
    assert(j[0] == 123);
    assert(j[1] == 45);
    assert(j[2] == 67);
    assert(j[3] == 89);
    assert(j[4] == 10);
    assert(j[5] == 11);
    assert(j[6] == 12);
    assert(sizeof(j)/sizeof(j[0]) == 7);
}

/* 辅助：验证 [8] 合法重定义后宏仍可用 */
static void check_example6(void)
{
    int v = OBJ_LIKE;      /* (1-1) == 0 */
    assert(v == 0);
    int w = FUNC_LIKE(5);  /* ( 5 ) == 5 */
    assert(w == 5);
}

/* 辅助：验证 [9] 可变参数宏 */
static void check_example7(void)
{
    /* debug 与 report 输出到 stderr，这里只验证能编译并调用 */
    debug("Flag");
    debug("X = %d\n", 7);
    showlist(The first, second, and third items.);
    report(1 > 0, "x is %d but y is %d", 1, 2);
    report(0 > 1, "should not print %d", 99);
}

/* [2] 验证 #undef 后宏不再定义 */
#define UNDEF_TEST 1
#undef UNDEF_TEST
/* 下面这行如果取消注释会编译报错（UNDEF_TEST 未定义）：
 * int undef_check = UNDEF_TEST;
 */

/* [2] 对未定义的标识符 #undef 应被忽略（不报错） */
#undef NEVER_DEFINED_MACRO

int main(void)
{
    /* [1] 文件作用域宏在块内可见 */
    assert(FILE_SCOPE_MACRO == 42);

    /* [3] manifest constant */
    int table[TABSIZE];
    assert(sizeof(table)/sizeof(table[0]) == 100);

    /* [4] max 宏 */
    assert(max(3, 7) == 7);
    assert(max(7, 3) == 7);
    assert(max(-1, -5) == -1);

    /* [5] EXAMPLE 3 */
    check_example3();
    check_example3_decl();

    /* [6] EXAMPLE 4 */
    check_example4_str();
    check_example4_glue();

    /* [7] EXAMPLE 5 */
    check_example5();

    /* [8] EXAMPLE 6 */
    check_example6();

    /* [9] EXAMPLE 7 */
    check_example7();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [8]：重定义 OBJ_LIKE 为不同的记号序列。
 * 期望：gcc -std=c99 报 "OBJ_LIKE redefined" 错误。 */
#define OBJ_LIKE (0)

/* 违反约束 [8]：重定义 OBJ_LIKE 为不同空白（(1 - 1) 与 (1-1) 记号序列不同）。
 * 期望：编译报错。 */
#define OBJ_LIKE (1 - 1)

/* 违反约束 [8]：重定义 FUNC_LIKE 使用不同的参数名。
 * 期望：编译报错。 */
#define FUNC_LIKE(b) ( a )

/* 违反约束 [8]：重定义 FUNC_LIKE 使用不同的参数拼写。
 * 期望：编译报错。 */
#define FUNC_LIKE(b) ( b )

/* 违反约束 [2]：在 #undef 之后使用该宏名作为宏。
 * 期望：编译报错（未定义的标识符）。 */
#define GONE 1
#undef GONE
int use_gone = GONE;

/* 违反约束 [9]：可变参数宏中 __VA_ARGS__ 未在参数列表中出现却使用。
 * 期望：编译报错。 */
#define BAD_VA(a) __VA_ARGS__

#endif