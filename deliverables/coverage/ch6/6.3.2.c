/*
 * 验证 C99 条款 6.3.2 (Other operands)
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [6.3.2.1-3] 数组到指针的转换 */
void test_array_to_ptr(void) {
    int arr[3] = {10, 20, 30};
    int *p = arr; /* 数组类型的左值转换为指向首元素的指针 */
    assert(p[0] == 10 && p[1] == 20);
    
    /* sizeof 和 & 操作数不发生转换 */
    assert(sizeof(arr) == sizeof(int) * 3);
    int (*pa)[3] = &arr;
    assert((*pa)[2] == 30);
}

/* [6.3.2.1-4] 函数指示符到指针的转换 */
void dummy_func(void) {}

void test_func_to_ptr(void) {
    /* 函数指示符转换为指向函数的指针 */
    void (*fp)(void) = dummy_func;
    fp(); /* 调用 */
    
    /* & 操作数不发生转换 */
    void (*fp2)(void) = &dummy_func;
    assert(fp == fp2);
}

/* [6.3.2.2-3] void 表达式求值 */
void v_func(void) { /* do nothing */ }

void test_void_expr(void) {
    /* void 表达式可以作为逗号表达式的左操作数，其求值不产生任何值 */
    int x = (v_func(), 42);
    assert(x == 42);
}

/* [6.3.2.3-1,2] void* 转换与往返 */
void test_void_ptr(void) {
    int i = 99;
    void *vp = &i; /* 对象指针转 void* */
    int *ip = vp;  /* void* 转对象指针 */
    assert(*ip == 99);
}

/* [6.3.2.3-4,5,6] 指针与整数转换 */
void test_ptr_int_conv(void) {
    int i = 100;
    int *p = &i;
    /* 指针转换为足够大的整数类型 */
    uintptr_t ui = (uintptr_t)p;
    /* 整数转换回指针 */
    int *p2 = (int*)ui;
    assert(p2 == p);
    assert(*p2 == 100);
}

/* [6.3.2.3-7] 指针对齐转换 */
void test_ptr_align_conv(void) {
    int a = 55;
    int *pi = &a;
    /* 转换为对齐要求更小的指针 */
    char *pc = (char*)pi;
    /* 转回原类型 */
    int *pi2 = (int*)pc;
    assert(pi2 == pi);
    assert(*pi2 == 55);
}

/* [6.3.2.3-8] 函数指针转换 */
void f1(int x) {}

void test_func_ptr_conv(void) {
    void (*fp1)(int) = f1;
    /* 显式转换为不同函数指针类型 */
    void (*fp2)(double) = (void(*)(double))fp1;
    /* 转回原类型 */
    void (*fp3)(int) = (void(*)(int))fp2;
    assert(fp3 == fp1);
}

/* [6.3.2.3-10,11,12] 空指针 */
void test_null_ptr(void) {
    int *np = NULL;
    int *np2 = 0; /* 值为 0 的整数常量表达式是空指针常量 */
    assert(np == np2);
    assert(np == 0); /* 空指针转换为其他类型的空指针，比较结果相等 */
}

int main(void) {
    test_array_to_ptr();
    test_func_to_ptr();
    test_void_expr();
    test_void_ptr();
    test_ptr_int_conv();
    test_ptr_align_conv();
    test_func_ptr_conv();
    test_null_ptr();
    printf("All positive tests passed!\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [6.3.2.1-2] 违反约束「不可修改左值」：对 const 限定类型赋值 */
const int ci = 0;
void test_const_assign(void) {
    ci = 1; /* gcc -std=c99 应报错：assignment of read-only variable 'ci' */
}

/* [6.3.2.1-2] 违反约束「不可修改左值」：对数组类型赋值 */
void test_array_assign(void) {
    int arr1[2] = {1, 2};
    int arr2[2] = {3, 4};
    arr1 = arr2; /* gcc -std=c99 应报错：assignment to expression with array type */
}

/* [6.3.2.1-5] 违反约束「赋值运算符左操作数必须是可修改左值」：对非左值赋值 */
struct S { int x; };
struct S get_s(void) { struct S s = {1}; return s; }
void test_non_lvalue_assign(void) {
    get_s().x = 2; /* gcc -std=c99 应报错：lvalue required as left operand of assignment */
}

/* [6.3.2.2-1] 违反约束「void 表达式不能转换为除 void 外的任何类型」 */
void v_func2(void) {}
void test_void_cast(void) {
    int x = (int)v_func2(); /* gcc -std=c99 应报错：void value not ignored as it ought to be */
}

/* [6.3.2.2-2] 违反约束「void 表达式的值不能以任何方式使用」 */
void test_void_use(void) {
    int x = v_func2() + 1; /* gcc -std=c99 应报错：void value not ignored as it ought to be */
}

/* [6.3.2.3-3] 违反约束「不兼容指针类型的隐式转换」 */
void test_incompatible_ptr(void) {
    int i = 0;
    float *fp = &i; /* gcc -std=c99 应报错：pointer from incompatible pointer type */
}

#endif