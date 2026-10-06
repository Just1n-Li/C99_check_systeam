/*
 * 验证 C99 条款 6.5.3.1 Prefix increment and decrement operators
 * 预期行为：
 * - 正向测试：能编译并运行通过，assert 验证结果正确。
 * - 负向测试：违反约束，应编译报错。
 */

#include <stdio.h>
#include <assert.h>

int func(void) {
    return 5;
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    int i;
    float f;
    int arr[3] = {10, 20, 30};
    int *p;

    /* [2] 前缀 ++ 语义：操作数值递增，结果是递增后的新值，等价于 (E+=1) */
    i = 5;
    assert((++i) == 6); /* 结果是新值 */
    assert(i == 6);     /* 副作用：i 被修改 */

    f = 1.5f;
    assert((++f) == 2.5f);
    assert(f == 2.5f);

    p = arr;
    assert((++p) == &arr[1]); /* 指针递增，指向下一个元素 */
    assert(*p == 20);

    /* [3] 前缀 -- 语义：操作数值递减，结果是递减后的新值 */
    i = 5;
    assert((--i) == 4);
    assert(i == 4);

    f = 1.5f;
    assert((--f) == 0.5f);
    assert(f == 0.5f);

    p = &arr[2];
    assert((--p) == &arr[1]);
    assert(*p == 20);

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* [1] 违反约束「操作数必须是可修改的左值」：const 变量不可修改 */
    const int ci = 0;
    ++ci;
    --ci;

    /* [1] 违反约束「操作数必须是可修改的左值」：数组名不可修改 */
    int arr2[3];
    ++arr2;
    --arr2;

    /* [1] 违反约束「操作数必须是可修改的左值」：算术表达式结果不是左值 */
    int x = 0;
    ++(x + 1);
    --(x + 1);

    /* [1] 违反约束「操作数必须是可修改的左值」：函数返回值不是左值 */
    ++func();
    --func();

    /* [1] 违反约束「操作数必须具有实数或指针类型」：结构体不是算术或指针类型 */
    struct S { int a; } s;
    ++s;
    --s;

    /* [1] 违反约束「操作数必须具有实数或指针类型」：联合体不是算术或指针类型 */
    union U { int a; } u;
    ++u;
    --u;
    #endif

    return 0;
}