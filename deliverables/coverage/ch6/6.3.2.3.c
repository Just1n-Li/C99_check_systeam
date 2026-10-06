/*
 * 验证 C99 6.3.2.3 Pointers
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [8] 函数指针互转测试用到的函数 */
int sample_func(int a) { return a + 1; }
typedef int (*FuncPtr1)(int);
typedef void (*FuncPtr2)(void);

void test_6_3_2_3_semantics(void) {
    /* [1] void指针与对象/不完整类型指针互转，转回应相等 */
    int x = 42;
    int *p_int = &x;
    void *p_void = p_int; /* 转 void* */
    int *p_int_back = p_void; /* 转回 int* */
    assert(p_int_back == p_int);

    struct Incomplete; /* 不完整类型 */
    struct Incomplete *p_inc = 0;
    void *p_void_inc = p_inc;
    struct Incomplete *p_inc_back = p_void_inc;
    assert(p_inc_back == p_inc);

    /* [2] 非q限定指针转q限定指针，值相等 */
    int y = 10;
    int *p_non_q = &y;
    const int *p_q = p_non_q; /* 转 const int* */
    volatile int *p_v = p_non_q; /* 转 volatile int* */
    assert(p_q == p_non_q);
    assert(p_v == p_non_q);

    /* [3] 空指针常量转为指针类型，保证与任何对象或函数指针不相等 */
    int *p_null1 = 0; /* 整数常量表达式 0 */
    int *p_null2 = (void *)0; /* 转为 void* 的 0 */
    int obj = 1;
    assert(p_null1 != &obj);
    assert(p_null2 != &obj);
    /* 与函数指针比较不等 */
    assert(p_null1 != (int (*)(int))sample_func);

    /* [4] 空指针转其他指针类型得到该类型空指针，任意两个空指针相等 */
    void *p_null_void = (int *)0; /* 空指针转 void* */
    int *p_null_int = p_null_void; /* 空指针转 int* */
    assert(p_null_int == p_null_void);
    assert(p_null_int == p_null1);
    assert(p_null1 == p_null2);

    /* [5] 整数转指针类型 (实现定义) */
    /* 仅测试转换能正常发生并取回原值 */
    uintptr_t addr = (uintptr_t)&x;
    int *p_from_int = (int *)addr; /* cast 转换 */
    assert(*p_from_int == 42);

    /* [6] 指针转整数类型 (实现定义) */
    uintptr_t addr2 = (uintptr_t)p_int;
    assert(addr2 == (uintptr_t)&x);

    /* [7] 对象/不完整类型指针互转，转回应相等；转字符类型指向最低字节 */
    double d = 1.23;
    double *p_d = &d;
    char *p_c = (char *)p_d; /* 转字符类型 */
    assert(p_c == (char *)&d); /* 指向最低字节 */
    
    /* 递增指向剩余字节 */
    size_t size = sizeof(double);
    for (size_t i = 0; i < size; i++) {
        volatile char byte = p_c[i]; /* 访问连续字节 */
        (void)byte;
    }
    
    /* 转回原类型 */
    double *p_d_back = (double *)p_c;
    assert(p_d_back == p_d);

    /* [8] 函数指针互转，转回应相等 */
    FuncPtr1 fp1 = sample_func;
    FuncPtr2 fp2 = (FuncPtr2)fp1; /* 转为不同函数指针类型 */
    FuncPtr1 fp1_back = (FuncPtr1)fp2; /* 转回 */
    assert(fp1_back == fp1);
    /* 注意：不兼容调用是UB，此处不调用 fp2 */
}

int main(void) {
    test_6_3_2_3_semantics();
    printf("6.3.2.3 正向测试通过\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束：隐式将非0整数转为指针（需要cast，违反 6.5.16.1 简单赋值约束） */
void neg_test_1(void) {
    int x = 123;
    int *p = x; /* gcc -std=c99 应报错：assignment to ‘int *’ from ‘int’ makes pointer from integer without a cast */
}

/* 违反约束：隐式将指针转为整数（需要cast，违反 6.5.16.1 简单赋值约束） */
void neg_test_2(void) {
    int x = 0;
    int *p = &x;
    int i = p; /* gcc -std=c99 应报错：assignment to ‘int’ from ‘int *’ makes integer from pointer without a cast */
}

/* 违反约束：隐式将不兼容指针互转（int* 到 struct S*，违反 6.5.16.1 简单赋值约束） */
void neg_test_3(void) {
    int x = 0;
    int *p = &x;
    struct S { int a; } *ps;
    ps = p; /* gcc -std=c99 应报错：assignment to ‘struct S *’ from incompatible pointer type ‘int *’ */
}

/* 违反约束：隐式将函数指针转为对象指针，或反之（违反 6.5.16.1 简单赋值约束） */
void neg_test_4(void) {
    int x = 0;
    int *p = &x;
    void (*fp)(void) = (void (*)(void))p; /* 显式cast合法 */
    int *p2 = fp; /* 隐式转换，gcc -std=c99 应报错：assignment to ‘int *’ from incompatible pointer type ‘void (*)(void)’ */
}

/* 违反约束：隐式将 const int* 转为 int*（丢弃限定符，违反 6.5.16.1 简单赋值约束） */
void neg_test_5(void) {
    const int x = 0;
    const int *cp = &x;
    int *p = cp; /* gcc -std=c99 应报错：assignment to ‘int *’ from incompatible pointer type ‘const int *’ */
}
#endif