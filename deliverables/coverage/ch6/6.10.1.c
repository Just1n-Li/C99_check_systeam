/*
 * 验证 C99 6.10.1 Conditional inclusion
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <assert.h>
#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 整型常量表达式与 defined 操作符的两种形式 */
#define DEF_MACRO 1
#if defined DEF_MACRO
  #define FORM1 1
#endif
#if defined(DEF_MACRO)
  #define FORM2 1
#endif

/* [1] 标识符（包括与关键字词法相同的标识符）的解释 */
#define int 1
#if int == 1
  #define KEYWORD_AS_ID 1
#endif
#undef int

/* [2] 宏替换后剩余 token 合法 */
#define VAL 10
#if VAL == 10
  #define MACRO_TOKEN_OK 1
#endif

/* [3] #if 和 #elif 检查控制常量表达式是否求值为非零 */
#if 1
  #define IF_TRUE 1
#endif
#if 0
  #define IF_FALSE 1
#else
  #define IF_FALSE 0
#endif

/* [4] 宏替换，剩余标识符替换为 0 */
#if VAL + UNDEFINED_ID == 10
  #define REPLACEMENT_OK 1
#endif

/* [4] 整型提升为 intmax_t/uintmax_t，-1 作为有符号类型小于 0 */
#if -1 < 0
  #define SIGNED_PROMOTION 1
#endif

/* [4] 在 #if 中，大常量被当作 intmax_t，0x80000000 在 32 位 int 机器上是正的 */
#if 0x80000000 > 0
  #define LARGE_CONST_SIGNED 1
#endif

/* [4] 字符常量在 #if 中求值 */
#if 'A' > 0
  #define CHAR_CONST_OK 1
#endif

/* [5] #ifdef 和 #ifndef 等价于 #if defined 和 #if !defined */
#ifdef DEF_MACRO
  #define TEST_IFDEF 1
#endif
#if defined DEF_MACRO
  #define TEST_IFDEF_EQ 1
#endif

#ifndef UNDEF_MACRO
  #define TEST_IFNDEF 1
#endif
#if !defined UNDEF_MACRO
  #define TEST_IFNDEF_EQ 1
#endif

/* [6] 条件顺序：第一个为真的组被处理 */
#if 0
  #define FIRST_GROUP 1
#elif 1
  #define FIRST_GROUP 2
#else
  #define FIRST_GROUP 3
#endif

/* [6] 如果没有条件为真，且有 #else，则处理 #else */
#if 0
  #define ELSE_GROUP 1
#elif 0
  #define ELSE_GROUP 2
#else
  #define ELSE_GROUP 3
#endif

/* [6] 如果没有条件为真，且无 #else，则跳过所有组直到 #endif */
#if 0
  #define NO_ELSE_GROUP 1
#elif 0
  #define NO_ELSE_GROUP 2
#endif

/* [6] 嵌套条件：即使内部条件为真，外部为假时整个组被跳过 */
#if 0
  #if 1
    #define NESTED_IN_FALSE 1
  #endif
#endif

int main(void) {
    /* [1] */
    assert(FORM1 == 1);
    assert(FORM2 == 1);
    assert(KEYWORD_AS_ID == 1);

    /* [2] */
    assert(MACRO_TOKEN_OK == 1);

    /* [3] */
    assert(IF_TRUE == 1);
    assert(IF_FALSE == 0);

    /* [4] */
    assert(REPLACEMENT_OK == 1);
    assert(SIGNED_PROMOTION == 1);
    assert(LARGE_CONST_SIGNED == 1);
    assert(CHAR_CONST_OK == 1);

    /* [5] */
    assert(TEST_IFDEF == TEST_IFDEF_EQ);
    assert(TEST_IFNDEF == TEST_IFNDEF_EQ);

    /* [6] */
    assert(FIRST_GROUP == 2);
    assert(ELSE_GROUP == 3);
    
    #ifndef NO_ELSE_GROUP
        /* 正确，未定义 */
    #else
        assert(0); /* NO_ELSE_GROUP 不应被定义 */
    #endif

    #ifndef NESTED_IN_FALSE
        /* 正确，未定义 */
    #else
        assert(0); /* NESTED_IN_FALSE 不应被定义 */
    #endif

    printf("All tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束「控制表达式必须是整型常量表达式且不能包含 cast」：包含 cast，gcc -std=c99 应报错 */
#if (int)1
#endif

/* [1] 违反约束「控制表达式必须是整型常量表达式」：包含浮点常量，gcc -std=c99 应报错 */
#if 1.0
#endif

/* [2] 违反约束「宏替换后剩余的预处理 token 必须在合法 token 的词法形式中」：未闭合的字符常量，gcc -std=c99 应报错 */
#define BAD_CHAR '
#if BAD_CHAR
#endif
#endif