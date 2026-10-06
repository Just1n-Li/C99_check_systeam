/*
 * 验证 C99 条款 6.11.9 Predefined macro names
 * 预期行为：
 * - 正向测试：运行通过，验证预定义的 __STDC_ 宏符合 C99 标准。
 * - 负向测试：编译报错或警告（注：本条款属于未来方向，标准未强制要求编译器报错，
 *   但定义保留宏属于未定义行为，严格编译器应予以拒绝或警告）。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
/* [1] 验证预定义的以 __STDC_ 开头的宏 */
int main(void) {
    /* __STDC_VERSION__ 应定义为 199901L (C99) */
    #ifdef __STDC_VERSION__
    assert(__STDC_VERSION__ == 199901L);
    #else
    #error "__STDC_VERSION__ should be defined in C99"
    #endif

    /* __STDC_HOSTED__ 应定义为 0 或 1 */
    #ifdef __STDC_HOSTED__
    assert(__STDC_HOSTED__ == 0 || __STDC_HOSTED__ == 1);
    #endif

    /* __STDC_IEC_559__ 如果定义，应为 1 */
    #ifdef __STDC_IEC_559__
    assert(__STDC_IEC_559__ == 1);
    #endif

    /* __STDC_IEC_559_COMPLEX__ 如果定义，应为 1 */
    #ifdef __STDC_IEC_559_COMPLEX__
    assert(__STDC_IEC_559_COMPLEX__ == 1);
    #endif

    printf("Predefined __STDC_ macros test passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反条款 6.11.9 [1]：宏名以 __STDC_ 开头被保留用于未来标准化。
   虽然 C99 将其归类为未来方向而非强制约束，但定义此类宏属于未定义行为，
   严格编译器应拒绝或发出警告。 */
#define __STDC_FUTURE_RESERVED_MACRO 1

/* 违反条款 6.11.9 [1]：不应取消定义预定义的 __STDC_ 宏 */
#undef __STDC_VERSION__
#endif