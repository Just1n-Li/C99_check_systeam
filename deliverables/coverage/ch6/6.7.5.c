/*
 * 验证 C99 条款 6.7.5 Declarators (声明符)
 * 预期行为：
 * - 正向测试：能编译并运行通过，assert 验证语义。
 * - 负向测试：违反语法规则，应编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] 语义：每个声明符声明一个标识符，并断言其指定函数或对象 */
int obj_id = 42;
int *ptr_obj = &obj_id;

/* [5] 语义：T D1 形式，D1 为 identifier，则类型为 T */
int x = 10;

/* [6] 语义：括号改变绑定。
   int (*func_ptr)(int) 是「指向返回 int 的函数的指针」 */
int sample_func(int a) { return a + 1; }
int (*func_ptr)(int) = sample_func;

/* [6] 语义：int *(func_ptr2)(int) 等价于 int *func_ptr2(int)，是「返回 int 指针的函数」 */
int *sample_func_ret_ptr(int *a) { return a; }
int *(func_ptr2)(int *) = sample_func_ret_ptr;

/* [3] 语义：完整声明符末尾是序列点；包含变长数组(VLA)的类型是可变修改的 */
void test_variably_modified(int n) {
    /* 完整声明符 vla 的末尾是序列点，n 在此前已求值 */
    int vla[n]; 
    /* 验证 VLA 大小正确 */
    assert(sizeof(vla) == (size_t)n * sizeof(int));
}

int main(void) {
    /* [2] 验证对象与指针的声明符语义 */
    assert(obj_id == 42);
    assert(*ptr_obj == 42);

    /* [5] 验证基本声明符 */
    assert(x == 10);

    /* [6] 验证括号改变绑定：func_ptr 是函数指针 */
    assert(func_ptr(5) == 6);
    /* [6] 验证括号不改变绑定：func_ptr2 是函数 */
    int val = 100;
    assert(*(func_ptr2(&val)) == 100);

    /* [3] 验证可变修改类型 */
    test_variably_modified(5);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反 Syntax [1]：声明符缺少标识符（非 abstract-declarator 场景） */
int ; /* gcc -std=c99 应报错：expected identifier or '(' */

/* 违反 Syntax [1]：指针声明符语法错误，* 必须在标识符前 */
int p *; /* gcc -std=c99 应报错：expected ';', identifier or '(' */

/* 违反 Syntax [1]：数组声明符语法错误，[] 必须在 direct-declarator 之后 */
int [10] arr; /* gcc -std=c99 应报错：expected ';', identifier or '(' */

/* 违反 Syntax [1]：函数声明符语法错误，() 必须在 direct-declarator 之后 */
int (void) func; /* gcc -std=c99 应报错：expected identifier or '(' */

/* 违反 Syntax [1]：parameter-type-list 语法错误，参数声明缺少声明符 */
int bad_func(int, ); /* gcc -std=c99 应报错：expected declaration specifiers or '...' */

#endif