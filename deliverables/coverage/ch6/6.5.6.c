/* 验证 C99 6.5.6 Additive operators
 * 正向测试：应能编译并运行通过（assert 全部成立）
 * 负向测试：违反 6.5.6 约束 [2][3]，应编译报错
 */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 语法：additive-expression 由 multiplicative-expression 经 +/- 构成，左结合 */
    {
        int a = 6, b = 2, c = 3;
        int r1 = a + b * c;       /* 乘法优先于加法 */
        int r2 = a - b + c;       /* 加减法左结合 */
        assert(r1 == 12);
        assert(r2 == 7);
    }

    /* [2] 加法约束：两个算术类型相加 */
    {
        int i = 3, j = 4;
        double d = 2.5;
        assert(i + j == 7);
        assert(i + d == 5.5);
    }

    /* [2] 加法约束：指针 + 整数（指针指向对象类型） */
    {
        int arr[5] = {10, 20, 30, 40, 50};
        int *p = arr;
        assert(*(p + 2) == 30);
        assert(*(2 + p) == 30);   /* N + (P) 与 (P) + N 等价 */
    }

    /* [3] 减法约束：两个算术类型相减 */
    {
        int i = 10, j = 3;
        double d = 1.5;
        assert(i - j == 7);
        assert(i - d == 8.5);
    }

    /* [3] 减法约束：指针 - 整数 */
    {
        int arr[5] = {10, 20, 30, 40, 50};
        int *p = &arr[4];
        assert(*(p - 2) == 30);
    }

    /* [3] 减法约束：指针 - 指针（指向兼容的限定/非限定对象类型） */
    {
        int arr[5] = {0, 1, 2, 3, 4};
        int *p = &arr[4];
        const int *q = &arr[1];   /* const 限定版本，仍兼容 */
        ptrdiff_t diff = p - q;
        assert(diff == 3);
    }

    /* [4] 通常算术转换：int + unsigned → unsigned；float + double → double */
    {
        int i = 1;
        unsigned u = 2u;
        unsigned r = i + u;
        assert(r == 3u);

        float f = 1.5f;
        double d = 2.5;
        double dr = f + d;
        assert(dr == 4.0);
    }

    /* [5] 二元 + 结果为两操作数之和 */
    {
        int a = 100, b = 23;
        assert(a + b == 123);
    }

    /* [6] 二元 - 结果为第二操作数从第一操作数中减去后的差 */
    {
        int a = 100, b = 23;
        assert(a - b == 77);
    }

    /* [7] 指向非数组对象的指针，等同于指向长度为 1 的数组首元素的指针 */
    {
        int x = 42;
        int *p = &x;
        assert(*(p + 0) == 42);
        int *one_past = p + 1;    /* 合法：one-past-the-end，不解引用 */
        (void)one_past;
    }

    /* [8] 指针 + 整数：结果类型为指针操作数类型；指向偏移元素 */
    {
        int arr[5] = {0, 10, 20, 30, 40};
        int *p = arr;             /* 指向第 0 个 */
        int *p2 = p + 3;          /* 指向第 3 个 */
        assert(*p2 == 30);
        assert(p2 - p == 3);

        /* (P)+N 与 N+(P) 等价 */
        assert(*(p + 2) == *(2 + p));

        /* (P)-N 指向第 i-n 个元素 */
        int *p3 = &arr[4];
        assert(*(p3 - 4) == 0);

        /* P 指向最后一个元素，P+1 指向 one-past-the-end */
        int *last = &arr[4];
        int *one_past = last + 1;
        /* Q 指向 one-past-the-end，Q-1 指向最后一个元素 */
        assert(*(one_past - 1) == 40);
        (void)one_past;

        /* 结果类型为指针操作数类型 */
        assert(sizeof(p + 1) == sizeof(int*));
    }

    /* [9] 指针相减：结果为两下标之差，类型 ptrdiff_t（有符号整数） */
    {
        int arr[10];
        int *p = &arr[9], *q = &arr[2];
        ptrdiff_t d = p - q;
        assert(d == 7);
        assert((q - p) == -7);

        /* P 指向 one-past-the-end 时，((Q)+1)-(P) == 0 */
        int *Q = &arr[9];
        int *P = Q + 1;           /* one-past-the-end */
        assert(((Q + 1) - P) == 0);
        assert(((Q + 1) - P) == ((Q - P) + 1));
        assert(((Q + 1) - P) == -((P - (Q + 1))));
    }

    /* [9] ptrdiff_t 为有符号类型：可表示负值 */
    {
        int arr[5];
        assert((&arr[3] - &arr[0]) > 0);
        assert((&arr[0] - &arr[3]) < 0);
    }

    /* [10] EXAMPLE：VLA 指针运算 */
    {
        int n = 4, m = 3;
        int a[n][m];
        int (*p)[m] = a;          /* p == &a[0] */
        p += 1;                   /* p == &a[1] */
        (*p)[2] = 99;             /* a[1][2] == 99 */
        assert(a[1][2] == 99);
        n = (int)(p - a);         /* n == 1 */
        assert(n == 1);
    }

    /* [11] 若上例数组改为已知常量大小，结果相同 */
    {
        int a[4][3];
        int (*p)[3] = a;
        p += 1;
        (*p)[2] = 99;
        assert(a[1][2] == 99);
        int n = (int)(p - a);
        assert(n == 1);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 6.5.6 约束，应编译报错 ========== */
#if 0

/* [2] 违反加法约束：两个指针相加（pointer + pointer 不允许）
 *    期望 gcc -std=c99 报错："invalid operands to binary +" */
{
    int arr[5];
    int *p = arr, *q = arr;
    int *r = p + q;
    (void)r;
}

/* [2] 违反加法约束：指针 + 浮点（右操作数非整数类型）
 *    期望报错："invalid operands to binary +" */
{
    int arr[5];
    int *p = arr;
    int *q = p + 1.5;
    (void)q;
}

/* [2] 违反加法约束：指向函数的指针 + 整数（函数类型不是对象类型）
 *    期望报错："pointer to function used in arithmetic" 或类似 */
{
    void fp(void);
    void (*p)(void) = fp;
    void (*q)(void) = p + 1;
    (void)q;
}

/* [3] 违反减法约束：左操作数为整数、右操作数为指针（仅允许 指针-整数，不允许 整数-指针）
 *    期望报错："invalid operands to binary -" */
{
    int arr[5];
    int *p = arr;
    int *q = 3 - p;
    (void)q;
}

/* [3] 违反减法约束：指针 - 浮点（右操作数非整数）
 *    期望报错："invalid operands to binary -" */
{
    int arr[5];
    int *p = arr;
    int *q = p - 2.0;
    (void)q;
}

/* [3] 违反减法约束：两个指向不兼容对象类型的指针相减
 *    期望报错："invalid operands to binary -" 或类型不兼容 */
{
    int arr[5];
    double darr[5];
    int *p = arr;
    double *q = darr;
    ptrdiff_t d = p - q;
    (void)d;
}

/* [3] 违反减法约束：指向函数的指针相减（函数类型不是对象类型）
 *    期望报错："pointer to function used in arithmetic" 或类似 */
{
    void f1(void), f2(void);
    void (*p)(void) = f1;
    void (*q)(void) = f2;
    ptrdiff_t d = p - q;
    (void)d;
}

#endif