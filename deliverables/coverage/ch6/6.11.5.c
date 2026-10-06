/*
 * 验证 C99 6.11.5: Storage-class specifiers (obsolescent feature)
 *
 * 条款 [1]: 在声明中，存储类说明符不放在声明说明符最开头是过时特性（obsolescent），
 *           但仍然是合法的 C99 代码，编译器应接受（可能发出警告）。
 *
 * 预期行为：
 * - 正向测试：存储类说明符放在非开头位置，代码应能编译并运行通过。
 * - 负向测试：本条款仅标记为 obsolescent feature，未定义任何约束（constraint），
 *             故无负向测试。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] static 放在类型说明符 int 之后（obsolescent 但合法） */
int static si = 42;

/* [1] extern 放在类型说明符 int 之后 */
int extern ei;

/* [1] 存储类说明符放在类型限定符 const 之后 */
const int static csi = 7;

/* [1] auto 放在类型说明符之后（函数内局部变量） */
void test_auto_placement(void) {
    int auto ai = 10;
    assert(ai == 10);
}

/* [1] register 放在类型说明符之后（函数内局部变量） */
void test_register_placement(void) {
    int register ri = 20;
    assert(ri == 20);
}

/* [1] static 放在类型限定符 const 之后（函数内） */
void test_const_static_placement(void) {
    const int static local_csi = 99;
    assert(local_csi == 99);
}

/* [1] extern 放在类型限定符 const 之后，用于函数原型声明 */
void test_extern_func(void);
const int extern (test_extern_func)(void) { return 55; }

/* ei 的定义 */
int ei = 100;

int main(void) {
    /* [1] 验证 static 非开头放置 */
    assert(si == 42);
    printf("[1] int static si = %d (OK)\n", si);

    /* [1] 验证 extern 非开头放置 */
    assert(ei == 100);
    printf("[1] int extern ei = %d (OK)\n", ei);

    /* [1] 验证 const + static 非开头放置 */
    assert(csi == 7);
    printf("[1] const int static csi = %d (OK)\n", csi);

    /* [1] 验证 auto 非开头放置 */
    test_auto_placement();
    printf("[1] int auto ai = 10 (OK)\n");

    /* [1] 验证 register 非开头放置 */
    test_register_placement();
    printf("[1] int register ri = 20 (OK)\n");

    /* [1] 验证 const + static 非开头放置（函数内） */
    test_const_static_placement();
    printf("[1] const int static local_csi = 99 (OK)\n");

    /* [1] 验证 extern 放在 const 之后用于函数定义 */
    assert(test_extern_func() == 55);
    printf("[1] const int extern func() = 55 (OK)\n");

    printf("\nAll positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
/*
 * 6.11.5 [1] 仅将存储类说明符非开头放置标记为 "obsolescent feature"（过时特性），
 * 并未将其列为约束。因此编译器必须接受此类代码（可发出警告），不应报错。
 *
 * 本条款无 Constraints 段落，故无负向测试。
 */
#if 0
/* 无约束可测试 —— 存储类说明符非开头放置是合法但过时的写法，编译器应接受 */
#endif