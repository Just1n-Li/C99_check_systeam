/*
 * 验证 C99 条款 6.2 (Concepts)
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [6.2.1 Scopes of identifiers] / [6.2.3 Name spaces of identifiers] */
int label_var = 1;               /* 普通标识符，文件作用域 */
struct label_var { int x; };     /* 标签命名空间，与普通标识符不冲突 */

void test_scope_namespace(void) {
    int label_var = 2;           /* 块作用域，隐藏文件作用域的 label_var */
    assert(label_var == 2);

    struct label_var s;          /* 结构体标签，与普通标识符不冲突 */
    s.x = 3;
    assert(s.x == 3);

    goto label_var;              /* 标签名，与普通标识符和标签不冲突 */
label_var:
    ;
}

/* [6.2.2 Linkages of identifiers] */
static int internal_link = 10;   /* 内部链接 */
int external_link = 20;          /* 外部链接 */

void test_linkage(void) {
    extern int internal_link;    /* 引用内部链接 */
    extern int external_link;    /* 引用外部链接 */
    assert(internal_link == 10);
    assert(external_link == 20);
}

/* [6.2.4 Storage durations of objects] */
static int static_obj;           /* 静态存储期 */

void test_storage_duration(void) {
    int auto_obj = 5;            /* 自动存储期 */
    static int block_static = 5; /* 静态存储期 */
    assert(auto_obj == 5);
    assert(block_static == 5);
    block_static++;
    static_obj++;
}

/* [6.2.5 Types] */
void test_types(void) {
    /* const 和 volatile 独立性 [6.2.5/19] */
    const volatile int cv = 10;
    volatile int v = 20;
    const int c = 30;
    assert(cv == 10);
    assert(v == 20);
    assert(c == 30);

    /* const 限定符在成员访问上的传播 */
    struct S { int x; };
    const struct S s = {10};
    const int *p = &s.x;         /* s.x 继承了 const 限定 */
    assert(*p == 10);

    /* 函数指针类型 */
    typedef void (*func_ptr)(void);
    func_ptr fp = test_types;
    assert(fp != NULL);
}

/* [6.2.6 Representations of types] */
void test_representations(void) {
    /* unsigned char 占 1 字节，纯二进制表示 */
    assert(sizeof(unsigned char) == 1);
    unsigned char uc = 255;
    assert(uc == 255);
}

/* [6.2.7 Compatible type and composite type] */
int compatible_func(int a, int b) {
    return a + b;
}

void test_compatible_type(void) {
    /* 兼容的函数类型 */
    int (*fp)(int, int) = compatible_func;
    assert(fp(1, 2) == 3);

    /* 兼容的指针类型 */
    int arr[5] = {0, 1, 2, 3, 4};
    int (*p)[5] = &arr;
    assert((*p)[2] == 2);
}

int main(void) {
    test_scope_namespace();
    test_linkage();
    test_storage_duration();
    test_types();
    test_representations();
    test_compatible_type();
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [6.2.1 Constraints] 同一作用域内同名标识符在同一命名空间中 */
int same_name = 1;
int same_name = 2; /* 违反约束，同一作用域内同名标识符重复定义，gcc -std=c99 应报错 */

/* [6.2.2 Constraints] 内部链接标识符的外部声明不能包含定义 */
static int internal_def;
extern int internal_def = 10; /* 违反约束，内部链接标识符的外部声明包含定义，gcc -std=c99 应报错 */

/* [6.2.4 Constraints] 具有链接性的声明不能声明变长数组 */
int vla_n = 5;
extern int vla[vla_n]; /* 违反约束，变长数组不能具有链接性，gcc -std=c99 应报错 */

/* [6.2.5 Constraints] 函数不能返回数组或函数类型 */
int func_returning_array()[5]; /* 违反约束，函数返回数组类型，gcc -std=c99 应报错 */
int func_returning_func()(int); /* 违反约束，函数返回函数类型，gcc -std=c99 应报错 */

/* [6.2.5 Constraints] 函数类型不能被限定 */
typedef int FuncType(void);
const FuncType qualified_func; /* 违反约束，函数类型被 const 限定，gcc -std=c99 应报错 */

/* [6.2.7 Constraints] 函数声明符不能指定返回类型为函数或数组 */
typedef int func_type(int);
func_type f(void); /* 违反约束，返回函数类型，gcc -std=c99 应报错 */

#endif