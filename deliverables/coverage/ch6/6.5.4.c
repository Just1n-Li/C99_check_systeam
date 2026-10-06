/* 验证 C99 6.5.4 Cast operators
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <stdio.h>
#include <assert.h>

struct S { int a; };

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
int main(void) {
    int i = 10;
    double d = 3.14;

    /* [1] 语法：cast-expression */
    /* [4] 语义：转换值 */
    double d2 = (double)i;
    assert(d2 == 10.0);

    int i2 = (int)d;
    assert(i2 == 3);

    /* [2] 约束：标量类型之间的转换 */
    int *pi = &i;
    char *pc = (char *)pi;
    assert(pc == (char *)pi);

    /* [2] 约束：void 类型的转换 */
    (void)d2;

    /* [3] 约束：涉及指针的转换使用显式 cast */
    int arr[10];
    int *p = arr;
    void *vp = (void *)p;
    int *p2 = (int *)vp;
    assert(p2 == p);

    /* [4] 语义：无转换的 cast 对类型或值没有影响 */
    int x = 5;
    int y = (int)x;
    assert(y == 5);
    assert(sizeof((int)x) == sizeof(int));

    /* [5] 语义：如果值以更大精度表示，cast 指定转换 */
    long double ld = 1.0L;
    double dd = (double)ld;
    assert(dd == 1.0);

    /* Footnote 89: cast 到限定类型等同于非限定类型 */
    int z = 10;
    int val = (const int)z;
    assert(val == 10);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「类型名必须为标量类型」：cast 到结构体类型 */
struct S s;
int val = 10;
s = (struct S)val; /* gcc -std=c99 应报错：conversion to non-scalar type requested */

/* [2] 违反约束「操作数必须为标量类型」：数组不能 cast 到 int */
int arr[10];
int i = (int)arr; /* gcc -std=c99 应报错：used array type where scalar type required */

/* [2] 违反约束「操作数必须为标量类型」：结构体不能 cast 到 int */
struct S s2;
int j = (int)s2; /* gcc -std=c99 应报错：used struct type where scalar type required */

/* [3] 违反约束「涉及指针的转换必须使用显式 cast」：不兼容指针赋值 */
int *pi;
float *pf;
pf = pi; /* gcc -std=c99 应报错：incompatible pointer types */

/* Footnote 89 违反「cast 不产生左值」：对 cast 结果赋值 */
int k = 5;
(int)k = 10; /* gcc -std=c99 应报错：lvalue required as left operand of assignment */

#endif