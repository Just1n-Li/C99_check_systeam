/*
 * 测试 C99 7.21.1 —— String function conventions（字符串函数约定）
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，gcc -std=c99 应报错。
 *
 * 覆盖段落：
 *   [1] <string.h> 声明 size_t 类型与 NULL 宏；char*/void* 指向数组首字符。
 *   [2] n 可以为 0；此时定位函数找不到、比较函数返回 0、拷贝函数拷贝 0 个字符；
 *       指针参数仍须有效（7.1.4）。
 *   [3] 每个字符按 unsigned char 解释（所有对象表示均有效且值不同）。
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <string.h> 声明 size_t 类型并定义 NULL 宏 */
static void test_header_declarations(void)
{
    size_t sz = sizeof(char);          /* size_t 可用 */
    char *p = NULL;                    /* NULL 宏可用 */
    assert(sz == 1);
    assert(p == NULL);
}

/* [1] char* / void* 参数指向数组首（最低地址）字符 */
static void test_pointer_to_initial_char(void)
{
    char buf[8] = "hello";
    /* memchr 返回指向首次出现字符的指针，即数组内地址 */
    void *r = memchr(buf, 'h', sizeof buf);
    assert(r == (void *)buf);          /* 首字符地址 */
    /* strchr 同理 */
    char *s = strchr(buf, 'h');
    assert(s == buf);
}

/* [2] n == 0 时：定位函数找不到（返回 NULL） */
static void test_zero_length_locate(void)
{
    char buf[8] = "hello";
    /* memchr 长度 0：找不到任何字符 */
    assert(memchr(buf, 'h', 0) == NULL);
    /* strchr 对空串查找非 '\0' 字符返回 NULL */
    assert(strchr("", 'x') == NULL);
    /* strstr 空串中查找非空子串返回 NULL */
    assert(strstr("", "x") == NULL);
}

/* [2] n == 0 时：比较函数返回 0 */
static void test_zero_length_compare(void)
{
    char a[4] = "abc";
    char b[4] = "xyz";
    /* memcmp 长度 0：返回 0 */
    assert(memcmp(a, b, 0) == 0);
    /* strncmp 长度 0：返回 0 */
    assert(strncmp(a, b, 0) == 0);
}

/* [2] n == 0 时：拷贝函数拷贝 0 个字符（目标不变） */
static void test_zero_length_copy(void)
{
    char dst[8] = "AAAAAAA";
    char src[8] = "BBBBBBB";
    /* memcpy 长度 0：不拷贝任何字符 */
    memcpy(dst, src, 0);
    assert(dst[0] == 'A');
    /* memmove 长度 0 */
    memmove(dst, src, 0);
    assert(dst[0] == 'A');
    /* strncpy 长度 0：不拷贝 */
    strncpy(dst, src, 0);
    assert(dst[0] == 'A');
}

/* [2] n == 0 时指针参数仍须有效（7.1.4）：指向有效对象 */
static void test_zero_length_valid_pointers(void)
{
    char a[4] = "abc";
    char b[4] = "abc";
    /* 指针均有效，长度 0 调用合法 */
    assert(memcmp(a, b, 0) == 0);
    memcpy(a, b, 0);
    assert(a[0] == 'a');
}

/* [3] 每个字符按 unsigned char 解释：高位字节比较 */
static void test_unsigned_char_interpretation(void)
{
    /* 0xFF 作为 unsigned char 是 255，大于 0x01 */
    unsigned char hi = 0xFF;
    unsigned char lo = 0x01;
    char a[1];
    char b[1];
    a[0] = (char)hi;
    b[0] = (char)lo;
    /* 若按 signed char 解释，0xFF 为 -1，会小于 1；
       按 unsigned char 解释，0xFF(255) > 0x01(1)，memcmp 返回正数 */
    int r = memcmp(a, b, 1);
    assert(r > 0);

    /* 所有对象表示均有效且值不同：0x00..0xFF 两两不同 */
    unsigned char seen[256];
    int i;
    for (i = 0; i < 256; i++) seen[i] = 0;
    for (i = 0; i < 256; i++) {
        unsigned char c = (unsigned char)i;
        assert(seen[c] == 0);          /* 每个值唯一 */
        seen[c] = 1;
    }
}

/* [3] 字符按 unsigned char 解释：strcmp 对高位字符 */
static void test_strcmp_unsigned(void)
{
    char s1[2];
    char s2[2];
    s1[0] = (char)0xFF; s1[1] = '\0';
    s2[0] = (char)0x01; s2[1] = '\0';
    /* 按 unsigned char：0xFF > 0x01，strcmp 返回正 */
    assert(strcmp(s1, s2) > 0);
}

int main(void)
{
    test_header_declarations();
    test_pointer_to_initial_char();
    test_zero_length_locate();
    test_zero_length_compare();
    test_zero_length_copy();
    test_zero_length_valid_pointers();
    test_unsigned_char_interpretation();
    test_strcmp_unsigned();

    printf("C99 7.21.1 string function conventions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「<string.h> 中函数参数类型」：memcpy 第一参数须为 void*，
   传入整数常量应报错（参数类型不兼容）。
   期望：gcc -std=c99 报 incompatible type / passing argument 错误。 */
void bad_memcpy_arg(void)
{
    memcpy(1, "x", 1);
}

/* 违反约束「size_t 参数」：memchr 第三参数须为 size_t（整数类型），
   传入结构体应报错。
   期望：gcc -std=c99 报 incompatible type 错误。 */
struct S { int x; };
void bad_memchr_arg(void)
{
    char buf[4] = "abc";
    struct S s;
    memchr(buf, 'a', s);
}

/* 违反约束「函数返回类型」：strlen 返回 size_t，不能赋给结构体。
   期望：gcc -std=c99 报 incompatible type 错误。 */
void bad_strlen_assign(void)
{
    struct S s;
    s = strlen("abc");
}

/* 违反约束「指针参数须为指针类型」：strcpy 第一参数须为 char*，
   传入 double 应报错。
   期望：gcc -std=c99 报 incompatible type 错误。 */
void bad_strcpy_arg(void)
{
    double d = 0.0;
    strcpy(d, "x");
}

/* 违反约束「const 限定传播」：strchr 第一参数为 const char*，
   返回 char*，但把字符串字面量（const）传给非 const 指针参数
   在 C99 中为约束违反（丢弃限定符）。
   期望：gcc -std=c99 报 discards qualifiers 错误。 */
void bad_discard_const(void)
{
    const char *cs = "abc";
    char *p = cs;          /* 丢弃 const 限定符 */
    (void)p;
}

#endif