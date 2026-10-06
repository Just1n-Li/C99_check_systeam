/*
 * 验证 C99 条款 6.10.3.3 The ## operator
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [4] EXAMPLE: 展示 # ## # 生成新的 token "##"，但它不是 ## 操作符 */
#define hash_hash # ## #
#define mkstr(a) # a
#define in_between(a) mkstr(a)
#define join(c, d) in_between(c hash_hash d)

/* [2] 参数前后紧跟 ## */
#define CONCAT(a, b) a ## b

/* [2] 实参为空时，参数被替换为 placemarker preprocessing token */
#define CONCAT_EMPTY(a, b) a ## b

/* [3] 拼接后的 token 可用于进一步的宏替换 */
#define VAR1 100
#define MAKE_VAR(n) VAR ## n

/* [3] 对象式宏中的 ## */
#define OBJ_HASH(a) a ## _suffix

int main(void) {
    /* [4] EXAMPLE 测试 */
    char p[] = join(x, y); /* 相当于 char p[] = "x ## y"; */
    assert(strcmp(p, "x ## y") == 0);

    /* [2] 正常拼接 */
    int CONCAT(foo, bar) = 42;
    assert(foobar == 42);

    /* [2] 实参为空，placemarker 与非 placemarker 拼接 */
    int CONCAT_EMPTY(foo, ) = 10; /* foo ## placemarker -> foo */
    int CONCAT_EMPTY(, bar) = 20; /* placemarker ## bar -> bar */
    assert(foo == 10);
    assert(bar == 20);

    /* [3] 两个 placemarker 拼接结果为单个 placemarker，最终被删除 */
    int CONCAT_EMPTY(, )val = 30; /* placemarker ## placemarker -> placemarker -> int val = 30 */
    assert(val == 30);

    /* [3] 拼接结果进一步宏替换 */
    int w = MAKE_VAR(1); /* VAR ## 1 -> VAR1 -> 100 */
    assert(w == 100);

    /* [3] 对象式宏中的 ## */
    int OBJ_HASH(my) = 40; /* my ## _suffix -> my_suffix */
    assert(my_suffix == 40);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束「## 不能出现在替换列表的开头」：对象式宏 */
#define BAD_OBJ_START ## x

/* [1] 违反约束「## 不能出现在替换列表的结尾」：对象式宏 */
#define BAD_OBJ_END x ##

/* [1] 违反约束「## 不能出现在替换列表的开头」：函数式宏 */
#define BAD_FUNC_START(a) ## a

/* [1] 违反约束「## 不能出现在替换列表的结尾」：函数式宏 */
#define BAD_FUNC_END(a) a ##

/* 注：[3] 中提到“如果结果不是有效的预处理 token，行为是未定义的”，这属于 UB，不作为约束负向测试 */
#endif