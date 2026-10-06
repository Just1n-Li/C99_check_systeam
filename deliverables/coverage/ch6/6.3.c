/*
 * 验证 C99 6.3 Conversions
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 隐式转换与显式转换 */
static void test_implicit_explicit_conversion(void) {
    char c = 'A';
    /* 隐式转换：char 提升为 int 参与算术运算 */
    int i = c + 1;
    assert(i == 66);

    double d = 3.14;
    /* 显式转换：cast 操作 */
    int j = (int)d;
    assert(j == 3);
}

/* [1] 默认实参提升：char->int, float->double。
   对于有原型的函数，省略号前的参数按原型转换（停止默认提升），省略号后的参数进行默认提升。 */
static void test_default_argument_promotions(int fixed, ...) {
    va_list ap;
    va_start(ap, fixed);
    /* fixed 参数已按原型转换为 int，不发生默认提升 */
    assert(fixed == 0);
    /* 省略号后的 char 提升为 int */
    int i = va_arg(ap, int);
    assert(i == 65);
    /* 省略号后的 float 提升为 double */
    double d = va_arg(ap, double);
    assert(d > 3.13 && d < 3.15);
    va_end(ap);
}

/* [2] 转换为兼容类型不改变值或表示 */
static void test_compatible_type_conversion(void) {
    int a = 12345;
    int b = (int)a; /* 显式转换为兼容类型 */
    assert(a == b);
    assert(memcmp(&a, &b, sizeof(int)) == 0);

    typedef int MyInt;
    MyInt c = 67890;
    int d = c; /* 隐式转换为兼容类型 */
    assert(c == d);
    assert(memcmp(&c, &d, sizeof(int)) == 0);
}

/* const/volatile 限定类型在成员访问与解引用上的传播（正向：读取允许） */
static void test_qualifier_propagation_read(void) {
    struct S { int x; };
    const struct S s = {10};
    /* s.x 具有 const-qualified 类型，读取是允许的 */
    int val = s.x;
    assert(val == 10);

    const int arr[3] = {1, 2, 3};
    const int *p = arr;
    /* *p 具有 const-qualified 类型，读取是允许的 */
    int v = *p;
    assert(v == 1);
}

int main(void) {
    test_implicit_explicit_conversion();
    test_default_argument_promotions(0, (char)'A', (float)3.14f);
    test_compatible_type_conversion();
    test_qualifier_propagation_read();
    
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束「赋值运算符左操作数必须是可修改的左值」：非左值结果不能赋值 */

/* 函数返回结构体的成员 f().x 是非左值，对其赋值违反约束 */
struct S { int x; };
struct S f(void);
void test_non_lvalue_assign_1(void) {
    f().x = 1;
}

/* 强制转换结果是非左值，对其赋值违反约束 */
void test_non_lvalue_assign_2(void) {
    int a = 0;
    (int)a = 1;
}

/* 条件表达式的结果是非左值，对其赋值违反约束 */
void test_non_lvalue_assign_3(void) {
    int a = 1, b = 2;
    (a > b ? a : b) = 3;
}

/* 逗号表达式的结果是非左值，对其赋值违反约束 */
void test_non_lvalue_assign_4(void) {
    int a = 1, b = 2;
    (a, b) = 3;
}

/* 违反约束「赋值运算符左操作数必须是可修改的左值」：const 限定类型传播导致不可修改 */

/* const 对象不可修改 */
void test_const_assign_1(void) {
    const int ci = 0;
    ci = 1;
}

/* 解引用 const 指针的结果具有 const 限定类型，不可修改 */
void test_const_assign_2(void) {
    const int ci = 0;
    const int *p = &ci;
    *p = 1;
}

/* const 结构体成员访问的结果具有 const 限定类型，不可修改 */
void test_const_assign_3(void) {
    struct S { int x; };
    const struct S s = {0};
    s.x = 1;
}
#endif