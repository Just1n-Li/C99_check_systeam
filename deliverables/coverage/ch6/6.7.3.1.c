/*
 * 验证 C99 6.7.3.1 (Formal definition of restrict)
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [7] EXAMPLE 1: 文件作用域 restrict 指针 */
int * restrict a;
int * restrict b;
int c_arr[10]; /* 模拟 extern int c[] */

/* [8] EXAMPLE 2: 函数参数 restrict 指针 */
void f(int n, int * restrict p, int * restrict q) {
    while (n-- > 0) *p++ = *q++;
}

/* [10] EXAMPLE 3: 未修改对象可以通过两个 restrict 指针别名 */
void h(int n, int * restrict p, int * restrict q, int * restrict r) {
    int i;
    for (i = 0; i < n; i++) p[i] = q[i] + r[i];
}

/* [12] EXAMPLE 4 例外: 返回包含 restrict 指针的结构体 */
typedef struct {
    int n;
    float * restrict v;
} vector;

vector new_vector(int n) {
    vector t;
    t.n = n;
    t.v = malloc(n * sizeof(float));
    return t;
}

/* 脚注 119: int ** restrict */
void footnote_119(int ** restrict p) {
    /* p 和 p+1 基于 p，*p 和 p[1] 不基于 p */
    int *x = *p;
    int *y = p[1];
    (void)x; (void)y;
}

int main(void) {
    int i;
    
    /* [1] [2] [3] [4] [5] [6] 基本语义测试 */
    int arr1[10] = {0};
    int arr2[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int * restrict p = arr1;
    int * restrict q = arr2;
    
    for (i = 0; i < 10; i++) {
        p[i] = q[i];
    }
    assert(p[5] == 6);

    /* [7] EXAMPLE 1 测试 */
    a = arr1;
    b = arr2;
    a[0] = 100;
    assert(a[0] == 100);
    b[0] = 200;
    assert(b[0] == 200);

    /* [8] [9] EXAMPLE 2 测试 */
    int src[5] = {10, 20, 30, 40, 50};
    int dst[5] = {0};
    f(5, dst, src);
    assert(dst[4] == 50);

    /* [9] EXAMPLE 2 解释: f(50, d + 50, d) 是合法的 */
    int d[100];
    for (i = 0; i < 100; i++) d[i] = i;
    f(50, d + 50, d); /* valid */
    assert(d[50] == 0);
    assert(d[99] == 49);

    /* [10] EXAMPLE 3 测试 */
    int A[5] = {1, 2, 3, 4, 5};
    int B[5] = {10, 20, 30, 40, 50};
    int C[5] = {0};
    h(5, C, A, B);
    assert(C[2] == 33);
    
    /* [10] 未修改对象别名测试: h(100, a, b, b) */
    int a_arr[3] = {1, 2, 3};
    int b_arr[3] = {10, 20, 30};
    int c_res[3] = {0};
    h(3, c_res, b_arr, b_arr); /* b_arr 未被修改，可以别名 */
    assert(c_res[2] == 60);

    /* [11] EXAMPLE 4: 嵌套块赋值 */
    {
        int * restrict p1 = arr1;
        int * restrict q1 = arr2;
        {
            int * restrict p2 = p1; /* valid */
            int * restrict q2 = q1; /* valid */
            p2[0] = 999;
            q2[0] = 888;
            assert(p2[0] == 999);
            assert(q2[0] == 888);
        }
    }

    /* [12] EXAMPLE 4 例外: 返回包含 restrict 指针的结构体 */
    vector vec = new_vector(5);
    assert(vec.n == 5);
    for (i = 0; i < 5; i++) {
        vec.v[i] = (float)i * 1.5f;
    }
    assert(vec.v[3] == 4.5f);
    free(vec.v);

    /* 脚注 119 测试 */
    int *ptrs[2] = {arr1, arr2};
    int ** restrict pp = ptrs;
    footnote_119(pp);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反 6.7.3 约束：restrict 限定符只能用于指向对象类型或不完整类型的指针 */
restrict int x;
int restrict y;

/* 违反 6.7.3 约束：restrict 限定符不能在同一 specifier-qualifier-list 中出现多次 */
int * restrict restrict p;

/* 违反 6.7.3 约束：restrict 不能用于指向函数的指针（函数类型不是对象类型或不完整类型） */
int (* restrict fp)(int);

#endif