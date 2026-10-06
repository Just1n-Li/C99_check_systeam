/*
 * 测试条款：C99 7.21.2.1  The memcpy function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型：void *memcpy(void * restrict s1, const void * restrict s2, size_t n);
 *   [2] 语义：从 s2 指向的对象复制 n 个字符到 s1 指向的对象；
 *            若两对象重叠，行为未定义（UB，不作为负向测试）。
 *   [3] 返回值：返回 s1 的值。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* 用于验证 [1] 原型中 restrict 限定符与 const 限定符可被接受 */
static void test_prototype_and_restrict(void)
{
    char src[8] = "abcdefg";
    char dst[8] = {0};

    /* [1] 原型：第一个参数为 void * restrict，第二个为 const void * restrict */
    void *ret = memcpy(dst, src, 8);

    /* [3] 返回值应等于 s1（即 dst） */
    assert(ret == (void *)dst);
    assert(memcmp(dst, "abcdefg", 8) == 0);
}

/* [2] 复制 n 个字符：逐字节复制，包括嵌入的 '\0' */
static void test_copy_n_chars(void)
{
    unsigned char src[6] = { 0x01, 0x00, 0x02, 0x03, 0x00, 0x04 };
    unsigned char dst[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    memcpy(dst, src, 6);

    assert(dst[0] == 0x01);
    assert(dst[1] == 0x00);   /* 嵌入的 0 也被复制 */
    assert(dst[2] == 0x02);
    assert(dst[3] == 0x03);
    assert(dst[4] == 0x00);
    assert(dst[5] == 0x04);
}

/* [2] n == 0 时不复制任何字符，目标对象保持不变 */
static void test_zero_length(void)
{
    char dst[4] = "XYZ";
    char src[4] = "abc";

    void *ret = memcpy(dst, src, 0);

    assert(ret == (void *)dst);   /* [3] 仍返回 s1 */
    assert(dst[0] == 'X' && dst[1] == 'Y' && dst[2] == 'Z' && dst[3] == '\0');
}

/* [2] 复制任意对象类型（此处为结构体），按字节复制 */
struct Point { int x; int y; };

static void test_copy_struct(void)
{
    struct Point a = { 3, 4 };
    struct Point b = { 0, 0 };

    void *ret = memcpy(&b, &a, sizeof(struct Point));

    assert(ret == (void *)&b);    /* [3] */
    assert(b.x == 3 && b.y == 4);
}

/* [2] 复制到数组中间位置（非重叠），验证偏移正确 */
static void test_partial_copy(void)
{
    char buf[16] = "0123456789ABCDE";
    char src[4]  = "xyz";

    memcpy(buf + 5, src, 3);      /* 只复制 3 个字符 */

    assert(buf[4] == '4');
    assert(buf[5] == 'x');
    assert(buf[6] == 'y');
    assert(buf[7] == 'z');
    assert(buf[8] == '8');        /* 未被覆盖 */
}

/* [3] 返回值可被直接使用（链式/表达式上下文） */
static void test_return_value_usable(void)
{
    char src[4] = "abc";
    char dst[4] = {0};

    /* 返回值是 void*，可赋给 void* 变量并比较 */
    void *p = memcpy(dst, src, 4);
    assert(p == (void *)dst);
    assert(strcmp(dst, "abc") == 0);
}

int main(void)
{
    test_prototype_and_restrict();
    test_copy_n_chars();
    test_zero_length();
    test_copy_struct();
    test_partial_copy();
    test_return_value_usable();

    printf("C99 7.21.2.1 memcpy: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 原型要求第一个参数为 void *（可写对象指针）」：
 * 传入指向 const 对象的指针作为 s1，丢弃 const 限定，
 * gcc -std=c99 应报错（discards 'const' qualifier / assignment ... discards qualifiers）。
 * 注意：这里 s1 指向 const 数据，属于约束违反，而非 UB。 */
void neg_const_dest(void)
{
    const char src[4] = "abc";
    const char dst[4] = "xyz";
    memcpy(dst, src, 4);   /* 错误：dst 为 const char[4]，不能作为 void* 实参 */
}

/* 违反约束「[1] 原型要求第二个参数为 const void *」：
 * 传入非指针类型（int）作为 s2，实参与形参类型不兼容，
 * gcc -std=c99 应报错（incompatible type for argument 2 / passing argument ... makes pointer from integer）。 */
void neg_non_pointer_src(void)
{
    char dst[4];
    memcpy(dst, 42, 4);    /* 错误：42 是 int，不是指针 */
}

/* 违反约束「[1] 原型要求第三个参数为 size_t」：
 * 传入结构体类型作为 n，实参与形参类型不兼容，
 * gcc -std=c99 应报错（incompatible type for argument 3）。 */
struct NotSize { int a; };
void neg_bad_size_type(void)
{
    char dst[4], src[4];
    struct NotSize n;
    memcpy(dst, src, n);   /* 错误：n 是结构体，不是 size_t */
}

/* 违反约束「[1] 调用函数前必须有可见声明」：
 * 在未包含 <string.h> 且无自身声明的情况下调用 memcpy，
 * 在 C99 中隐式函数声明已被删除，gcc -std=c99 应报错
 * （implicit declaration of function 'memcpy'）。 */
void neg_no_declaration(void)
{
    char dst[4], src[4];
    memcpy(dst, src, 4);   /* 错误：无可见原型 */
}

/* 违反约束「[1] 形参个数必须匹配」：
 * 少传一个实参，gcc -std=c99 应报错（too few arguments to function 'memcpy'）。 */
void neg_too_few_args(void)
{
    char dst[4], src[4];
    memcpy(dst, src);      /* 错误：缺少第三个实参 n */
}

/* 违反约束「[1] 形参个数必须匹配」：
 * 多传一个实参，gcc -std=c99 应报错（too many arguments to function 'memcpy'）。 */
void neg_too_many_args(void)
{
    char dst[4], src[4];
    memcpy(dst, src, 4, 0); /* 错误：多出第四个实参 */
}

#endif /* 负向测试结束 */