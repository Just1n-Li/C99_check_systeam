/*
 * 验证 C99 条款 6.7.5.1 Pointer declarators
 * 预期行为：
 * - 正向测试：能编译并运行通过，断言成功。
 * - 负向测试：违反 C99 约束，编译器应报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
int main(void) {
    int a = 10;
    int b = 20;

    /* [1] 语义：* type-qualifier-list D 声明指针，限定符限定的是指针本身 */
    /* [3] EXAMPLE：variable pointer to a constant value */
    const int *ptr_to_constant = &a;
    
    /* 语义验证：ptr_to_constant 本身可以被修改，指向另一个对象 */
    ptr_to_constant = &b;
    assert(*ptr_to_constant == 20);

    /* [3] EXAMPLE：constant pointer to a variable value */
    int *const constant_ptr = &a;
    
    /* 语义验证：constant_ptr 指向的内容可以被修改 */
    *constant_ptr = 30;
    assert(a == 30);

    /* [2] 语义：两个指针类型兼容的条件是限定符完全相同且指向兼容类型 */
    /* 这里测试兼容的指针赋值 */
    const int *cip1 = &a;
    const int *cip2 = cip1; /* 兼容：都是 const-qualified pointer to int */
    assert(cip2 == &a);

    /* [4] EXAMPLE：typedef int *int_ptr; const int_ptr constant_ptr; */
    /* 语义验证：const int_ptr 等价于 int *const，而不是 const int * */
    typedef int *int_ptr;
    const int_ptr constant_ptr2 = &b;
    
    /* 语义验证：constant_ptr2 是 const 限定的指针，但其指向的内容不是 const */
    *constant_ptr2 = 40;
    assert(b == 40);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1][3] 违反约束：不能通过 ''variable pointer to a constant value'' 修改指向的内容 */
void test_const_ptr_to_value(void) {
    int a = 10;
    const int *ptr_to_constant = &a;
    *ptr_to_constant = 20; /* gcc -std=c99 应报错：assignment of read-only location '*ptr_to_constant' */
}

/* [1][3] 违反约束：不能修改 ''constant pointer to a variable value'' 本身的值 */
void test_const_ptr_itself(void) {
    int a = 10, b = 20;
    int *const constant_ptr = &a;
    constant_ptr = &b; /* gcc -std=c99 应报错：assignment of read-only variable 'constant_ptr' */
}

/* [2] 违反约束：类型不兼容的指针赋值（丢弃 const 限定符） */
void test_incompatible_pointer_types(void) {
    const int *pc = NULL;
    int *p = pc; /* gcc -std=c99 应报错：assignment from incompatible pointer type [-Werror=int-conversion] 或类似约束错误 */
}

/* [4] 违反约束：const int_ptr 是 const-qualified pointer to int，不能修改指针本身 */
void test_typedef_const_ptr(void) {
    typedef int *int_ptr;
    int a = 10, b = 20;
    const int_ptr constant_ptr = &a;
    constant_ptr = &b; /* gcc -std=c99 应报错：assignment of read-only variable 'constant_ptr' */
}
#endif