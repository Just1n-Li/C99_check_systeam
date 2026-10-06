/*
 * 测试 C99 7.21.2.2 —— memmove 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型：void *memmove(void *s1, const void *s2, size_t n);
 *   [2] 语义：把 s2 指向对象的 n 个字符拷贝到 s1 指向的对象；
 *       拷贝行为如同先复制到不与 s1/s2 重叠的临时数组，再复制到 s1。
 *       因此即使源与目标区域重叠，结果也必须是正确的。
 *   [3] 返回值：返回 s1 的值。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* 用于验证 [1] 原型：把函数指针赋成正确类型，若原型不符则编译报错 */
static void *(*fp_memmove)(void *, const void *, size_t) = memmove;

int main(void)
{
    /* ---------- [1] 原型检查 ---------- */
    /* 参数类型为 void*、const void*、size_t，返回 void* */
    assert(fp_memmove == memmove);

    /* ---------- [2] 基本拷贝：不重叠区域 ---------- */
    {
        char src[16] = "hello, world";
        char dst[16];
        memset(dst, 0, sizeof dst);

        void *ret = memmove(dst, src, 13); /* 含结尾 '\0' 共 13 字节 */
        assert(strcmp(dst, "hello, world") == 0);
        /* [3] 返回值应为 s1（即 dst） */
        assert(ret == (void *)dst);
    }

    /* ---------- [2] 拷贝 n 个字符（不要求是字符串，可含 '\0'） ---------- */
    {
        unsigned char src[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
        unsigned char dst[8];
        memset(dst, 0xFF, sizeof dst);

        void *ret = memmove(dst, src, 8);
        assert(memcmp(dst, src, 8) == 0);
        assert(ret == (void *)dst);
    }

    /* ---------- [2] n == 0：不拷贝任何字符，仍返回 s1 ---------- */
    {
        char src[4] = "abc";
        char dst[4] = "xyz";
        void *ret = memmove(dst, src, 0);
        assert(strcmp(dst, "xyz") == 0); /* 目标未被修改 */
        assert(ret == (void *)dst);
    }

    /* ---------- [2] 重叠：目标在源之前（dst < src） ---------- */
    {
        char buf[16] = "0123456789";
        /* 把 buf[2..] 拷贝到 buf[0..]，即左移 2 字节 */
        void *ret = memmove(buf, buf + 2, 8);
        assert(memcmp(buf, "23456789", 8) == 0);
        assert(ret == (void *)buf);
    }

    /* ---------- [2] 重叠：目标在源之后（dst > src） ---------- */
    {
        char buf[16] = "0123456789";
        /* 把 buf[0..] 拷贝到 buf[2..]，即右移 2 字节 */
        void *ret = memmove(buf + 2, buf, 8);
        assert(memcmp(buf, "0101234567", 10) == 0);
        assert(ret == (void *)(buf + 2));
    }

    /* ---------- [2] 完全重叠：s1 == s2，内容不变 ---------- */
    {
        char buf[8] = "abcdefg";
        void *ret = memmove(buf, buf, 8);
        assert(strcmp(buf, "abcdefg") == 0);
        assert(ret == (void *)buf);
    }

    /* ---------- [2] 部分重叠：单字节步进，验证“如同临时数组”语义 ---------- */
    {
        char buf[8] = "ABCDEFG";
        /* 逐字节右移一位：期望 "AABCDEF" */
        memmove(buf + 1, buf, 6);
        assert(memcmp(buf, "AABCDEF", 7) == 0);
    }

    /* ---------- [2] 通过 void* 使用，验证可接受任意对象指针 ---------- */
    {
        int src[4] = { 10, 20, 30, 40 };
        int dst[4] = { 0, 0, 0, 0 };
        void *ret = memmove(dst, src, sizeof src);
        assert(memcmp(dst, src, sizeof src) == 0);
        assert(ret == (void *)dst);
    }

    /* ---------- [2] 重叠的 int 数组（按字节拷贝，重叠安全） ---------- */
    {
        int a[6] = { 1, 2, 3, 4, 5, 6 };
        /* 把 a[1..4] 拷贝到 a[0..3] */
        memmove(a, a + 1, 4 * sizeof(int));
        assert(a[0] == 2 && a[1] == 3 && a[2] == 4 && a[3] == 5);
        assert(a[4] == 5 && a[5] == 6);
    }

    /* ---------- [3] 返回值恒等于 s1（多种 s1 取值） ---------- */
    {
        char buf[8] = "1234567";
        assert(memmove(buf, buf + 1, 3) == (void *)buf);
        assert(memmove(buf + 1, buf, 3) == (void *)(buf + 1));
        assert(memmove(buf, buf, 0) == (void *)buf);
    }

    printf("All positive tests for C99 7.21.2.2 (memmove) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [1]：memmove 的第一个参数类型为 void*，
 * 不能把 const 对象的地址（const char*）直接传入而不丢弃 const。
 * 期望：gcc -std=c99 报 "discards 'const' qualifier" 警告/错误（-Werror 下为错误）。 */
void neg_const_first_arg(void)
{
    const char src[8] = "abc";
    char dst[8];
    memmove(src, dst, 8); /* 错误：s1 指向 const 对象 */
}

/* 违反约束 [1]：第二个参数类型为 const void*，但传入的是指向
 * 非 const 的指针本身没问题；此处演示参数个数错误。
 * 期望：编译报错 "too few arguments to function 'memmove'"。 */
void neg_too_few_args(void)
{
    char a[4], b[4];
    memmove(a, b); /* 错误：缺少第三个参数 n */
}

/* 违反约束 [1]：参数个数过多。
 * 期望：编译报错 "too many arguments to function 'memmove'"。 */
void neg_too_many_args(void)
{
    char a[4], b[4];
    memmove(a, b, 4, 0); /* 错误：多了一个参数 */
}

/* 违反约束 [1]：第三个参数类型为 size_t，传入指针类型不兼容。
 * 期望：编译报错（指针不能隐式转换为整型 size_t）。 */
void neg_bad_third_arg(void)
{
    char a[4], b[4];
    memmove(a, b, b); /* 错误：n 应为 size_t，传入了 char* */
}

/* 违反约束 [1]：把返回值赋给不兼容的函数指针类型。
 * 期望：编译报错（类型不兼容）。 */
void neg_bad_return_use(void)
{
    int (*p)(void) = memmove; /* 错误：类型不兼容 */
    (void)p;
}

#endif /* 负向测试结束 */