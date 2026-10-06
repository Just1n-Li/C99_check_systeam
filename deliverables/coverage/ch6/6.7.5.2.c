/*
 * 验证 C99 6.7.5.2 Array declarators (数组声明符)
 * 预期行为：正向测试能编译并运行通过；负向测试违反约束，应编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [8] EXAMPLE 2: extern int y[] 声明为不完整类型，定义在别处 */
extern int y[];
int y[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/* [10] EXAMPLE 4: 文件作用域非VM类型 */
int B[100];

/* [4] 语义：* 只能用于函数原型作用域，声明完整类型 */
void func_vla_proto(int [*]);

/* [10] EXAMPLE 4: 函数原型作用域的VLA参数合法 */
void fvla(int m, int C[m][m]);

/* [10] EXAMPLE 4: 块作用域静态指针指向VLA合法 */
void fvla(int m, int C[m][m]) {
    /* [10] 块作用域VLA合法 */
    int D[m];
    /* [10] 静态块作用域指针指向VLA合法 */
    static int (*q)[100] = &B;
    /* [10] 块作用域typedef VLA合法 */
    typedef int VLA[m][m];
    
    D[0] = 0;
    C[0][0] = 0;
    assert(sizeof(D) == m * sizeof(int));
}

void test_semantics(void) {
    /* [7] EXAMPLE 1: float fa[11], *afp[17]; */
    float fa[11];
    float *afp[17];
    fa[0] = 1.0f;
    afp[0] = &fa[0];
    assert(fa[0] == 1.0f);
    assert(afp[0] == &fa[0]);

    /* [3] 语义：派生数组类型 */
    int arr[5] = {10, 20, 30, 40, 50};
    assert(arr[2] == 30);

    /* [4] 语义：无大小，数组类型为不完整类型 (通过extern验证) */
    assert(y[5] == 5);

    /* [4] 语义：整数常量表达式且元素类型大小已知，非VLA */
    int const_arr[10];
    assert(sizeof(const_arr) == 10 * sizeof(int));

    /* [4] & [5] 语义：非常量表达式构成VLA，生命周期内大小不变 */
    int n = 5;
    int vla[n];
    /* [5] 语义：sizeof操作数中的大小表达式如果不影响结果，是否求值未指定。
       这里我们测试VLA大小在生命周期内不变 */
    n = 10; /* 改变n不影响已创建的vla */
    assert(sizeof(vla) == 5 * sizeof(int));

    /* [6] 语义：数组类型兼容性 */
    int a[5];
    int (*p)[5] = &a; /* p 的类型与 &a 兼容 */
    assert(*p == a);

    /* [9] EXAMPLE 3: VLA兼容性规则 */
    int n2 = 6, m2 = 7;
    int c[n2][n2][6][m2];
    int (*r)[n2][n2][n2+1];
    /* r = c; // 标准说明这仅在 n2==6 且 m2==n2+1 时行为定义良好，这里仅测试能编译 */
    r = c; 
    assert(r == c);
}

int main(void) {
    test_semantics();
    printf("All positive tests passed!\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [1] 约束：大小表达式必须为整数类型 */
void test_non_integer_size(void) {
    double d = 1.0;
    int arr[d]; /* 违反约束：大小表达式不是整数类型 */
}

/* [1] 约束：常量表达式值必须大于0 */
void test_zero_size(void) {
    int arr[0]; /* 违反约束：常量表达式值不大于0 */
}

/* [1] 约束：元素类型不能是函数类型 */
void test_func_element(void) {
    typedef int FuncType(void);
    FuncType arr[5]; /* 违反约束：元素类型为函数类型 */
}

/* [1] 约束：元素类型不能是不完整类型 */
void test_incomplete_element(void) {
    struct Incomplete;
    struct Incomplete arr[5]; /* 违反约束：元素类型为不完整类型 */
}

/* [1] 约束：static 和类型限定符只能出现在函数参数的数组声明中 */
void test_static_non_param(void) {
    int arr[static 5]; /* 违反约束：static 出现在非函数参数声明中 */
}

/* [1] 约束：static 和类型限定符只能出现在最外层数组派生 */
void test_static_inner_array(int n) {
    void func(int arr[5][static 5]); /* 违反约束：static 不在最外层数组派生 */
}

/* [2] 约束：文件作用域不能有VLA类型 */
extern int n_vla;
int A[n_vla]; /* 违反约束：文件作用域VLA */

/* [2] 约束：文件作用域不能有VM类型 */
extern int (*p2)[n_vla]; /* 违反约束：文件作用域VM类型 */

/* [2] 约束：静态存储期对象不能是VLA */
void test_static_vla(int m) {
    static int E[m]; /* 违反约束：静态存储期VLA */
}

/* [2] 约束：有链接的标识符不能是VLA */
void test_extern_vla(int m) {
    extern int F[m]; /* 违反约束：F有链接且是VLA */
}

/* [2] 约束：有链接的标识符不能指向VLA */
void test_extern_vm_ptr(int m) {
    extern int (*r)[m]; /* 违反约束：r有链接且指向VLA */
}

/* [10] 约束：VM类型不能是结构体或联合的成员 */
void test_struct_vla_member(int n) {
    struct tag {
        int (*y)[n]; /* 违反约束：y不是普通标识符，是结构体成员 */
        int z[n];     /* 违反约束：z不是普通标识符，是结构体成员 */
    };
}

#endif