/*
 * 测试 C99 7.20.3.4 —— realloc 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明 void *realloc(void *ptr, size_t size);
 *   [2] 释放旧对象、返回新对象；内容保留到 min(new, old)；超出旧大小的字节值不确定。
 *   [3] ptr 为 NULL 时等价于 malloc；ptr 非 NULL 且非 calloc/malloc/realloc 返回值或已释放 → UB（UB 不作为负向测试）。
 *   [4] 返回新对象指针（可能与旧指针相同），失败返回 NULL，且旧对象不被释放、值不变。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与 void *(void *, size_t) 兼容 */
static void test_prototype(void)
{
    void *(*fp)(void *, size_t) = realloc;   /* [1] 原型匹配 */
    assert(fp != NULL);
    printf("[1] prototype ok\n");
}

/* [2] 内容保留：新对象内容与旧对象相同，直到 min(new, old) */
static void test_content_preserved(void)
{
    const size_t oldsz = 16;
    const size_t newsz = 64;
    unsigned char *p = (unsigned char *)malloc(oldsz);
    assert(p != NULL);

    for (size_t i = 0; i < oldsz; ++i)
        p[i] = (unsigned char)(i + 1);

    unsigned char *q = (unsigned char *)realloc(p, newsz);
    assert(q != NULL);                       /* [4] 成功返回非 NULL */

    /* [2] 前 oldsz 字节内容必须与旧对象一致 */
    for (size_t i = 0; i < oldsz; ++i)
        assert(q[i] == (unsigned char)(i + 1));

    /* [2] 超出旧大小的字节值不确定 —— 只验证可访问，不验证具体值 */
    for (size_t i = oldsz; i < newsz; ++i)
        q[i] = (unsigned char)i;             /* 可写即可 */

    free(q);
    printf("[2] content preserved up to min(new, old)\n");
}

/* [2] 缩小：内容保留到 min(new, old) = new */
static void test_shrink(void)
{
    const size_t oldsz = 32;
    const size_t newsz = 8;
    unsigned char *p = (unsigned char *)malloc(oldsz);
    assert(p != NULL);
    for (size_t i = 0; i < oldsz; ++i)
        p[i] = (unsigned char)(0xA0 + i);

    unsigned char *q = (unsigned char *)realloc(p, newsz);
    assert(q != NULL);
    for (size_t i = 0; i < newsz; ++i)
        assert(q[i] == (unsigned char)(0xA0 + i));

    free(q);
    printf("[2] shrink preserves first min(new, old) bytes\n");
}

/* [3] ptr == NULL 时等价于 malloc(size) */
static void test_null_like_malloc(void)
{
    size_t n = 100;
    unsigned char *p = (unsigned char *)realloc(NULL, n);
    assert(p != NULL);                       /* [3] 等价 malloc，应成功 */
    for (size_t i = 0; i < n; ++i)
        p[i] = (unsigned char)i;             /* 可写 */
    free(p);
    printf("[3] realloc(NULL, n) behaves like malloc(n)\n");
}

/* [4] 返回指针可能与旧指针相同（不强制，但允许）—— 只验证返回值可用 */
static void test_return_value_usable(void)
{
    int *p = (int *)malloc(4 * sizeof(int));
    assert(p != NULL);
    p[0] = 1; p[1] = 2; p[2] = 3; p[3] = 4;

    int *q = (int *)realloc(p, 8 * sizeof(int));
    assert(q != NULL);
    assert(q[0] == 1 && q[1] == 2 && q[2] == 3 && q[3] == 4);
    free(q);
    printf("[4] return value points to usable new object\n");
}

/* [4] 分配失败：返回 NULL，旧对象不被释放、值不变 */
static void test_failure_keeps_old(void)
{
    size_t n = 16;
    unsigned char *p = (unsigned char *)malloc(n);
    assert(p != NULL);
    for (size_t i = 0; i < n; ++i)
        p[i] = (unsigned char)(i ^ 0x5A);

    /* 请求一个几乎不可能满足的巨大尺寸，期望失败 */
    void *r = realloc(p, (size_t)-1);
    if (r == NULL) {
        /* [4] 失败：旧对象未被释放，值不变 */
        for (size_t i = 0; i < n; ++i)
            assert(p[i] == (unsigned char)(i ^ 0x5A));
        free(p);
        printf("[4] failure returns NULL, old object unchanged\n");
    } else {
        /* 极端环境下若成功，则按成功路径处理 */
        free(r);
        printf("[4] huge realloc unexpectedly succeeded (env-dependent)\n");
    }
}

/* [2][3] 多次 realloc 链式增长，内容始终保留 */
static void test_chain_growth(void)
{
    size_t n = 4;
    unsigned char *p = (unsigned char *)malloc(n);
    assert(p != NULL);
    for (size_t i = 0; i < n; ++i)
        p[i] = (unsigned char)(i + 1);

    for (int step = 0; step < 5; ++step) {
        size_t oldn = n;
        n *= 2;
        unsigned char *q = (unsigned char *)realloc(p, n);
        assert(q != NULL);
        for (size_t i = 0; i < oldn; ++i)
            assert(q[i] == (unsigned char)(i + 1));   /* [2] 内容保留 */
        p = q;
    }
    free(p);
    printf("[2][3] chained realloc preserves contents\n");
}

int main(void)
{
    test_prototype();
    test_content_preserved();
    test_shrink();
    test_null_like_malloc();
    test_return_value_usable();
    test_failure_keeps_old();
    test_chain_growth();
    printf("ALL POSITIVE TESTS PASSED\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「realloc 原型为 void *realloc(void *, size_t)」：
 * 参数个数/类型不匹配，gcc -std=c99 应报错（too few / incompatible argument）。 */
void bad_call_too_few_args(void)
{
    void *p = realloc();          /* 缺少参数：应报错 */
    (void)p;
}

/* 违反约束「第一个参数应为 void *（对象指针）」：
 * 传入 double 值，无法隐式转换为 void *，应报错。 */
void bad_call_wrong_first_arg(void)
{
    double d = 1.0;
    void *p = realloc(d, 16);     /* double 不能转 void *：应报错 */
    (void)p;
}

/* 违反约束「第二个参数应为 size_t（整数类型）」：
 * 传入结构体，无法转换为 size_t，应报错。 */
struct S { int x; };
void bad_call_wrong_second_arg(void)
{
    struct S s;
    void *p = realloc(NULL, s);   /* 结构体不能转 size_t：应报错 */
    (void)p;
}

/* 违反约束「realloc 返回 void *，不能直接赋给不兼容的指针类型而不转换」：
 * 在 C99 中 void* 可隐式转对象指针，故此处改为验证「返回值不可解引用为函数指针」等
 * 更明确的约束：把返回值当函数调用，应报错。 */
void bad_call_return_as_function(void)
{
    realloc(NULL, 16)();          /* 对 void* 调用：应报错 */
}

/* 违反约束「realloc 声明于 <stdlib.h>」：
 * 若未包含 <stdlib.h>，隐式声明与 C99 不符（C99 取消隐式函数声明），应报错。 */
void bad_no_prototype(void)
{
    /* 假设此处未包含 <stdlib.h>，realloc 未声明即调用：
     * C99 下应报错（implicit declaration of function 'realloc'）。 */
    void *p = realloc(NULL, 16);
    (void)p;
}

#endif /* 负向测试结束 */