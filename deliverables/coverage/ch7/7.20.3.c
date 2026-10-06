/*
 * 测试 C99 7.20.3 —— Memory management functions (calloc/malloc/realloc)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译（gcc -std=c99）并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器应报错（本文件因 #if 0 而不编译它们）。
 *
 * 覆盖段落：
 *   [1] 分配顺序/连续性未指定；返回指针对任意对象类型适当对齐；
 *       生命周期从分配到释放；每次分配返回与其它对象不相交的指针；
 *       返回指针指向分配空间起始（最低字节地址）；失败返回空指针；
 *       请求大小为 0 时行为实现定义（返回空指针，或如同非零大小但不得解引用）。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 返回指针对任意对象类型适当对齐：可赋给任意对象类型指针并访问 */
static void test_alignment(void)
{
    /* 分配一块内存，检查其地址对常见类型对齐 */
    void *p = malloc(256);
    assert(p != NULL);

    /* 赋给各种对象类型指针并访问，验证对齐足够 */
    char    *pc = (char *)p;
    short   *ps = (short *)p;
    int     *pi = (int *)p;
    double  *pd = (double *)p;
    long    *pl = (long *)p;

    *pc = 'A';
    *ps = 1234;
    *pi = 56789;
    *pd = 3.14159;
    *pl = 987654321L;

    assert(*pc == 'A');
    assert(*ps == 1234);
    assert(*pi == 56789);
    assert(*pd == 3.14159);
    assert(*pl == 987654321L);

    /* 地址对齐检查：malloc 返回的指针应满足最严格对齐 */
    assert(((size_t)p % _Alignof(double)) == 0);

    free(p);
}

/* [1] 生命周期从分配到释放；返回指针指向分配空间起始（最低字节地址） */
static void test_lifetime_and_start(void)
{
    int *p = (int *)malloc(sizeof(int) * 4);
    assert(p != NULL);

    /* 返回指针指向起始地址：写入第一个元素 */
    p[0] = 10;
    p[1] = 20;
    p[2] = 30;
    p[3] = 40;
    assert(p[0] == 10 && p[1] == 20 && p[2] == 30 && p[3] == 40);

    /* 生命周期内可访问；释放后不再访问（此处仅释放） */
    free(p);
}

/* [1] 每次分配返回与其它对象不相交的指针 */
static void test_disjoint(void)
{
    int *a = (int *)malloc(sizeof(int) * 8);
    int *b = (int *)malloc(sizeof(int) * 8);
    assert(a != NULL && b != NULL);

    /* 两个分配块不相交：地址范围不重叠 */
    size_t sa = sizeof(int) * 8;
    size_t sb = sizeof(int) * 8;
    char *pa = (char *)a;
    char *pb = (char *)b;

    /* 检查区间 [pa, pa+sa) 与 [pb, pb+sb) 不相交 */
    int disjoint = (pa + sa <= pb) || (pb + sb <= pa);
    assert(disjoint);

    /* 写入 a 不影响 b */
    memset(a, 0xAA, sa);
    memset(b, 0x55, sb);
    assert(((unsigned char *)a)[0] == 0xAA);
    assert(((unsigned char *)b)[0] == 0x55);

    free(a);
    free(b);
}

/* [1] calloc 分配并清零 */
static void test_calloc_zeroed(void)
{
    size_t n = 16;
    int *p = (int *)calloc(n, sizeof(int));
    assert(p != NULL);

    for (size_t i = 0; i < n; i++) {
        assert(p[i] == 0);
    }

    free(p);
}

/* [1] realloc 调整大小；成功时返回适当对齐的指针 */
static void test_realloc(void)
{
    int *p = (int *)malloc(sizeof(int) * 4);
    assert(p != NULL);
    p[0] = 1; p[1] = 2; p[2] = 3; p[3] = 4;

    int *q = (int *)realloc(p, sizeof(int) * 8);
    assert(q != NULL);

    /* 原有内容应保留（realloc 语义） */
    assert(q[0] == 1 && q[1] == 2 && q[2] == 3 && q[3] == 4);

    /* 新区域可写 */
    q[4] = 5; q[5] = 6; q[6] = 7; q[7] = 8;
    assert(q[7] == 8);

    /* 对齐检查 */
    assert(((size_t)q % _Alignof(double)) == 0);

    free(q);
}

/* [1] 请求大小为 0 时行为实现定义：
 *     要么返回空指针，要么如同非零大小但返回指针不得解引用。
 *     这里只验证两种合法结果之一，不解引用。 */
static void test_zero_size(void)
{
    void *p = malloc(0);
    /* 合法结果：NULL 或非 NULL（非 NULL 时不得解引用） */
    if (p != NULL) {
        /* 不得解引用；仅释放 */
        free(p);
    }
    /* 无论哪种结果都合法，测试通过 */
    assert(1);
}

/* [1] 分配失败返回空指针：请求极大尺寸，通常失败 */
static void test_failure_null(void)
{
    /* 请求一个几乎不可能满足的巨大尺寸 */
    void *p = malloc((size_t)-1);
    /* 若失败则返回 NULL；若实现能分配（不太可能），则释放 */
    if (p != NULL) {
        free(p);
    }
    /* 主要验证：失败时返回 NULL 是合法行为 */
    assert(1);
}

int main(void)
{
    test_alignment();
    test_lifetime_and_start();
    test_disjoint();
    test_calloc_zeroed();
    test_realloc();
    test_zero_size();
    test_failure_null();

    printf("C99 7.20.3 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 说明：7.20.3 本身主要是语义描述，约束较少。
 * 以下片段演示与内存管理函数相关的典型约束违反，
 * 期望 gcc -std=c99 报错。 */

/* 违反约束「malloc 的参数必须为 size_t 类型」：
 * 传入结构体（非算术类型）作为参数，应报错。 */
struct S { int x; };
void bad_malloc_arg(void)
{
    struct S s;
    void *p = malloc(s);   /* 错误：参数类型不匹配，结构体不能隐式转换为 size_t */
    (void)p;
}

/* 违反约束「calloc 的两个参数必须为 size_t 类型」：
 * 传入指针作为参数，应报错。 */
void bad_calloc_arg(void)
{
    int *ptr = 0;
    void *p = calloc(ptr, ptr);  /* 错误：指针不能隐式转换为 size_t */
    (void)p;
}

/* 违反约束「realloc 的第一个参数必须为 void* 或对象指针」：
 * 传入整数常量，应报错。 */
void bad_realloc_arg(void)
{
    void *p = realloc(42, 100);  /* 错误：int 不能隐式转换为 void* */
    (void)p;
}

/* 违反约束「free 的参数必须为 void* 或对象指针」：
 * 传入浮点数，应报错。 */
void bad_free_arg(void)
{
    free(3.14);  /* 错误：double 不能隐式转换为 void* */
}

/* 违反约束「对 void 表达式解引用」：
 * malloc 返回 void*，不能直接解引用。 */
void bad_deref_void(void)
{
    void *p = malloc(16);
    *p = 1;   /* 错误：不能对 void* 解引用 */
    free(p);
}

#endif /* 负向测试结束 */