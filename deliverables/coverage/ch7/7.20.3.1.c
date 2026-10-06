/*
 * 测试目标：C99 7.20.3.1  calloc 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：void *calloc(size_t nmemb, size_t size);  需要 <stdlib.h>
 *   [2] 分配 nmemb 个对象、每个 size 字节的数组空间，并初始化为全 0 位
 *   [3] 返回空指针或指向所分配空间的指针
 *   脚注 261：全 0 位不必等于浮点 0 或空指针常量的表示
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：calloc 的返回类型必须是 void *，参数为两个 size_t。
 *     通过把函数指针赋给精确匹配的类型来静态验证原型。 */
static void test_prototype(void)
{
    void *(*fp)(size_t, size_t) = calloc;   /* [1] 原型匹配 */
    assert(fp != NULL);
}

/* [2] 分配 nmemb 个对象、每个 size 字节，且初始化为全 0 位 */
static void test_allocation_and_zero_init(void)
{
    size_t nmemb = 10;
    size_t size  = sizeof(int);
    unsigned char *p;
    size_t i;

    int *arr = (int *)calloc(nmemb, size);   /* [2] 分配数组空间 */
    assert(arr != NULL);                     /* [3] 非空指针 */

    /* [2] 空间被初始化为全 0 位：逐字节检查 */
    p = (unsigned char *)arr;
    for (i = 0; i < nmemb * size; ++i) {
        assert(p[i] == 0);
    }

    /* 全 0 位对整数类型意味着值为 0 */
    for (i = 0; i < nmemb; ++i) {
        assert(arr[i] == 0);
    }

    free(arr);
}

/* [2] 分配的元素个数与元素大小相乘决定总字节数 */
static void test_total_bytes(void)
{
    size_t nmemb = 8;
    size_t size  = 16;
    unsigned char *p = (unsigned char *)calloc(nmemb, size);
    size_t i;

    assert(p != NULL);                       /* [3] */
    for (i = 0; i < nmemb * size; ++i) {
        assert(p[i] == 0);                   /* [2] 全 0 位 */
    }
    free(p);
}

/* [2] 分配结构体数组，验证每个成员都被置 0 */
struct Point { int x; int y; double z; };

static void test_struct_array(void)
{
    size_t n = 5;
    struct Point *pts = (struct Point *)calloc(n, sizeof(struct Point));
    size_t i;

    assert(pts != NULL);                     /* [3] */
    for (i = 0; i < n; ++i) {
        assert(pts[i].x == 0);
        assert(pts[i].y == 0);
        assert(pts[i].z == 0.0);             /* 全 0 位对 IEEE 浮点即 0.0 */
    }
    free(pts);
}

/* [2] 脚注 261：全 0 位不必等于浮点 0 或空指针常量的表示。
 *      这里只验证「全 0 位」这一事实，而不假设它与浮点 0 的位模式相同。 */
static void test_footnote_261(void)
{
    unsigned char *p = (unsigned char *)calloc(1, sizeof(double));
    double d;
    size_t i;

    assert(p != NULL);
    for (i = 0; i < sizeof(double); ++i) {
        assert(p[i] == 0);                   /* 全 0 位 */
    }
    memcpy(&d, p, sizeof(double));
    /* 在常见实现上全 0 位即 0.0，但标准只保证「全 0 位」，
     * 因此这里只断言位模式，不强制 d == 0.0 的表示假设。 */
    (void)d;
    free(p);
}

/* [3] 返回空指针或指向所分配空间的指针。
 *      请求 0 个元素时，实现可返回空指针或可解引用的指针（此处只检查二者之一）。 */
static void test_zero_elements(void)
{
    void *p = calloc(0, sizeof(int));
    /* [3] 允许返回 NULL，也允许返回非 NULL 的可释放指针 */
    if (p != NULL) {
        free(p);
    }
    assert(1); /* 两种结果都符合 [3] */
}

/* [3] 返回的指针可用于读写所分配的空间 */
static void test_writable(void)
{
    int *a = (int *)calloc(4, sizeof(int));
    assert(a != NULL);                       /* [3] */
    a[0] = 42;
    a[3] = -7;
    assert(a[0] == 42);
    assert(a[3] == -7);
    free(a);
}

int main(void)
{
    test_prototype();
    test_allocation_and_zero_init();
    test_total_bytes();
    test_struct_array();
    test_footnote_261();
    test_zero_elements();
    test_writable();

    printf("C99 7.20.3.1 calloc: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「calloc 的返回类型为 void *，参数为两个 size_t」：
 * 用不兼容的函数指针类型接收 calloc，gcc -std=c99 应报错
 * （incompatible pointer type / initialization from incompatible pointer type）。 */
int (*bad_fp)(int, int) = calloc;

/* 违反约束「calloc 需要 <stdlib.h> 中的原型」：
 * 若未包含 <stdlib.h>，在 C99 中调用未声明函数是约束违反，
 * 编译器应报错（implicit declaration of function 'calloc'）。
 * 下面通过显式取消声明来模拟： */
#undef calloc
void *bad_call = calloc(1, 1);   /* 未声明标识符，应报错 */

/* 违反约束「参数个数必须为 2」：
 * 少传参数，gcc -std=c99 应报错（too few arguments to function 'calloc'）。 */
void *bad_argc = calloc(1);

/* 违反约束「参数个数必须为 2」：
 * 多传参数，gcc -std=c99 应报错（too many arguments to function 'calloc'）。 */
void *bad_argc2 = calloc(1, 1, 1);

/* 违反约束「calloc 返回 void *，不能直接解引用」：
 * 对 void * 解引用是约束违反，gcc -std=c99 应报错
 * （dereferencing 'void *' pointer）。 */
void bad_deref(void)
{
    void *p = calloc(1, 1);
    *p = 0;   /* 对 void * 解引用，应报错 */
}

#endif /* 负向测试结束 */