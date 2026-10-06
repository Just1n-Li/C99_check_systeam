/*
 * 验证 C99 6.8.6.4 The return statement
 * 预期行为：正向测试运行通过，负向测试编译报错
 */
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] 函数可以有任意数量的 return 语句，并终止执行返回控制权 */
int test_multiple_returns(int x) {
    if (x < 0) return -1;
    if (x == 0) return 0;
    return 1;
}

/* [3] 带表达式的 return，返回值正确，且发生类型转换（int -> double） */
double test_return_conversion_to_double(int x) {
    return x;
}

/* [3] 带表达式的 return，返回值正确，且发生类型转换（double -> int，截断） */
int test_return_conversion_to_int(double x) {
    return x;
}

/* [4] EXAMPLE: 通过函数返回结构体避免直接赋值时的重叠 UB */
struct s { double i; };

union u {
    struct { int f1; struct s f2; } u1;
    struct { struct s f3; int f4; } u2;
};

union u g;

struct s f(void) {
    return g.u1.f2;
}

int main(void) {
    /* [2] 验证多个 return 语句和控制权返回 */
    assert(test_multiple_returns(-5) == -1);
    assert(test_multiple_returns(0) == 0);
    assert(test_multiple_returns(10) == 1);

    /* [3] 验证返回值的类型转换 */
    assert(test_return_conversion_to_double(5) == 5.0);
    assert(test_return_conversion_to_int(3.9) == 3);

    /* [4] EXAMPLE: 验证通过函数调用获取值并赋值给重叠成员不产生 UB */
    g.u1.f2.i = 42.0;
    /* 如果直接 g.u2.f3 = g.u1.f2 会有 UB，但通过函数返回则没有 */
    g.u2.f3 = f();
    assert(g.u2.f3.i == 42.0);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束「带表达式的 return 不能出现在 void 函数中」 */
void bad_return_with_expr(void) {
    return 1; /* gcc -std=c99 应报错：warning: 'return' with a value, in function returning void */
}

/* [1] 违反约束「不带表达式的 return 只能出现在 void 函数中」 */
int bad_return_without_expr(void) {
    return; /* gcc -std=c99 应报错：warning: 'return' with no value, in function returning non-void */
}
#endif