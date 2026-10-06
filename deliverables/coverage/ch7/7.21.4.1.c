/*
 * 测试条款：C99 7.21.4.1  The memcmp function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明 #include <string.h>
 *       int memcmp(const void *s1, const void *s2, size_t n);
 *   [2] 比较 s1 与 s2 所指对象的前 n 个字符
 *   [3] 返回值 >0 / ==0 / <0 分别对应 s1 大于 / 等于 / 小于 s2
 *   Footnote 271) 结构体填充字节、短于分配空间的字符串、联合体比较问题
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型可用性：包含 <string.h> 后 memcmp 可被调用，返回 int，
 *     参数为 (const void *, const void *, size_t)。
 *     这里用一个函数指针来静态验证原型签名。 */
static int (*memcmp_proto_check)(const void *, const void *, size_t) = memcmp;

static void test_prototype(void)
{
    /* [1] 原型签名匹配（若签名不符，上面的初始化会编译报错/警告） */
    assert(memcmp_proto_check == memcmp);
}

static void test_equal(void)
{
    /* [2][3] 相同内容 -> 返回 0 */
    const char a[] = "hello";
    const char b[] = "hello";
    int r = memcmp(a, b, sizeof(a));
    assert(r == 0);

    /* [2] n == 0 时，不比较任何字符，应返回 0（相等） */
    assert(memcmp(a, b, 0) == 0);

    /* [2] 只比较前 n 个字符：前 3 个相同即返回 0 */
    assert(memcmp("abcdef", "abcxyz", 3) == 0);
}

static void test_greater_less(void)
{
    /* [3] s1 > s2 -> 返回值 > 0 */
    const char hi[] = "b";
    const char lo[] = "a";
    assert(memcmp(hi, lo, 1) > 0);

    /* [3] s1 < s2 -> 返回值 < 0 */
    assert(memcmp(lo, hi, 1) < 0);

    /* [3] 多字节：'z' > 'a' */
    assert(memcmp("zebra", "apple", 1) > 0);
    assert(memcmp("apple", "zebra", 1) < 0);

    /* [3] 符号性：返回值只需与 0 比较，不要求具体数值 */
    int r1 = memcmp("abc", "abd", 3);
    int r2 = memcmp("abd", "abc", 3);
    assert(r1 < 0);
    assert(r2 > 0);
    /* 反向比较的符号应相反 */
    assert((r1 < 0) == (r2 > 0));
}

static void test_binary_data(void)
{
    /* [2] 按“字符”（字节）比较，可处理含 0 的二进制数据 */
    const unsigned char x[] = { 0x00, 0x01, 0x02, 0xFF };
    const unsigned char y[] = { 0x00, 0x01, 0x02, 0xFE };
    /* 前 3 字节相同 */
    assert(memcmp(x, y, 3) == 0);
    /* 第 4 字节 0xFF > 0xFE */
    assert(memcmp(x, y, 4) > 0);
    assert(memcmp(y, x, 4) < 0);

    /* [2] 比较的是无符号字节值：0x80 (128) > 0x7F (127) */
    const unsigned char p[] = { 0x80 };
    const unsigned char q[] = { 0x7F };
    assert(memcmp(p, q, 1) > 0);
}

static void test_void_pointer(void)
{
    /* [1][2] 参数为 const void *，可接受任意对象指针 */
    int ia = 0x01020304;
    int ib = 0x01020304;
    assert(memcmp(&ia, &ib, sizeof ia) == 0);

    double da = 1.5, db = 1.5;
    assert(memcmp(&da, &db, sizeof da) == 0);

    /* 混合类型指针也能作为 void* 传入 */
    char buf[4];
    memcpy(buf, &ia, 4);
    assert(memcmp(buf, &ia, 4) == 0);
}

static void test_prefix_ordering(void)
{
    /* [2][3] 前缀关系：较短者视为较小（在相同前缀后遇到 '\0'） */
    const char s1[] = "abc";
    const char s2[] = "abcd";
    /* 比较前 3 字节相同 -> 0 */
    assert(memcmp(s1, s2, 3) == 0);
    /* 比较前 4 字节：s1 第 4 字节为 '\0'，s2 为 'd' -> s1 < s2 */
    assert(memcmp(s1, s2, 4) < 0);
    assert(memcmp(s2, s1, 4) > 0);
}

/* Footnote 271) 结构体填充字节不确定，因此比较整个结构体可能不可靠。
 * 这里演示“只比较确定成员”的安全做法，并验证 memcmp 对确定字节的行为。 */
struct Padded {
    char  c;
    int   i;   /* 可能引入填充字节 */
};

static void test_footnote271(void)
{
    struct Padded a, b;
    memset(&a, 0, sizeof a);
    memset(&b, 0, sizeof b);
    a.c = 'x'; a.i = 42;
    b.c = 'x'; b.i = 42;
    /* 由于已 memset 清零，填充字节确定，整体比较应相等 */
    assert(memcmp(&a, &b, sizeof a) == 0);

    /* 只比较确定成员：更稳健 */
    assert(memcmp(&a.c, &b.c, sizeof a.c) == 0);
    assert(memcmp(&a.i, &b.i, sizeof a.i) == 0);

    /* 改变一个成员 -> 不相等 */
    b.i = 43;
    assert(memcmp(&a.i, &b.i, sizeof a.i) != 0);
}

int main(void)
{
    test_prototype();
    test_equal();
    test_greater_less();
    test_binary_data();
    test_void_pointer();
    test_prefix_ordering();
    test_footnote271();

    printf("C99 7.21.4.1 memcmp: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「memcmp 原型要求 3 个实参」：实参个数不匹配，应报错
 * 期望：error: too few arguments to function 'memcmp' */
void neg_too_few_args(void)
{
    char a[4] = "abc", b[4] = "abc";
    memcmp(a, b);            /* 缺少 size_t n 实参 */
}

/* 违反约束「memcmp 原型要求 3 个实参」：实参过多，应报错
 * 期望：error: too many arguments to function 'memcmp' */
void neg_too_many_args(void)
{
    char a[4] = "abc", b[4] = "abc";
    memcmp(a, b, 3, 4);      /* 多出一个实参 */
}

/* 违反约束「第 3 个实参必须为整型（size_t）」：传入结构体，应报错
 * 期望：error: incompatible type for argument 3 of 'memcmp' */
struct S { int x; };
void neg_bad_third_arg(void)
{
    char a[4] = "abc", b[4] = "abc";
    struct S s;
    memcmp(a, b, s);         /* 第 3 实参不是整型 */
}

/* 违反约束「第 3 个实参必须为整型」：传入浮点，应报错
 * 期望：error: incompatible type for argument 3 of 'memcmp' */
void neg_float_third_arg(void)
{
    char a[4] = "abc", b[4] = "abc";
    memcmp(a, b, 3.0);       /* 浮点不能隐式转换为 size_t 参数 */
}

/* 违反约束「memcmp 返回 int，不可作为赋值目标」：对函数调用结果赋值，应报错
 * 期望：error: lvalue required as left operand of assignment */
void neg_assign_to_call(void)
{
    char a[4] = "abc", b[4] = "abc";
    memcmp(a, b, 3) = 0;     /* 函数调用结果不是左值 */
}

/* 违反约束「memcmp 返回 int，不可取地址」：对函数调用结果取地址，应报错
 * 期望：error: lvalue required as unary '&' operand */
void neg_address_of_call(void)
{
    char a[4] = "abc", b[4] = "abc";
    int *p = &memcmp(a, b, 3);   /* 非左值不能取地址 */
    (void)p;
}

/* 违反约束「memcmp 返回 int，不可自增」：对函数调用结果 ++，应报错
 * 期望：error: lvalue required as increment operand */
void neg_increment_call(void)
{
    char a[4] = "abc", b[4] = "abc";
    memcmp(a, b, 3)++;       /* 非左值不能自增 */
}

#endif /* 负向测试结束 */