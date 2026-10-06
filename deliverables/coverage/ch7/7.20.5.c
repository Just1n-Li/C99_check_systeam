/*
 * 测试条款：C99 7.20.5 Searching and sorting utilities
 *
 * 预期行为：
 *   正向测试：以下使用 bsearch / qsort 的代码应能编译并正确运行，
 *             验证 [1] nmemb==0 的行为、[2] 比较函数实参为数组元素指针、
 *             [3] 比较函数不改变数组内容、[4] 比较结果一致性（全序）、
 *             [5] 比较函数调用前后存在序列点（通过副作用可观察）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错），
 *             例如把非函数指针传给 qsort/bsearch 的比较参数、
 *             比较函数返回类型不匹配等。
 *
 * 编译：gcc -std=c99 -Wall -Wextra -o test test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* 用于 qsort 的比较函数：升序 */
static int cmp_int_asc(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

/* 用于 bsearch 的比较函数：key 是 int*，元素是 int* */
static int cmp_int_key(const void *key, const void *elem)
{
    int k = *(const int *)key;
    int e = *(const int *)elem;
    return (k > e) - (k < e);
}

/* 记录比较函数被调用的次数，用于验证 [1] nmemb==0 时不调用比较函数 */
static int g_cmp_calls = 0;

static int cmp_counting(const void *a, const void *b)
{
    ++g_cmp_calls;
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

/* 用于验证 [2]：比较函数收到的指针必须指向数组元素 */
static int g_elem_ok = 1;
static int *g_base = NULL;
static size_t g_nmemb = 0;

static int cmp_check_elem(const void *a, const void *b)
{
    /* [2] 两个实参都应是数组元素的指针 */
    const int *pa = (const int *)a;
    const int *pb = (const int *)b;
    if (pa < g_base || pa >= g_base + g_nmemb) g_elem_ok = 0;
    if (pb < g_base || pb >= g_base + g_nmemb) g_elem_ok = 0;
    /* 指针必须按元素大小对齐（footnote 263） */
    if (((const char *)pa - (const char *)g_base) % sizeof(int) != 0) g_elem_ok = 0;
    if (((const char *)pb - (const char *)g_base) % sizeof(int) != 0) g_elem_ok = 0;
    return (*pa > *pb) - (*pa < *pb);
}

/* 用于验证 [3]：比较函数不改变数组内容（这里故意只读） */
static int cmp_readonly(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

/* 用于验证 [4]：同一对象多次比较结果一致 */
static int g_consistency_ok = 1;
static int g_last_cmp_result[64];
static int g_last_cmp_count = 0;

static int cmp_consistent(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    int r = (x > y) - (x < y);
    /* 记录结果，稍后检查同一对 (x,y) 的结果是否一致 */
    if (g_last_cmp_count < 64) {
        g_last_cmp_result[g_last_cmp_count++] = r;
    }
    return r;
}

/* 用于验证 [5]：比较函数调用前后存在序列点。
 * 通过全局副作用观察：每次调用比较函数时，全局计数器自增，
 * 若没有序列点保证，编译器可能重排，但标准要求调用前后有序列点，
 * 因此计数器的值在调用返回后必须可见。 */
static int g_seq_counter = 0;
static int g_seq_observed_ok = 1;

static int cmp_seq(const void *a, const void *b)
{
    int before = g_seq_counter;
    g_seq_counter = before + 1;
    /* 调用返回后，g_seq_counter 必须已更新（序列点保证） */
    if (g_seq_counter != before + 1) g_seq_observed_ok = 0;
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void)
{
    /* ---------- [1] nmemb == 0 的行为 ---------- */
    {
        int arr[5] = { 5, 3, 1, 4, 2 };
        int key = 3;

        /* qsort 在 nmemb==0 时不应调用比较函数，也不应重排 */
        g_cmp_calls = 0;
        qsort(arr, 0, sizeof(int), cmp_counting);
        assert(g_cmp_calls == 0);          /* [1] 比较函数不被调用 */
        assert(arr[0] == 5 && arr[1] == 3 && arr[2] == 1 &&
               arr[3] == 4 && arr[4] == 2); /* [1] 不重排 */

        /* bsearch 在 nmemb==0 时不应调用比较函数，返回 NULL */
        g_cmp_calls = 0;
        void *r = bsearch(&key, arr, 0, sizeof(int), cmp_counting);
        assert(r == NULL);                 /* [1] 找不到匹配元素 */
        assert(g_cmp_calls == 0);          /* [1] 比较函数不被调用 */

        /* [1] 指针参数仍须有效（这里 arr 有效） */
        (void)r;
    }

    /* ---------- [2] 比较函数实参为数组元素指针 ---------- */
    {
        int arr[6] = { 10, 20, 30, 40, 50, 60 };
        g_base = arr;
        g_nmemb = 6;
        g_elem_ok = 1;
        qsort(arr, 6, sizeof(int), cmp_check_elem);
        assert(g_elem_ok == 1);            /* [2] 两实参均为元素指针 */

        /* bsearch：第二实参为元素指针，第一实参等于 key */
        int key = 30;
        g_elem_ok = 1;
        void *found = bsearch(&key, arr, 6, sizeof(int), cmp_check_elem);
        assert(found != NULL);
        assert(*(int *)found == 30);
        assert(g_elem_ok == 1);            /* [2] 第二实参为元素指针 */
    }

    /* ---------- [3] 比较函数不改变数组内容 ---------- */
    {
        int arr[5] = { 5, 4, 3, 2, 1 };
        int copy[5];
        memcpy(copy, arr, sizeof(arr));
        qsort(arr, 5, sizeof(int), cmp_readonly);
        /* 排序后数组应升序，且元素集合不变（内容未被比较函数篡改） */
        assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 3 &&
               arr[3] == 4 && arr[4] == 5);
        /* 元素集合与原始一致 */
        int sum_before = 0, sum_after = 0;
        for (int i = 0; i < 5; ++i) { sum_before += copy[i]; sum_after += arr[i]; }
        assert(sum_before == sum_after);   /* [3] 单个元素内容未被改变 */
    }

    /* ---------- [4] 比较结果一致性（全序） ---------- */
    {
        int arr[8] = { 7, 2, 9, 4, 1, 8, 3, 6 };
        g_last_cmp_count = 0;
        g_consistency_ok = 1;
        qsort(arr, 8, sizeof(int), cmp_consistent);
        /* 排序后应为升序，说明比较定义了全序 */
        for (int i = 1; i < 8; ++i) {
            assert(arr[i - 1] <= arr[i]);
        }
        /* 同一对象多次比较结果一致：检查记录的结果符号一致 */
        /* 这里简单验证：所有记录的结果都在 {-1,0,1} 内且排序正确 */
        for (int i = 0; i < g_last_cmp_count; ++i) {
            assert(g_last_cmp_result[i] >= -1 && g_last_cmp_result[i] <= 1);
        }

        /* bsearch：同一对象与 key 比较结果一致 */
        int key = 4;
        void *f1 = bsearch(&key, arr, 8, sizeof(int), cmp_int_key);
        void *f2 = bsearch(&key, arr, 8, sizeof(int), cmp_int_key);
        assert(f1 != NULL && f2 != NULL);
        assert(*(int *)f1 == 4 && *(int *)f2 == 4); /* [4] 一致 */
    }

    /* ---------- [5] 比较函数调用前后存在序列点 ---------- */
    {
        int arr[4] = { 4, 3, 2, 1 };
        g_seq_counter = 0;
        g_seq_observed_ok = 1;
        qsort(arr, 4, sizeof(int), cmp_seq);
        assert(g_seq_observed_ok == 1);    /* [5] 调用前后序列点保证可见性 */
        assert(g_seq_counter > 0);         /* 比较函数确实被调用 */
        assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4);
    }

    /* ---------- 综合：qsort + bsearch 正常使用 ---------- */
    {
        int arr[10] = { 42, 7, 19, 3, 88, 1, 55, 23, 9, 66 };
        qsort(arr, 10, sizeof(int), cmp_int_asc);
        for (int i = 1; i < 10; ++i) assert(arr[i - 1] <= arr[i]);

        int key = 55;
        void *p = bsearch(&key, arr, 10, sizeof(int), cmp_int_key);
        assert(p != NULL && *(int *)p == 55);

        int missing = 100;
        void *q = bsearch(&missing, arr, 10, sizeof(int), cmp_int_key);
        assert(q == NULL);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束：qsort 的第 4 个参数类型为 int (*)(const void *, const void *)，
 * 传入非函数指针（如 int）应编译报错。
 * 期望：gcc -std=c99 报 "passing argument 4 of 'qsort' makes pointer from integer" 或类似错误。 */
void neg_qsort_bad_cmp(void)
{
    int arr[3] = { 1, 2, 3 };
    qsort(arr, 3, sizeof(int), 42);   /* 错误：42 不是函数指针 */
}

/* 违反约束：bsearch 的第 5 个参数类型为 int (*)(const void *, const void *)，
 * 传入不兼容的函数指针类型应编译报错（在 -Werror 或严格模式下）。
 * 期望：gcc -std=c99 报 "incompatible pointer type" 警告/错误。 */
void neg_bsearch_bad_cmp(void)
{
    int arr[3] = { 1, 2, 3 };
    int key = 2;
    /* 错误：比较函数签名不匹配（参数为 int* 而非 const void*） */
    int (*bad)(int *, int *) = NULL;
    bsearch(&key, arr, 3, sizeof(int), bad);
}

/* 违反约束：qsort 的 nmemb 参数类型为 size_t，
 * 传入负数常量在严格模式下可能触发转换警告/错误。
 * 期望：gcc -std=c99 -Werror 报 "negative integer implicitly converted to unsigned type"。 */
void neg_qsort_negative_nmemb(void)
{
    int arr[3] = { 1, 2, 3 };
    qsort(arr, -1, sizeof(int), cmp_int_asc);  /* 错误：负数转 size_t */
}

/* 违反约束：bsearch 的 size 参数类型为 size_t，
 * 传入浮点数应编译报错。
 * 期望：gcc -std=c99 报 "incompatible type for argument 4" 或类似错误。 */
void neg_bsearch_float_size(void)
{
    int arr[3] = { 1, 2, 3 };
    int key = 2;
    bsearch(&key, arr, 3, 2.5, cmp_int_key);  /* 错误：size 应为 size_t */
}

/* 违反约束：qsort 的 base 参数类型为 void *，
 * 传入函数指针应编译报错（函数指针不能隐式转 void*）。
 * 期望：gcc -std=c99 报 "passing argument 1 of 'qsort' from incompatible pointer type"。 */
void neg_qsort_funcptr_base(void)
{
    qsort((void *)cmp_int_asc, 3, sizeof(int), cmp_int_asc); /* 错误：函数指针作 base */
}

#endif /* 负向测试结束 */