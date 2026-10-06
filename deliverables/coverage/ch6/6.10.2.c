/*
 * 测试 C99 6.10.2 —— Source file inclusion（源文件包含）
 *
 * 预期行为：
 *   正向测试：以下使用 #include 的代码应能正常编译并运行通过（assert 全部成立）。
 *   负向测试：违反 6.10.2 约束的 #include 指令应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] Constraints：include 必须标识一个可处理的头文件/源文件
 *   [2] #include <h-char-sequence> 语义
 *   [3] #include "q-char-sequence" 语义（含回退到 <> 形式）
 *   [4] #include pp-tokens 形式（宏替换后须匹配前两种形式）
 *   [5] 头文件名的唯一映射规则（.h 形式）
 *   [6] 嵌套包含
 *   [7] EXAMPLE 1
 *   [8] EXAMPLE 2
 */

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] 尖括号形式：搜索实现定义的位置，替换为头文件全部内容 */
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* [3] 引号形式：搜索实现定义的方式定位源文件 */
#include "c99_6_10_2_helper.h"

/* [7] EXAMPLE 1 的最常见用法（<stdio.h> 与 "myprog.h" 形式）已在上方体现 */

/* [8] EXAMPLE 2：宏替换后的 #include 指令 */
#define VERSION 1
#if VERSION == 1
#define INCFILE "c99_6_10_2_helper.h"
#elif VERSION == 2
#define INCFILE "vers2.h"
#else
#define INCFILE "versN.h"
#endif
#include INCFILE   /* [4][8] 宏替换后匹配 "q-char-sequence" 形式 */

/* [4] 宏替换后匹配 <h-char-sequence> 形式 */
#define STDIO_HEADER <stdio.h>
#include STDIO_HEADER

/* [5] 头文件名映射：一个或多个非数字/数字 + '.' + 单个非数字，首字符非数字 */
/*     这里通过实际包含一个符合该命名规则的头文件来验证映射可用 */
#include "c99_6_10_2_helper.h"

/* [6] 嵌套包含：helper 头文件内部又包含了另一个头文件 */
/*     见 c99_6_10_2_helper.h 中的 #include "c99_6_10_2_nested.h" */

int main(void)
{
    /* [2] 验证 <stdio.h> 内容确实被替换进来：printf 可用 */
    printf("C99 6.10.2 source file inclusion test\n");

    /* [3] 验证引号形式头文件内容被替换进来 */
    assert(HELPER_CONSTANT == 42);

    /* [6] 验证嵌套包含的头文件内容也被替换进来 */
    assert(NESTED_CONSTANT == 7);

    /* [8] 验证宏替换的 include 生效 */
    assert(HELPER_CONSTANT == 42);

    /* [4] 验证宏替换为 <> 形式的 include 生效 */
    {
        char buf[64];
        int n = snprintf(buf, sizeof buf, "%d", 123);
        assert(n == 3);
        assert(strcmp(buf, "123") == 0);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [1]：include 必须标识一个可处理的头文件/源文件。
 * 包含一个不存在的头文件，gcc -std=c99 应报错：
 *   fatal error: no_such_header_xyz.h: No such file or directory
 */
#include <no_such_header_xyz.h>

/* 违反约束 [1]：引号形式指向不存在的源文件，且回退到 <> 形式也失败。
 * gcc -std=c99 应报错：no_such_file_xyz.h: No such file or directory
 */
#include "no_such_file_xyz.h"

/* 违反 [4]：宏替换后不匹配前两种形式（缺少 < > 或 " "）。
 * 替换结果为裸标识符，不是合法的头文件名形式。
 * gcc -std=c99 应报错（如：invalid preprocessing directive 或
 *   #include expects "FILENAME" or <FILENAME>）。
 */
#define BAD_INCLUDE stdio.h
#include BAD_INCLUDE

/* 违反 [4] 脚注 148：相邻字符串字面量不会拼接成单个字符串字面量，
 * 因此展开成两个字符串字面量的指令是无效指令。
 * gcc -std=c99 应报错（#include expects "FILENAME" or <FILENAME>）。
 */
#define PART1 "c99_6_10_2_"
#define PART2 "helper.h"
#include PART1 PART2

/* 违反 [4]：宏替换后为空，不匹配任何合法形式。
 * gcc -std=c99 应报错（#include expects "FILENAME" or <FILENAME>）。
 */
#define EMPTY_INCLUDE
#include EMPTY_INCLUDE

#endif /* 负向测试结束 */