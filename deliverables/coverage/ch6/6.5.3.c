/*
 * 验证 C99 条款 6.5.3 Unary operators
 * 预期行为：正向测试运行通过，负向测试编译报错
 */
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    int a = 10;
    int *p;
    size_t sz;

    /* [1] postfix-expression 作为 unary-expression */
    a;

    /* [1] ++ unary-expression */
    ++a;
    assert(a == 11);

    /* [1] -- unary-expression */
    --a;
    assert(a == 10);

    /* [1] unary-operator cast-expression: & */
    p = &a;
    assert(*p == 10);

    /* [1] unary-operator cast-expression: * */
    *p = 20;
    assert(a == 20);

    /* [1] unary-operator cast-expression: + */
    a = +a;
    assert(a == 20);

    /* [1] unary-operator cast-expression: - */
    a = -a;
    assert(a == -20);

    /* [1] unary-operator cast-expression: ~ */
    a = ~a;
    assert(a == ~(-20));

    /* [1] unary-operator cast-expression: ! */
    a = !a;
    assert(a == 0);

    /* [1] sizeof unary-expression */
    sz = sizeof a;
    assert(sz == sizeof(int));

    /* [1] sizeof ( type-name ) */
    sz = sizeof(int);
    assert(sz == sizeof(int));

    /* const/volatile 限定类型在解引用上的传播 */
    const int *cp = &a;
    volatile int *vp = &a;
    /* *cp 的类型是 const int，*vp 的类型是 volatile int，读取合法 */
    int val = *cp + *vp;
    assert(val == 0);

    printf("6.5.3 正向测试通过\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束：++ 的操作数必须是可修改的左值，不能是算术表达式 */
int x = 0;
++(x + 1);

/* [1] 违反约束：-- 的操作数必须是可修改的左值，不能是常量 */
--5;

/* [1] 违反约束：& 的操作数不能是位域 */
struct S { int a:1; } s;
&s.a;

/* [1] 违反约束：* 的操作数必须是指针类型 */
int y = 0;
*y;

/* [1] 违反约束：~ 的操作数必须是整数类型 */
float f = 1.0f;
~f;

/* [1] 违反约束：sizeof 不能用于函数类型 */
int func(void);
sizeof(func);

/* [1] 违反语法：sizeof 后面如果是类型名，必须加括号 */
sizeof int;

/* [1] 违反约束：&a 的结果是非左值，不能对其赋值 */
int z = 0;
(&z) = (int*)0;
#endif