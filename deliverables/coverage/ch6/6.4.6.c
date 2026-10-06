/*
 * 测试 C99 6.4.6 Punctuators（标点符号）
 *
 * 预期行为：
 *   - 正向测试：以下使用各种标点符号（运算符、分隔符、digraph）的代码
 *     应能正常编译并运行，assert 全部通过。
 *   - 负向测试：违反标点符号语法/语义约束的片段应导致编译报错，
 *     统一放在 #if 0 ... #endif 中，保证本文件整体仍可编译运行。
 *
 * 覆盖段落：
 *   [1] punctuator 列表（含 digraph 拼写）
 *   [2] 标点符号的语法/语义意义：运算符、操作数
 *   [3] digraph 与主拼写等价（<: :> <% %> %: %:%:）
 *   Footnote 68: 字符串化时 [ 与 <: 行为不同
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 用 digraph 拼写写一个头文件包含（<% 等价于 {，%> 等价于 }） */
/* 这里用宏演示 <% %> 作为块分隔符 */
#define BLOCK_BEGIN <%
#define BLOCK_END   %>

/* [1][3] digraph 作为数组下标与初始化列表分隔符 */
static int arr_digraph[] = <%
    10, 20, 30
%>;

/* [1][3] %: 等价于 #，用于字符串化 */
#define STR(x) %:x
#define STR2(x) #x

/* [1][3] %:%: 等价于 ##，用于记号粘贴 */
#define GLUE(a, b) a %:%: b

/* [2] 运算符作用于操作数：算术、关系、逻辑、位、赋值、条件、逗号 */
static int test_operators(void)
{
    int a = 5, b = 3, c = 0;

    /* 算术运算符 */
    assert(a + b == 8);
    assert(a - b == 2);
    assert(a * b == 15);
    assert(a / b == 1);
    assert(a % b == 2);

    /* 一元运算符 */
    assert(-a == -5);
    assert(+a == 5);
    assert(~a == -6);
    assert(!c == 1);

    /* 自增自减 */
    c = a++;
    assert(c == 5 && a == 6);
    c = a--;
    assert(c == 6 && a == 5);

    /* 移位 */
    assert((1 << 3) == 8);
    assert((16 >> 2) == 4);

    /* 关系与相等 */
    assert((a > b) == 1);
    assert((a < b) == 0);
    assert((a >= b) == 1);
    assert((a <= b) == 0);
    assert((a == b) == 0);
    assert((a != b) == 1);

    /* 位运算 */
    assert((a & b) == 1);
    assert((a | b) == 7);
    assert((a ^ b) == 6);

    /* 逻辑运算 */
    assert((a && b) == 1);
    assert((c || b) == 1);

    /* 条件运算符 */
    assert((a > b ? 100 : 200) == 100);

    /* 逗号运算符 */
    c = (a = 1, b = 2, a + b);
    assert(c == 3);

    /* 复合赋值 */
    c = 10;
    c += 5;  assert(c == 15);
    c -= 3;  assert(c == 12);
    c *= 2;  assert(c == 24);
    c /= 4;  assert(c == 6);
    c %= 4;  assert(c == 2);
    c <<= 3; assert(c == 16);
    c >>= 2; assert(c == 4);
    c &= 6;  assert(c == 4);
    c ^= 1;  assert(c == 5);
    c |= 2;  assert(c == 7);

    return 0;
}

/* [2] 成员访问运算符 . 和 -> */
struct Point { int x, y; };

static int test_member_access(void)
{
    struct Point p = { 3, 4 };
    struct Point *pp = &p;

    assert(p.x == 3);
    assert(p.y == 4);
    assert(pp->x == 3);
    assert(pp->y == 4);

    /* 取地址与解引用 */
    int v = 42;
    int *pv = &v;
    assert(*pv == 42);

    return 0;
}

/* [1][3] digraph 等价性测试 */
static int test_digraphs(void)
{
    /* <: 等价于 [，:> 等价于 ] */
    int a<:3:> = <:1, 2, 3:>;
    assert(a<:0:> == 1);
    assert(a<:1:> == 2);
    assert(a<:2:> == 3);

    /* <% %> 等价于 { } */
    int x = 0;
    if (1) BLOCK_BEGIN
        x = 99;
    BLOCK_END
    assert(x == 99);

    /* %: 等价于 #（字符串化） */
    const char *s1 = STR(hello);
    const char *s2 = STR2(hello);
    assert(strcmp(s1, "hello") == 0);
    assert(strcmp(s2, "hello") == 0);

    /* %:%: 等价于 ##（记号粘贴） */
    int GLUE(my, var) = 7;
    assert(myvar == 7);

    /* 数组下标使用 digraph */
    assert(arr_digraph<:0:> == 10);
    assert(arr_digraph<:1:> == 20);
    assert(arr_digraph<:2:> == 30);

    return 0;
}

/* [1] 省略号 ... 作为标点符号（变参函数） */
#include <stdarg.h>

static int sum_ints(int n, ...)
{
    va_list ap;
    int total = 0, i;
    va_start(ap, n);
    for (i = 0; i < n; i++)
        total += va_arg(ap, int);
    va_end(ap);
    return total;
}

static int test_ellipsis(void)
{
    assert(sum_ints(3, 1, 2, 3) == 6);
    assert(sum_ints(0) == 0);
    return 0;
}

/* [1] 各种分隔符：; , : ? ( ) [ ] { } */
static int test_separators(void)
{
    int i, sum = 0;
    for (i = 0; i < 5; i++) {
        sum += i;
    }
    assert(sum == 10);

    /* 三目运算符中的 : 与 ? */
    int r = (sum > 5) ? 1 : 0;
    assert(r == 1);

    return 0;
}

/* [Footnote 68] 字符串化时 [ 与 <: 行为不同 */
#define STRINGIZE(x) #x
#define XSTRINGIZE(x) STRINGIZE(x)

static int test_stringize_difference(void)
{
    /* 直接字符串化 [ 得到 "[" */
    const char *s1 = STRINGIZE([);
    assert(strcmp(s1, "[") == 0);

    /* 直接字符串化 <: 得到 "<:"（拼写不同） */
    const char *s2 = STRINGIZE(<:);
    assert(strcmp(s2, "<:") == 0);

    /* 两者拼写不同，故字符串化结果不同 */
    assert(strcmp(s1, s2) != 0);

    return 0;
}

int main(void)
{
    test_operators();
    test_member_access();
    test_digraphs();
    test_ellipsis();
    test_separators();
    test_stringize_difference();

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「标点符号必须构成合法的记号序列」：
 * 单独的 @ 不是 C99 标点符号，gcc -std=c99 应报错
 * error: stray '@' in program
 */
int bad1 = 1 @ 2;

/* 违反约束「标点符号必须构成合法的记号序列」：
 * 单独的 $ 不是 C99 标点符号，应报错
 * error: stray '$' in program
 */
int bad2 = $;

/* 违反约束「标点符号必须构成合法的记号序列」：
 * 单独的 ` 不是 C99 标点符号，应报错
 * error: stray '`' in program
 */
int bad3 = `;

/* 违反约束「运算符必须作用于合法操作数」：
 * 两个结构体不能相乘，应报错
 * error: invalid operands to binary * (have 'struct S' and 'struct S')
 */
struct S { int x; } sa, sb;
void bad4(void) { sa * sb; }

/* 违反约束「运算符必须作用于合法操作数」：
 * 对非左值赋值，应报错
 * error: lvalue required as left operand of assignment
 */
void bad5(void) { 1 = 2; }

/* 违反约束「运算符必须作用于合法操作数」：
 * 对强制转换结果赋值，应报错
 * error: lvalue required as left operand of assignment
 */
void bad6(void) { (int)3 = 4; }

/* 违反约束「运算符必须作用于合法操作数」：
 * 对条件表达式结果赋值，应报错
 * error: lvalue required as left operand of assignment
 */
void bad7(void) { int a = 1, b = 2; (a ? a : b) = 5; }

/* 违反约束「运算符必须作用于合法操作数」：
 * 对逗号表达式结果赋值，应报错
 * error: lvalue required as left operand of assignment
 */
void bad8(void) { int a = 1, b = 2; (a, b) = 5; }

/* 违反约束「运算符必须作用于合法操作数」：
 * 对函数返回结构体的成员赋值，应报错
 * error: lvalue required as left operand of assignment
 */
struct T { int x; };
struct T make_t(void) { struct T t = { 0 }; return t; }
void bad9(void) { make_t().x = 5; }

/* 违反约束「-> 左操作数必须为指针」：
 * 对非指针使用 ->，应报错
 * error: invalid type argument of '->' (have 'int')
 */
void bad10(void) { int n = 0; n->x; }

/* 违反约束「. 左操作数必须为结构体或联合体」：
 * 对非结构体使用 .，应报错
 * error: request for member 'x' in something not a structure or union
 */
void bad11(void) { int n = 0; n.x; }

/* 违反约束「digraph 必须成对使用」：
 * 只有 <: 没有 :>，应报错
 * error: expected ']' before ';' token
 */
void bad12(void) { int a<:3; }

/* 违反约束「省略号只能出现在参数列表末尾」：
 * ... 后还有参数，应报错
 * error: '...' must be the last parameter
 */
void bad13(int a, ... , int b);

/* 违反约束「省略号只能出现在参数列表中」：
 * 在表达式中使用 ...，应报错
 * error: expected expression before '...' token
 */
void bad14(void) { int x = ...; }

/* 违反约束「%: 只能用于预处理指令中作为 # 的等价物」：
 * 在普通表达式中使用 %: 作为运算符，应报错
 * error: expected expression before '%:' token
 */
void bad15(void) { int x = 1 %: 2; }

#endif /* 负向测试结束 */