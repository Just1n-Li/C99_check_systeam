/*
 * 验证 C99 6.5.2.4 Postfix increment and decrement operators
 * 预期行为：
 * - 正向测试：编译并运行通过，所有 assert 成立。
 * - 负向测试：编译报错（违反约束）。
 */

#include <stdio.h>
#include <assert.h>

int f(void) { return 10; }

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
int main(void) {
    /* [1][2] 实数类型 int 的后缀 ++，结果为原值，操作数递增 */
    int i = 5;
    int j = i++;
    assert(j == 5);
    assert(i == 6);

    /* [1][3] 实数类型 int 的后缀 --，结果为原值，操作数递减 */
    int k = 5;
    int l = k--;
    assert(l == 5);
    assert(k == 4);

    /* [1][2] 实数类型 float 的后缀 ++ */
    float fi = 1.5f;
    float fj = fi++;
    assert(fj == 1.5f);
    assert(fi == 2.5f);

    /* [1][3] 实数类型 double 的后缀 -- */
    double di = 2.5;
    double dj = di--;
    assert(dj == 2.5);
    assert(di == 1.5);

    /* [1] 限定类型 volatile int 的后缀 ++，验证 qualified real type 且为可修改左值 */
    volatile int vi = 10;
    int vj = vi++;
    assert(vj == 10);
    assert(vi == 11);

    /* [1][2] 指针类型的后缀 ++，验证指针按指向类型的大小递增 */
    int arr[5] = {0, 1, 2, 3, 4};
    int *p = arr;
    int *q = p++;
    assert(q == arr);
    assert(*p == 1);

    /* [1][3] 指针类型的后缀 --，验证指针按指向类型的大小递减 */
    int *r = arr + 4;
    int *s = r--;
    assert(s == arr + 4);
    assert(*r == 3);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束「操作数必须为实数或指针类型」：结构体不能后缀自增 */
struct S { int x; } s;
s++;

/* [1] 违反约束「操作数必须是可修改的左值」：const int 不可修改 */
const int ci = 0;
ci++;

/* [1] 违反约束「操作数必须是可修改的左值」：字面量不是左值 */
5++;

/* [1] 违反约束「操作数必须是可修改的左值」：数组名不可修改 */
int arr[5];
arr++;

/* [1] 违反约束「操作数必须是可修改的左值」：函数返回值不是左值 */
f()++;

/* [1] 违反约束「操作数必须是可修改的左值」：算术表达式结果不是左值 */
int a = 1, b = 2;
(a + b)++;

/* [1] 违反约束「操作数必须是可修改的左值」：强制转换结果不是左值 */
float x = 1.0f;
((int)x)++;
#endif