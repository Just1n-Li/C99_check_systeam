/*
 * 测试条款：C99 7.20.5.1  The bsearch function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdlib.h>
 *   [2] 在 nmemb 个对象中查找，元素大小由 size 指定
 *   [3] compar 的调用约定（key 在前、数组元素在后），返回值语义，
 *       以及数组必须按 compar 排好序（小于、相等、大于三段）
 *   [4] 返回值：匹配元素指针，或未找到时返回空指针；
 *       两个元素相等时匹配哪一个未指定
 *   Footnote 264：实践中整个数组按比较函数排序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ---------- 供正向测试使用的比较函数 ---------- */

/* 比较 int，符合 [3]：key 在前，数组元素在后 */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

/* 比较字符串指针（数组元素为 const char *） */
static int cmp_str(const void *a, const void *b)
{
    const char *const *pa = (const char *const *)a;
    const char *const *pb = (const char *const *)b;
    return strcmp(*pa, *pb);
}

/* 记录 compar 被调用时的参数顺序，用于验证 [3] 的“key 在前” */
static const void *g_first_arg  = NULL;
static const void *g_second_arg = NULL;
static int cmp_record(const void *a, const void *b)
{
    g_first_arg  = a;
    g_second_arg = b;
    return cmp_int(a, b);
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：包含 <stdlib.h> 后 bsearch 可被调用，
     *     返回类型为 void *，参数类型与标准一致。 */
    {
        void *(*fp)(const void *, const void *, size_t, size_t,
                    int (*)(const void *, const void *)) = bsearch;
        assert(fp != NULL);
    }

    /* [2][3][4] 基本查找：在已排序的 int 数组中查找存在的元素 */
    {
        int arr[] = { 1, 3, 5, 7, 9, 11, 13 };
        size_t n = sizeof arr / sizeof arr[0];
        int key;

        /* 查找每个存在的元素，应返回指向匹配元素的指针 */
        for (size_t i = 0; i < n; i++) {
            key = arr[i];
            int *p = (int *)bsearch(&key, arr, n, sizeof arr[0], cmp_int);
            assert(p != NULL);          /* [4] 找到则非空 */
            assert(*p == key);          /* [4] 指向匹配元素 */
            assert(p >= arr && p < arr + n);
        }

        /* [4] 未找到时返回空指针 */
        key = 0;
        assert(bsearch(&key, arr, n, sizeof arr[0], cmp_int) == NULL);
        key = 2;
        assert(bsearch(&key, arr, n, sizeof arr[0], cmp_int) == NULL);
        key = 100;
        assert(bsearch(&key, arr, n, sizeof arr[0], cmp_int) == NULL);
    }

    /* [2] size 参数决定元素大小：用结构体数组验证 */
    {
        struct Rec { int k; int payload; };
        struct Rec recs[] = {
            { 10, 100 }, { 20, 200 }, { 30, 300 }, { 40, 400 }
        };
        size_t n = sizeof recs / sizeof recs[0];
        struct Rec key = { 30, 0 };
        struct Rec *p = (struct Rec *)bsearch(&key, recs, n,
                                              sizeof recs[0], cmp_int);
        assert(p != NULL);
        assert(p->k == 30);
        assert(p->payload == 300);      /* 证明 size 被正确使用 */

        key.k = 35;
        assert(bsearch(&key, recs, n, sizeof recs[0], cmp_int) == NULL);
    }

    /* [3] compar 的调用顺序：第一个实参指向 key，第二个指向数组元素 */
    {
        int arr[] = { 2, 4, 6, 8 };
        size_t n = sizeof arr / sizeof arr[0];
        int key = 6;
        g_first_arg = g_second_arg = NULL;
        int *p = (int *)bsearch(&key, arr, n, sizeof arr[0], cmp_record);
        assert(p != NULL && *p == 6);
        assert(g_first_arg == (const void *)&key);   /* key 在前 */
        assert(g_second_arg >= (const void *)arr &&
               g_second_arg <  (const void *)(arr + n)); /* 元素在后 */
    }

    /* [3] compar 返回值语义：<0 / ==0 / >0 分别表示小于 / 匹配 / 大于 */
    {
        int a = 5, b = 7;
        assert(cmp_int(&a, &b) < 0);
        assert(cmp_int(&b, &a) > 0);
        assert(cmp_int(&a, &a) == 0);
    }

    /* [3] 数组必须按 compar 排好序（小于、相等、大于三段） */
    {
        const char *words[] = { "apple", "banana", "cherry", "date" };
        size_t n = sizeof words / sizeof words[0];
        const char *key = "cherry";
        const char **p = (const char **)bsearch(&key, words, n,
                                                sizeof words[0], cmp_str);
        assert(p != NULL);
        assert(strcmp(*p, "cherry") == 0);

        key = "fig";
        assert(bsearch(&key, words, n, sizeof words[0], cmp_str) == NULL);
    }

    /* [4] 两个元素比较相等时，匹配哪一个未指定：
     *     只要求返回的指针指向某个与 key 相等的元素，不假设具体是哪一个。 */
    {
        struct Pair { int k; int tag; };
        struct Pair ps[] = { { 1, 11 }, { 2, 22 }, { 2, 33 }, { 3, 44 } };
        size_t n = sizeof ps / sizeof ps[0];
        struct Pair key = { 2, 0 };
        struct Pair *p = (struct Pair *)bsearch(&key, ps, n,
                                                sizeof ps[0], cmp_int);
        assert(p != NULL);
        assert(p->k == 2);              /* 与 key 相等 */
        assert(p->tag == 22 || p->tag == 33); /* 二者之一，未指定 */
    }

    /* [2] nmemb == 0：没有元素，必然找不到，返回空指针 */
    {
        int arr[1] = { 42 };
        int key = 42;
        assert(bsearch(&key, arr, 0, sizeof arr[0], cmp_int) == NULL);
    }

    /* [2] nmemb == 1：单元素数组 */
    {
        int arr[1] = { 42 };
        int key = 42;
        int *p = (int *)bsearch(&key, arr, 1, sizeof arr[0], cmp_int);
        assert(p == arr && *p == 42);

        key = 7;
        assert(bsearch(&key, arr, 1, sizeof arr[0], cmp_int) == NULL);
    }

    /* Footnote 264：整个数组按比较函数排序，bsearch 才能正确工作。
     * 这里用完整排序后的数组做一次全量查找验证。 */
    {
        int arr[] = { -5, -1, 0, 2, 4, 4, 9, 17, 100 };
        size_t n = sizeof arr / sizeof arr[0];
        for (size_t i = 0; i < n; i++) {
            int key = arr[i];
            int *p = (int *)bsearch(&key, arr, n, sizeof arr[0], cmp_int);
            assert(p != NULL && *p == key);
        }
    }

    printf("All positive tests for C99 7.20.5.1 bsearch passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「bsearch 的返回类型为 void *，不能直接解引用」：
     * 对 void * 解引用是非法的，gcc -std=c99 应报错
     * （error: dereferencing 'void *' pointer / invalid use of void expression）。 */
    {
        int arr[] = { 1, 2, 3 };
        int key = 2;
        int v = *bsearch(&key, arr, 3, sizeof arr[0], cmp_int);
        (void)v;
    }

    /* 违反约束「compar 参数类型必须为 int (*)(const void *, const void *)」：
     * 传入 int (*)(int *, int *) 类型不兼容，gcc -std=c99 应报错
     * （error: passing argument 5 ... from incompatible pointer type）。 */
    {
        int arr[] = { 1, 2, 3 };
        int key = 2;
        int (*bad_cmp)(int *, int *) = NULL;
        bsearch(&key, arr, 3, sizeof arr[0], bad_cmp);
    }

    /* 违反约束「bsearch 需要 5 个实参」：
     * 实参个数不足，gcc -std=c99 应报错
     * （error: too few arguments to function 'bsearch'）。 */
    {
        int arr[] = { 1, 2, 3 };
        int key = 2;
        bsearch(&key, arr, 3);
    }

    /* 违反约束「bsearch 需要 5 个实参」：
     * 实参个数过多，gcc -std=c99 应报错
     * （error: too many arguments to function 'bsearch'）。 */
    {
        int arr[] = { 1, 2, 3 };
        int key = 2;
        bsearch(&key, arr, 3, sizeof arr[0], cmp_int, 0);
    }

    /* 违反约束「nmemb 与 size 参数类型为 size_t」：
     * 传入结构体类型无法转换为 size_t，gcc -std=c99 应报错
     * （error: incompatible type for argument 3/4 of 'bsearch'）。 */
    {
        struct S { int x; } s;
        int arr[] = { 1, 2, 3 };
        int key = 2;
        bsearch(&key, arr, s, s, cmp_int);
    }

    /* 违反约束「bsearch 的返回类型为 void *，不能赋给不兼容的指针类型
     * 而不做转换」——在 C99 中 void * 可隐式转换为对象指针，
     * 但赋给函数指针类型则违反约束，gcc -std=c99 应报错
     * （error: assignment ... from incompatible pointer type）。 */
    {
        int arr[] = { 1, 2, 3 };
        int key = 2;
        void (*fp)(void) = bsearch(&key, arr, 3, sizeof arr[0], cmp_int);
        (void)fp;
    }

#endif /* 负向测试结束 */

    return 0;
}