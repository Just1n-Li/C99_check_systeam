/*
 * 验证 C99 条款 6.2.2 Linkages of identifiers
 * 预期行为：正向测试运行通过，负向测试编译报错（本条款无显式约束，故无负向测试）。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] 文件作用域 static -> internal linkage */
static int s_int = 1;

/* [5] 文件作用域对象无存储类说明符 -> external linkage */
int g_int = 2;

/* [4] extern 在先前声明可见时，继承先前链接性 */
extern int s_int; /* 继承 internal linkage */
extern int g_int; /* 继承 external linkage */

/* [5] 函数无存储类说明符 -> external linkage */
int func_no_sc(void);

/* [2] external linkage: 整个程序中同一标识符指同一对象/函数 */
/* [2] internal linkage: 同一翻译单元内指同一对象/函数 */

int block_ext; /* 用于测试块作用域 extern */

void test_linkage(int param) { /* [6] 函数参数无链接 */
    /* [6] 块作用域对象无 extern -> 无链接 */
    int local = 10;
    /* [6] 非对象/函数标识符 -> 无链接 */
    typedef int MyInt;
    MyInt mi = 20;

    /* [2] 验证 external linkage 指向同一对象 */
    g_int = 50;
    assert(g_int == 50);

    /* [2] 验证 internal linkage 指向同一对象 */
    s_int = 60;
    assert(s_int == 60);

    /* [4] 块作用域 extern 无先前可见声明 -> external linkage */
    extern int block_ext;
    block_ext = 70;
    assert(block_ext == 70);

    /* [4] 块作用域 extern 有先前可见声明(internal) -> internal linkage */
    extern int s_int;
    s_int = 80;
    assert(s_int == 80);

    /* [5] 验证函数 external linkage */
    assert(func_no_sc() == 100);

    /* [1] 验证三种链接性存在 */
    /* external: g_int, func_no_sc */
    /* internal: s_int */
    /* none: local, param, mi */
}

int func_no_sc(void) {
    return 100;
}

/* [7] 如果同一标识符同时具有 internal 和 external linkage，行为是未定义的。
   由于是未定义行为(UB)而非约束，此处不进行测试。 */

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 本条款 6.2.2 仅包含 Semantics 段落，无 Constraints 段落。
   段落 [7] 描述的是未定义行为(UB)，而非约束。
   根据规则，UB 不作为负向测试，故此处无负向测试代码。 */
#endif

int main(void) {
    test_linkage(0);
    printf("6.2.2 test passed.\n");
    return 0;
}