/*
 * 验证 C99 条款 6.3.1.2 Boolean type
 * 预期行为：
 * - 正向测试：编译并运行通过，assert 不触发。
 * - 负向测试：编译报错（违反类型转换约束，非标量类型不能转换为 _Bool）。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    _Bool b;

    /* [1] 整数 0 转换为 _Bool，结果为 0 */
    b = (int)0;
    assert(b == 0);

    /* [1] 整数非 0 转换为 _Bool，结果为 1 */
    b = (int)42;
    assert(b == 1);

    /* [1] 浮点 0.0 转换为 _Bool，结果为 0 */
    b = (double)0.0;
    assert(b == 0);

    /* [1] 浮点非 0.0 转换为 _Bool，结果为 1 */
    b = (double)3.14;
    assert(b == 1);

    /* [1] 空指针转换为 _Bool，结果为 0 */
    int *p = NULL;
    b = (_Bool)p;
    assert(b == 0);

    /* [1] 非空指针转换为 _Bool，结果为 1 */
    int x = 10;
    p = &x;
    b = (_Bool)p;
    assert(b == 1);

    /* [1] 隐式转换：字符类型（属于整数，即标量）'\0' 转换为 _Bool，结果为 0 */
    b = '\0';
    assert(b == 0);

    /* [1] 隐式转换：字符类型 'A' 转换为 _Bool，结果为 1 */
    b = 'A';
    assert(b == 1);

    printf("6.3.1.2 正向测试全部通过。\n");

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 
     * 违反约束：6.3.1.2 规定只有标量值能转换为 _Bool。
     * 结构体属于聚合类型，非标量类型，不能强制转换为 _Bool。
     * 期望编译器报错，例如：conversion to non-scalar type requested
     */
    struct S { int a; } s = {1};
    _Bool b_invalid = (_Bool)s; 

    /* 
     * 违反约束：非标量类型不能隐式转换为 _Bool (违反简单赋值约束 6.5.16.1)
     * 期望编译器报错，例如：incompatible types when assigning to type '_Bool' from type 'struct S'
     */
    _Bool b_invalid2 = s;
#endif

    return 0;
}