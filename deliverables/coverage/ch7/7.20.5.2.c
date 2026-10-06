/*
 * 测试目标：C99 7.20.5.2  qsort 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明（<stdlib.h> 中）
 *   [2] 排序 nmemb 个对象，每个对象 size 字节，首元素由 base 指向
 *   [3] 按 compar 升序排序；compar 返回 <0 / 0 / >0 表示小于/等于/大于
 *   [4] 相等元素的相对顺序未指定（不可依赖，仅验证“相等”语义）
 *   [5] 无返回值（void）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [3] 标准比较函数：升序 */
static int cmp_int_asc(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    if (x < y) return -1;
    if (x > y) return  1;
    return 0;
}

/* [3] 降序比较函数，用于验证 compar 的返回值语义 */
static int cmp_int_desc(const void *a, const void *b)
{
    return cmp_int_asc(b, a);
}

/* [3] 比较函数返回“差值”形式（<0/0/>0 语义） */
static int cmp_int_diff(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

/* 用于 [2] 验证 size 参数：结构体数组排序 */
struct Rec {
    int key;
    int tag;
};

static int cmp_rec(const void *a, const void *b)
{
    const struct Rec *ra = (const struct Rec *)a;
    const struct Rec *rb = (const struct Rec *)b;
    if (ra->key < rb->key) return -1;
    if (ra->key > rb->key) return  1;
    return 0;
}

/* 用于 [4] 验证“相等元素”语义：只检查 key 相等，忽略 tag */
static int cmp_rec_key_only(const void *a, const void *b)
{
    const struct Rec *ra = (const struct Rec *)a;
    const struct Rec *rb = (const struct Rec *)b;
    return (ra->key > rb->key) - (ra->key < rb->key);
}

int main(void)
{
    /* ---------- [1] 原型可用性：函数指针类型匹配 ---------- */
    {
        /* 若 <stdlib.h> 未按 [1] 声明 qsort，此处会因类型不匹配而报错 */
        void (*fp)(void *, size_t, size_t,
                   int (*)(const void *, const void *)) = qsort;
        assert(fp != NULL);
    }

    /* ---------- [2][3] 基本升序排序 ---------- */
    {
        int a[] = { 5, 3, 9, 1, 7, 3, 0, -2, 8, 4 };
        size_t n = sizeof a / sizeof a[0];
        size_t i;

        qsort(a, n, sizeof a[0], cmp_int_asc);

        for (i = 1; i < n; ++i)
            assert(a[i - 1] <= a[i]);          /* 升序 */

        assert(a[0] == -2 && a[n - 1] == 9);   /* 端点值 */
    }

    /* ---------- [3] compar 返回值语义：降序 ---------- */
    {
        int a[] = { 5, 3, 9, 1, 7 };
        size_t n = sizeof a / sizeof a[0];
        size_t i;

        qsort(a, n, sizeof a[0], cmp_int_desc);

        for (i = 1; i < n; ++i)
            assert(a[i - 1] >= a[i]);          /* 降序 */
        assert(a[0] == 9 && a[n - 1] == 1);
    }

    /* ---------- [3] compar 返回差值形式同样有效 ---------- */
    {
        int a[] = { 42, -1, 17, 0, 99, -50 };
        size_t n = sizeof a / sizeof a[0];
        size_t i;

        qsort(a, n, sizeof a[0], cmp_int_diff);

        for (i = 1; i < n; ++i)
            assert(a[i - 1] <= a[i]);
        assert(a[0] == -50 && a[n - 1] == 99);
    }

    /* ---------- [2] size 参数：结构体数组（size = sizeof(struct Rec)） ---------- */
    {
        struct Rec r[] = {
            { 3, 100 }, { 1, 200 }, { 2, 300 }, { 1, 400 }, { 3, 500 }
        };
        size_t n = sizeof r / sizeof r[0];
        size_t i;

        qsort(r, n, sizeof r[0], cmp_rec);

        for (i = 1; i < n; ++i)
            assert(r[i - 1].key <= r[i].key);

        /* 每个元素必须整体被搬运：tag 与 key 的配对不能错乱 */
        for (i = 0; i < n; ++i) {
            if (r[i].key == 1)
                assert(r[i].tag == 200 || r[i].tag == 400);
            else if (r[i].key == 2)
                assert(r[i].tag == 300);
            else if (r[i].key == 3)
                assert(r[i].tag == 100 || r[i].tag == 500);
            else
                assert(0);
        }
    }

    /* ---------- [2] nmemb == 0：不应访问数组，也不应崩溃 ---------- */
    {
        int dummy = 123;
        qsort(&dummy, 0, sizeof dummy, cmp_int_asc);
        assert(dummy == 123);                  /* 未被修改 */
    }

    /* ---------- [2] nmemb == 1：单元素，保持原值 ---------- */
    {
        int one = 77;
        qsort(&one, 1, sizeof one, cmp_int_asc);
        assert(one == 77);
    }

    /* ---------- [4] 相等元素：顺序未指定，但“相等”判定必须生效 ---------- */
    {
        struct Rec r[] = {
            { 2, 1 }, { 1, 2 }, { 2, 3 }, { 1, 4 }, { 2, 5 }, { 1, 6 }
        };
        size_t n = sizeof r / sizeof r[0];
        size_t i;
        int cnt1 = 0, cnt2 = 0;

        qsort(r, n, sizeof r[0], cmp_rec_key_only);

        for (i = 1; i < n; ++i)
            assert(r[i - 1].key <= r[i].key);

        for (i = 0; i < n; ++i) {
            if (r[i].key == 1) ++cnt1;
            else if (r[i].key == 2) ++cnt2;
            else assert(0);
        }
        assert(cnt1 == 3 && cnt2 == 3);        /* 元素个数守恒 */
    }

    /* ---------- [3] 大数组：验证排序正确性（含重复值） ---------- */
    {
        enum { N = 1000 };
        static int a[N];
        size_t i;

        for (i = 0; i < N; ++i)
            a[i] = (int)((i * 37u + 11u) % 101u) - 50;   /* 含重复值 */

        qsort(a, N, sizeof a[0], cmp_int_asc);

        for (i = 1; i < N; ++i)
            assert(a[i - 1] <= a[i]);
    }

    /* ---------- [5] 返回类型为 void：不能用于取值 ---------- */
    {
        int a[] = { 2, 1 };
        /* 合法用法：作为表达式语句调用，无返回值 */
        qsort(a, 2, sizeof a[0], cmp_int_asc);
        assert(a[0] == 1 && a[1] == 2);
    }

    printf("C99 7.20.5.2 qsort: all positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * （统一放在 #if 0 中，保证本文件整体仍可编译运行）
 * ============================================================ */
#if 0

/* 违反约束「[1] 原型参数类型」：第 4 个参数必须是
 * int (*)(const void *, const void *)，传入 int (*)(void) 类型不兼容。
 * 期望：gcc -std=c99 报 incompatible pointer type / 参数类型不匹配。 */
static int bad_cmp(void) { return 0; }
void t1(void)
{
    int a[3];
    qsort(a, 3, sizeof a[0], bad_cmp);
}

/* 违反约束「[1] 原型参数类型」：第 2、3 个参数为 size_t，
 * 传入指针类型无法隐式转换。
 * 期望：gcc -std=c99 报 incompatible type for argument。 */
void t2(void)
{
    int a[3];
    int *p = a;
    qsort(a, p, p, cmp_int_asc);
}

/* 违反约束「[1] 原型参数个数」：qsort 需要 4 个实参。
 * 期望：gcc -std=c99 报 too few arguments to function 'qsort'。 */
void t3(void)
{
    int a[3];
    qsort(a, 3, sizeof a[0]);
}

/* 违反约束「[1] 原型参数个数」：实参过多。
 * 期望：gcc -std=c99 报 too many arguments to function 'qsort'。 */
void t4(void)
{
    int a[3];
    qsort(a, 3, sizeof a[0], cmp_int_asc, 0);
}

/* 违反约束「[5] 返回类型为 void」：void 表达式不能作为右值使用。
 * 期望：gcc -std=c99 报 void value not ignored as it ought to be。 */
void t5(void)
{
    int a[3];
    int r = qsort(a, 3, sizeof a[0], cmp_int_asc);
    (void)r;
}

/* 违反约束「[5] 返回类型为 void」：不能对 void 表达式做算术运算。
 * 期望：gcc -std=c99 报 invalid use of void expression。 */
void t6(void)
{
    int a[3];
    int r = qsort(a, 3, sizeof a[0], cmp_int_asc) + 1;
    (void)r;
}

/* 违反约束「[1] 第 1 个参数为 void *」：const 限定对象地址
 * 不能隐式转换为 void *（丢弃 const 限定符）。
 * 期望：gcc -std=c99 报 discards 'const' qualifier。 */
void t7(void)
{
    const int a[3] = { 3, 2, 1 };
    qsort(a, 3, sizeof a[0], cmp_int_asc);
}

/* 违反约束「[1] 第 4 个参数为函数指针」：传入非函数指针（整数）。
 * 期望：gcc -std=c99 报 incompatible type for argument 4。 */
void t8(void)
{
    int a[3];
    qsort(a, 3, sizeof a[0], 0);
}

#endif /* 负向测试结束 */