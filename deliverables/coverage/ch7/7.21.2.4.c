/*
 * 测试 C99 7.21.2.4 —— strncpy 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明（Synopsis）
 *   [2] 最多复制 n 个字符；遇到 '\0' 后不再复制；重叠为 UB（不作负向测试）
 *   [3] s2 短于 n 时用 '\0' 填充至 n 个字符
 *   [4] 返回 s1
 *   脚注 269：s2 前 n 个字符无 '\0' 时结果不以 '\0' 结尾
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型：char *strncpy(char * restrict s1, const char * restrict s2, size_t n);
 *     验证返回类型为 char*，参数类型匹配。 */
static void test_synopsis(void)
{
    char dst[16];
    const char *src = "abc";
    char *ret = strncpy(dst, src, sizeof dst);
    /* [4] 返回值应等于 s1 */
    assert(ret == dst);
}

/* [2] 最多复制 n 个字符；遇到 '\0' 后不再复制（其后内容不复制）。 */
static void test_copy_at_most_n(void)
{
    char dst[16];
    /* s2 为 "hello"，n = 3：只复制前 3 个字符，不追加 '\0'（因为 n 已用尽） */
    memset(dst, 'X', sizeof dst);
    strncpy(dst, "hello", 3);
    assert(dst[0] == 'h');
    assert(dst[1] == 'e');
    assert(dst[2] == 'l');
    /* 第 4 个字节未被写入，仍是填充值 'X' */
    assert(dst[3] == 'X');
}

/* [2] 遇到 '\0' 后不再复制：s2 中 '\0' 之后的字符不应被复制。 */
static void test_stop_at_null(void)
{
    char dst[16];
    /* 源数组：'a','b','\0','c','d' —— '\0' 之后的 'c','d' 不应被复制 */
    const char src[5] = { 'a', 'b', '\0', 'c', 'd' };
    memset(dst, 'Z', sizeof dst);
    strncpy(dst, src, 5);
    /* [3] s2 短于 n（作为字符串长度 2 < 5），用 '\0' 填充至 5 个字符 */
    assert(dst[0] == 'a');
    assert(dst[1] == 'b');
    assert(dst[2] == '\0');
    assert(dst[3] == '\0');
    assert(dst[4] == '\0');
    /* 第 6 个字节未被写入 */
    assert(dst[5] == 'Z');
}

/* [3] s2 是短于 n 的字符串：用 '\0' 填充直到共写入 n 个字符。 */
static void test_null_padding(void)
{
    char dst[16];
    memset(dst, 'Q', sizeof dst);
    strncpy(dst, "hi", 6);
    assert(dst[0] == 'h');
    assert(dst[1] == 'i');
    assert(dst[2] == '\0');
    assert(dst[3] == '\0');
    assert(dst[4] == '\0');
    assert(dst[5] == '\0');
    /* 第 7 个字节未被写入 */
    assert(dst[6] == 'Q');
}

/* [3] n 恰好等于字符串长度（含结尾 '\0'）：应复制含 '\0' 的 n 个字符。 */
static void test_exact_length(void)
{
    char dst[16];
    memset(dst, 'W', sizeof dst);
    strncpy(dst, "abc", 4); /* "abc" 长度 3，加 '\0' 共 4 */
    assert(dst[0] == 'a');
    assert(dst[1] == 'b');
    assert(dst[2] == 'c');
    assert(dst[3] == '\0');
    assert(dst[4] == 'W');
}

/* [4] 返回值等于 s1（用不同缓冲区再验证一次）。 */
static void test_return_value(void)
{
    char dst[8];
    char *ret = strncpy(dst, "xyz", 8);
    assert(ret == dst);
    assert(strcmp(ret, "xyz") == 0);
}

/* 脚注 269：s2 前 n 个字符无 '\0' 时，结果不以 '\0' 结尾。 */
static void test_footnote_269(void)
{
    char dst[4];
    memset(dst, '#', sizeof dst);
    /* 源 "abcdef" 前 4 个字符无 '\0'，n = 4 */
    strncpy(dst, "abcdef", 4);
    assert(dst[0] == 'a');
    assert(dst[1] == 'b');
    assert(dst[2] == 'c');
    assert(dst[3] == 'd');
    /* 结果不是以 '\0' 结尾的字符串 */
    assert(memchr(dst, '\0', 4) == NULL);
}

/* [2] n = 0：不复制任何字符。 */
static void test_zero_n(void)
{
    char dst[4] = { 'A', 'B', 'C', '\0' };
    char *ret = strncpy(dst, "zzz", 0);
    assert(ret == dst);
    assert(dst[0] == 'A');
    assert(dst[1] == 'B');
    assert(dst[2] == 'C');
    assert(dst[3] == '\0');
}

int main(void)
{
    test_synopsis();
    test_copy_at_most_n();
    test_stop_at_null();
    test_null_padding();
    test_exact_length();
    test_return_value();
    test_footnote_269();
    test_zero_n();

    printf("C99 7.21.2.4 strncpy: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strncpy 的第一个参数类型为 char *（可修改的左值对象指针）」：
 * 传入字符串字面量（const char[] 退化为 const char *），
 * 且目标不可写。gcc -std=c99 应报错（丢弃 const 限定符 / 只读目标）。 */
#include <string.h>
void bad_const_target(void)
{
    strncpy("literal", "src", 3);
}

/* 违反约束「strncpy 的第二个参数类型为 const char *」：
 * 传入 int * 而非字符指针，参数类型不兼容。应编译报错。 */
#include <string.h>
void bad_wrong_arg_type(void)
{
    char dst[8];
    int nums[4] = { 1, 2, 3, 4 };
    strncpy(dst, nums, 4); /* int* 不能转换为 const char* */
}

/* 违反约束「strncpy 需要 3 个实参」：
 * 实参个数与原型不符。应编译报错。 */
#include <string.h>
void bad_too_few_args(void)
{
    char dst[8];
    strncpy(dst, "abc"); /* 缺少第三个参数 n */
}

/* 违反约束「strncpy 需要 3 个实参」：
 * 实参过多。应编译报错。 */
#include <string.h>
void bad_too_many_args(void)
{
    char dst[8];
    strncpy(dst, "abc", 3, 4); /* 多出第四个参数 */
}

/* 违反约束「strncpy 返回 char *，可赋值给 char *」：
 * 试图把返回值赋给不兼容的指针类型（如 int *），
 * 在 C99 中为约束违反（不兼容指针赋值）。应编译报错。 */
#include <string.h>
void bad_return_assign(void)
{
    char dst[8];
    int *p = strncpy(dst, "abc", 8); /* char* 赋给 int*，不兼容 */
    (void)p;
}

/* 违反约束「strncpy 的第三个参数类型为 size_t」：
 * 传入结构体类型，无法转换为 size_t。应编译报错。 */
#include <string.h>
struct S { int x; };
void bad_third_arg_type(void)
{
    char dst[8];
    struct S s;
    strncpy(dst, "abc", s); /* 结构体不能转换为 size_t */
}

#endif /* 负向测试结束 */