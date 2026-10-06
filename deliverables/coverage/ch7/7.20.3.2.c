/*
 * 测试条款：C99 7.20.3.2 —— free 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：void free(void *ptr);  需要 <stdlib.h>
 *   [2] 语义：释放 ptr 指向的空间；ptr 为 NULL 时无动作；
 *             非 malloc/calloc/realloc 返回的指针、或已释放/已 realloc 的
 *             空间 → 未定义行为（UB，不作为负向测试）。
 *   [3] 返回值：free 无返回值（void）。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：free 的返回类型为 void，参数为 void *。
 *     通过取函数指针类型来静态验证原型签名。 */
static void check_prototype(void)
{
    /* 若 <stdlib.h> 中 free 的原型不是 void(void *)，此赋值将产生
     * 不兼容指针类型的诊断。 */
    void (*fp)(void *) = free;
    assert(fp != NULL);
    (void)fp;
}

/* [2] 基本语义：malloc 得到的空间可被 free 释放，释放后可再次分配。 */
static void test_basic_free(void)
{
    int *p = (int *)malloc(sizeof(int) * 4);
    assert(p != NULL);
    p[0] = 1; p[1] = 2; p[2] = 3; p[3] = 4;
    assert(p[0] == 1 && p[3] == 4);

    free(p);                 /* [2] 释放空间，使其可用于后续分配 */

    /* 释放后再次分配，验证空间“made available for further allocation” */
    int *q = (int *)malloc(sizeof(int) * 4);
    assert(q != NULL);
    q[0] = 42;
    assert(q[0] == 42);
    free(q);
}

/* [2] calloc 返回的指针同样可被 free 释放。 */
static void test_free_calloc(void)
{
    int *p = (int *)calloc(8, sizeof(int));
    assert(p != NULL);
    for (int i = 0; i < 8; i++)
        assert(p[i] == 0);   /* calloc 已清零 */
    free(p);
}

/* [2] realloc 返回的指针可被 free 释放。 */
static void test_free_realloc(void)
{
    int *p = (int *)malloc(sizeof(int) * 2);
    assert(p != NULL);
    p[0] = 10; p[1] = 20;

    int *r = (int *)realloc(p, sizeof(int) * 8);
    assert(r != NULL);
    assert(r[0] == 10 && r[1] == 20);
    free(r);
}

/* [2] ptr 为 NULL 时，free 不执行任何动作（no action occurs）。
 *     这是明确定义的行为，可安全多次调用。 */
static void test_free_null(void)
{
    free(NULL);   /* 无动作，不应崩溃 */
    free(NULL);   /* 再次调用仍无动作 */
    void *np = NULL;
    free(np);
}

/* [2] 释放后指针变量本身仍可被重新赋值（free 不修改实参变量）。 */
static void test_pointer_not_modified(void)
{
    int *p = (int *)malloc(sizeof(int));
    assert(p != NULL);
    int *saved = p;
    free(p);
    /* free 的形参是按值传递，调用者的指针变量值不变 */
    assert(p == saved);
    p = NULL;   /* 良好实践：置空避免悬垂 */
    assert(p == NULL);
}

/* [3] free 返回 void：不能把 free 的“结果”用于任何需要值的上下文。
 *     正向验证：free 调用可作为表达式语句，其类型为 void。 */
static void test_returns_void(void)
{
    void *p = malloc(16);
    assert(p != NULL);
    free(p);          /* 表达式语句，类型 void */
    /* 用逗号表达式确认 free 的类型是 void（void 表达式可作逗号左操作数） */
    (void)(free(NULL), 0);
}

int main(void)
{
    check_prototype();          /* [1] */
    test_basic_free();          /* [2] */
    test_free_calloc();         /* [2] */
    test_free_realloc();        /* [2] */
    test_free_null();           /* [2] */
    test_pointer_not_modified();/* [2] */
    test_returns_void();        /* [3] */

    printf("C99 7.20.3.2 free: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「free 的原型为 void free(void *)」：
 * 用不兼容的函数指针类型接收 free，gcc -std=c99 应报
 * "incompatible pointer type" 或类似诊断。 */
void (*bad_fp)(int) = free;

/* 违反约束「free 返回 void，无返回值」：
 * 试图把 free 的返回值赋给变量，void 值不能用于赋值，
 * gcc -std=c99 应报 "void value not ignored as it ought to be"。 */
int x = free(NULL);

/* 违反约束「free 返回 void」：
 * 试图对 free 的返回值做算术运算，void 不是算术类型，
 * gcc -std=c99 应报错。 */
int y = free(NULL) + 1;

/* 违反约束「free 返回 void」：
 * 试图把 free 的返回值作为函数实参传递（void 不能作为实参值），
 * gcc -std=c99 应报错。 */
void take_int(int);
void call_bad(void) { take_int(free(NULL)); }

/* 违反约束「free 的参数为 void *」：
 * 传入不兼容的指针类型（如函数指针）且无显式转换，
 * gcc -std=c99 应报 "incompatible pointer type" 警告/错误。 */
void bad_arg(void)
{
    void (*fn)(void) = 0;
    free(fn);   /* 函数指针与 void * 不兼容 */
}

/* 违反约束「free 的参数为 void *」：
 * 传入非指针类型（整数），gcc -std=c99 应报
 * "passing argument ... makes pointer from integer without a cast"。 */
void bad_arg2(void)
{
    free(42);
}

/* 违反约束「free 的参数为 void *」：
 * 传入结构体值（非指针），gcc -std=c99 应报类型不匹配。 */
struct S { int a; };
void bad_arg3(void)
{
    struct S s;
    free(s);
}

#endif /* 负向测试结束 */