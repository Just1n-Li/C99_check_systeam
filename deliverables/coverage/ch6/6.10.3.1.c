/*
 * 验证 C99 6.10.3.1 Argument substitution (参数替换)
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 参数替换前，参数内部的宏会被完全展开 */
#define INNER 42
#define ADD(x, y) (x + y)
void test_1_arg_expansion() {
    /* INNER 作为参数传入前，先被完全展开为 42，然后再替换宏体中的 x */
    int r = ADD(INNER, 0);
    assert(r == 42);
}

/* [1] 参数前有 #，则不展开参数内部的宏，直接进行字符串化 */
#define STR(x) #x
#define TOSTR(x) STR(x)
#define VAL 123
void test_1_stringize_no_expand() {
    /* STR(VAL) 中 VAL 前有 #，不展开，直接字符串化为 "VAL" */
    assert(strcmp(STR(VAL), "VAL") == 0);
    /* TOSTR(VAL) 中 VAL 无 # 或 ##，先展开为 123，再传入 STR，得到 "123" */
    assert(strcmp(TOSTR(VAL), "123") == 0);
}

/* [1] 参数前或后有 ##，则不展开参数内部的宏，直接进行 token 粘合 */
#define CONCAT(a, b) a ## b
#define PREFIX my
int myVar = 999;
void test_1_concat_no_expand() {
    /* PREFIX 后面有 ##，不展开；Var 前面有 ##，不展开。直接粘合成 myVar */
    assert(CONCAT(PREFIX, Var) == 999);
}

/* [2] __VA_ARGS__ 被视为参数，可变参数构成用于替换它的预处理 token */
#define VAR_ARGS(...) (__VA_ARGS__)
#define DEBUG(fmt, ...) printf(fmt, __VA_ARGS__)
void test_2_va_args() {
    /* __VA_ARGS__ 被替换为 10, 20, 30，构成逗号表达式 */
    int r = VAR_ARGS(10, 20, 30);
    assert(r == 30);
    
    /* __VA_ARGS__ 被替换为 42 */
    DEBUG("test %d\n", 42);
}

int main() {
    test_1_arg_expansion();
    test_1_stringize_no_expand();
    test_1_concat_no_expand();
    test_2_va_args();
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/*
 * 6.10.3.1 条款本身未定义直接的约束条件（Constraints）。
 * 但与 [2] 密切相关的约束（见 6.10.3.4p1）：__VA_ARGS__ 只能在使用省略号(...)的函数式宏替换列表中使用。
 * 违反此约束会导致编译报错。
 */
#define NO_ELLIPSIS() __VA_ARGS__
NO_ELLIPSIS()

#endif