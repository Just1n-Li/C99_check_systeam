/*
 * 验证 C99 条款 6.10.8 (Predefined macro names)
 * 预期行为：
 * - 正向测试：能编译并运行通过，断言成功。
 * - 负向测试：违反约束，编译器应报错。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* [3] 记录文件开头的宏值，用于验证其值在整个翻译单元中保持不变 */
const long long initial_std = __STDC__;
const long long initial_ver = __STDC_VERSION__;
const long long initial_host = __STDC_HOSTED__;

/* [1] 验证 __STDC_MB_MIGHT_NEQ_WC__ 如果定义，必须为 1 */
#ifdef __STDC_MB_MIGHT_NEQ_WC__
    #if __STDC_MB_MIGHT_NEQ_WC__ != 1
        #error "预定义宏 __STDC_MB_MIGHT_NEQ_WC__ 的值必须为 1"
    #endif
#endif

/* [5] 验证实现不得预定义 __cplusplus */
#ifdef __cplusplus
    #error "实现不应预定义 __cplusplus"
#endif

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 验证 __DATE__ 格式："Mmm dd yyyy"，长度为 11 */
    assert(strlen(__DATE__) == 11);
    /* 月份名称与 asctime 相同，首字母大写 */
    assert(strchr("JFMAMJJASOND", __DATE__[0]) != NULL);
    /* dd 首字符为空格（如果小于10）或数字 */
    assert((__DATE__[4] == ' ' && __DATE__[5] >= '1' && __DATE__[5] <= '9') || 
           (isdigit(__DATE__[4]) && isdigit(__DATE__[5])));

    /* [1] 验证 __FILE__ 是字符串字面量 */
    const char *file = __FILE__;
    assert(file != NULL);
    assert(strlen(file) > 0);

    /* [1] 验证 __LINE__ 是整型常量，且随行号变化 */
    int line_num = __LINE__;
    assert(line_num > 0);
    int next_line = __LINE__;
    assert(next_line == line_num + 1);

    /* [1] 验证 __STDC__ 是整型常量 1 */
    assert(__STDC__ == 1);

    /* [1] 验证 __STDC_HOSTED__ 是 1（宿主）或 0（独立） */
    assert(__STDC_HOSTED__ == 0 || __STDC_HOSTED__ == 1);

    /* [1] 验证 __STDC_VERSION__ 是 199901L */
    assert(__STDC_VERSION__ == 199901L);

    /* [1] 验证 __TIME__ 格式："hh:mm:ss"，长度为 8 */
    assert(strlen(__TIME__) == 8);
    assert(__TIME__[2] == ':' && __TIME__[5] == ':');

    /* [2] 验证条件定义的宏：如果定义了，值必须符合要求 */
    #ifdef __STDC_IEC_559__
        assert(__STDC_IEC_559__ == 1);
    #endif

    #ifdef __STDC_IEC_559_COMPLEX__
        assert(__STDC_IEC_559_COMPLEX__ == 1);
    #endif

    #ifdef __STDC_ISO_10646__
        /* 验证形式为 yyyymmL，至少大于 199000L */
        assert(__STDC_ISO_10646__ >= 199000L);
    #endif

    /* [3] 验证预定义宏的值（除 __FILE__ 和 __LINE__）在整个翻译单元中保持不变 */
    assert(__STDC__ == initial_std);
    assert(__STDC_VERSION__ == initial_ver);
    assert(__STDC_HOSTED__ == initial_host);

    printf("6.10.8 正向测试全部通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [4] 违反约束「预定义宏名不得作为 #define 的对象」：对 __FILE__ 进行重定义，gcc -std=c99 应报错 */
#define __FILE__ "fake.c"

/* [4] 违反约束「预定义宏名不得作为 #undef 的对象」：取消 __LINE__ 的定义，gcc -std=c99 应报错 */
#undef __LINE__

/* [4] 违反约束「identifier defined 不得作为 #define 的对象」：重定义 defined 关键字，gcc -std=c99 应报错 */
#define defined(x) ((x) != 0)

/* [4] 违反约束「identifier defined 不得作为 #undef 的对象」：取消 defined 的定义，gcc -std=c99 应报错 */
#undef defined

/* [4] 违反约束「预定义宏名不得作为 #define 的对象」：对 __STDC__ 进行重定义，gcc -std=c99 应报错 */
#define __STDC__ 0

#endif