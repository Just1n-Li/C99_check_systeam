/*
 * 验证 C99 6.4.2.2 Predefined identifiers (__func__)
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] EXAMPLE：验证 __func__ 打印函数名 */
void myfunc(void) {
    printf("%s\n", __func__); /* 预期输出: myfunc */
    assert(strcmp(__func__, "myfunc") == 0);
}

/* [1] 验证 __func__ 在不同函数中值为对应的函数名 */
void another_func(void) {
    assert(strcmp(__func__, "another_func") == 0);
    assert(strcmp(__func__, "myfunc") != 0);
}

/* [1] 验证 __func__ 的类型为 const char[]（隐式声明 static const char __func__[]） */
void type_test(void) {
    const char *p = __func__; /* 可赋值给 const char*，说明元素类型为 const char */
    assert(strcmp(p, "type_test") == 0);
    /* 验证数组大小 = 函数名长度 + 1（终止空字符） */
    assert(sizeof(__func__) == strlen("type_test") + 1);
}

/* [2] 验证 __func__ 的编码：源字符集声明后转换为执行字符集（ASCII 场景验证） */
void encoding_test(void) {
    assert(strcmp(__func__, "encoding_test") == 0);
    assert(__func__[0] == 'e');
    assert(__func__[sizeof(__func__) - 1] == '\0');
}

int main(void) {
    myfunc();
    another_func();
    type_test();
    encoding_test();

    /* [1] main 函数中同样存在 __func__ */
    assert(strcmp(__func__, "main") == 0);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 说明：6.4.2.2 条款本身没有 Constraints 段落，但根据 [1] 的语义，
   __func__ 被隐式声明为 static const char __func__[]。
   因此对 __func__ 赋值或修改其元素，违反了赋值运算符的约束
   （6.5.16: 左操作数必须是可修改的左值；const 数组不可修改）。 */

void bad_func(void) {
    /* 违反约束：__func__ 是数组类型，不可赋值（期望编译报错） */
    __func__ = "other";

    /* 违反约束：__func__ 元素为 const char，不可修改（期望编译报错） */
    __func__[0] = 'x';
}

/* Footnote 61: 显式声明 __func__ 是未定义行为(UB)，不是约束违规，
   因此不作为负向测试（UB 代码仍能编译通过）。 */
#endif