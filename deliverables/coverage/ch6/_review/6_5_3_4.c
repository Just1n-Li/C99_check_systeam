/*
 * 验证 C99 6.5.3.4 The sizeof operator
 * 预期行为：正向测试运行通过，负向测试编译报错
 */
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

/* [5] EXAMPLE 1: 模拟存储分配函数 */
static void *alloc(size_t size) {
    static char buf[256];
    static size_t idx = 0;
    void *ptr = &buf[idx];
    idx += size;
    return ptr;
}

/* [7] EXAMPLE 3: 变长数组大小计算 */
size_t fsize3(int n) {
    char b[n+3]; /* variable length array */
    return sizeof b; /* execution time sizeof */
}

/* [88] 参数声明为数组类型，sizeof 产生调整后的指针类型大小 */
size_t param_array_size(int arr[10]) {
    return sizeof arr;
}

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [2] sizeof 产生操作数大小，结果为整数常量，且操作数不求值 */
    int counter = 0;
    size_t size_int = sizeof(counter++); /* counter++ 不应被执行 */
    assert(counter == 0);
    assert(size_int == sizeof(int));

    /* [2] 变长数组类型操作数会被求值，结果不是整数常量 */
    int n = 5;
    int vla[n];
    assert(sizeof vla == (size_t)n * sizeof(int));

    /* [3] char, unsigned char, signed char 及其限定版本结果为 1 */
    assert(sizeof(char) == 1);
    assert(sizeof(unsigned char) == 1);
    assert(sizeof(signed char) == 1);
    assert(sizeof(const char) == 1);
    assert(sizeof(volatile char) == 1);
    assert(sizeof(const volatile signed char) == 1);

    char c;
    const unsigned char cuc = 0;
    assert(sizeof c == 1);
    assert(sizeof cuc == 1);

    /* [3] 数组类型结果为总字节数 */
    int arr[10];
    assert(sizeof arr == 10 * sizeof(int));

    /* [3] 结构体和联合体包含内部和尾部填充 */
    struct S { char c; int i; } s;
    assert(sizeof(s) >= sizeof(char) + sizeof(int));
    
    union U { int i; double d; } u;
    assert(sizeof(u) >= sizeof(double));

    /* [4] 结果类型为 size_t (无符号整数类型) */
    size_t sz = sizeof(int);
    assert(sz > 0);

    /* [5] EXAMPLE 1: 与存储分配器通信 */
    double *dp = alloc(sizeof *dp);
    assert(dp != NULL);

    /* [6] EXAMPLE 2: 计算数组元素个数 */
    int array[] = {1, 2, 3, 4, 5};
    size_t num_elements = sizeof array / sizeof array[0];
    assert(num_elements == 5);

    /* [7] EXAMPLE 3: 变长数组大小计算 */
    size_t size = fsize3(10);
    assert(size == 13);

    /* [88] 参数声明为数组类型，sizeof 产生指针类型大小 */
    int arr_param[10];
    assert(param_array_size(arr_param) == sizeof(int*));

    printf("All positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* [1] 违反约束「sizeof 不能应用于函数类型的表达式」 */
    int func(int);
    sizeof(func); /* gcc -std=c99 应报错: invalid application of 'sizeof' to function type */

    /* [1] 违反约束「sizeof 不能应用于函数类型的带括号名称」 */
    typedef int FuncType(int);
    sizeof(FuncType); /* gcc -std=c99 应报错 */

    /* [1] 违反约束「sizeof 不能应用于不完整类型的表达式」 */
    struct Incomplete;
    struct Incomplete *p = 0;
    sizeof(*p); /* gcc -std=c99 应报错: invalid application of 'sizeof' to incomplete type */

    /* [1] 违反约束「sizeof 不能应用于不完整类型的带括号名称」 */
    struct Incomplete2;
    sizeof(struct Incomplete2); /* gcc -std=c99 应报错 */

    /* [1] 违反约束「sizeof 不能应用于位域成员」 */
    struct Bitfields { int a : 4; } bf;
    sizeof(bf.a); /* gcc -std=c99 应报错: 'sizeof' cannot be applied to a bit-field */
#endif
}