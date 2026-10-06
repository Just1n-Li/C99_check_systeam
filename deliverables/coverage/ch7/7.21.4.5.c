/*
 * 测试 C99 7.21.4.5 —— strxfrm 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <string.h>
 *   [2] 语义：变换、n 限制、n==0 时 s1 可为 NULL、重叠 UB（UB 不作负向测试）
 *   [3] 返回值：变换后字符串长度（不含 '\0'）；返回值 >= n 时 s1 内容不确定
 *   [4] EXAMPLE：1 + strxfrm(NULL, s, 0) 为所需数组大小
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：size_t strxfrm(char * restrict, const char * restrict, size_t) */
    {
        char buf[64];
        const char *src = "hello";
        size_t r = strxfrm(buf, src, sizeof buf);
        /* [3] 返回值是变换后字符串长度（不含 '\0'） */
        assert(r == strlen(buf));
        /* [2] 变换结果放入 s1，且以 '\0' 结尾 */
        assert(buf[r] == '\0');
    }

    /* [2] 变换保持与 strcoll 一致的比较顺序：
     *     对两个变换后的串用 strcmp，结果符号应与对原串用 strcoll 一致。 */
    {
        const char *a = "apple";
        const char *b = "banana";
        char ta[64], tb[64];
        size_t ra = strxfrm(ta, a, sizeof ta);
        size_t rb = strxfrm(tb, b, sizeof tb);
        assert(ra < sizeof ta && rb < sizeof tb);

        int c_orig = strcoll(a, b);
        int c_xfrm = strcmp(ta, tb);
        /* 符号一致：>0 / ==0 / <0 对应相同 */
        assert((c_orig > 0) == (c_xfrm > 0));
        assert((c_orig == 0) == (c_xfrm == 0));
        assert((c_orig < 0) == (c_xfrm < 0));
    }

    /* [2] 相等字符串：strcoll == 0 且 strcmp(变换后) == 0 */
    {
        const char *a = "same";
        const char *b = "same";
        char ta[64], tb[64];
        strxfrm(ta, a, sizeof ta);
        strxfrm(tb, b, sizeof tb);
        assert(strcoll(a, b) == 0);
        assert(strcmp(ta, tb) == 0);
    }

    /* [2] “No more than n characters are placed into s1, including '\0'”：
     *     n 足够大时完整写入；n 较小时最多写 n 个字符（含 '\0'）。 */
    {
        const char *src = "abcdef";
        char small[4];
        memset(small, 'X', sizeof small);
        size_t r = strxfrm(small, src, sizeof small);
        /* [3] 返回值是完整变换串长度，可能 >= n */
        assert(r == strlen(src)); /* 对 C locale 变换即原串 */
        /* 最多写入 n 个字符，且第 n 个位置（若写入）为 '\0' */
        assert(small[sizeof small - 1] == '\0');
        /* 前 n-1 个字符与源串前缀一致（C locale 下变换为恒等） */
        assert(strncmp(small, src, sizeof small - 1) == 0);
    }

    /* [2] n == 0 时 s1 允许为 NULL 指针（只查询所需长度） */
    {
        const char *src = "query length";
        size_t need = strxfrm(NULL, src, 0);
        /* [3] 返回完整变换串长度 */
        assert(need == strlen(src));
    }

    /* [4] EXAMPLE：1 + strxfrm(NULL, s, 0) 是容纳变换结果所需数组大小 */
    {
        const char *s = "example string";
        size_t sz = 1 + strxfrm(NULL, s, 0);
        char *arr = (char *)malloc(sz);
        assert(arr != NULL);
        size_t r = strxfrm(arr, s, sz);
        assert(r < sz);              /* 返回值 < n，内容确定 */
        assert(arr[r] == '\0');
        assert(strcmp(arr, s) == 0); /* C locale 下变换为恒等 */
        free(arr);
    }

    /* [2] n 恰好等于所需长度（含 '\0'）时，内容确定且完整 */
    {
        const char *src = "exact";
        size_t need = 1 + strxfrm(NULL, src, 0);
        char *arr = (char *)malloc(need);
        assert(arr != NULL);
        size_t r = strxfrm(arr, src, need);
        assert(r == need - 1);       /* r < n，内容确定 */
        assert(strcmp(arr, src) == 0);
        free(arr);
    }

    /* [3] 返回值 >= n 时 s1 内容不确定：这里只验证返回值语义，不检查 s1 内容 */
    {
        const char *src = "longer than buffer";
        char tiny[2];
        size_t r = strxfrm(tiny, src, sizeof tiny);
        assert(r == strlen(src));    /* 返回值是完整长度，>= n */
        assert(r >= sizeof tiny);
        /* 不检查 tiny 的内容：标准规定此时内容不确定 */
    }

    printf("All positive tests for C99 7.21.4.5 strxfrm passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「strxfrm 的第一个参数类型为 char *（restrict 限定）」：
     * 传入 const char * 会丢弃 const 限定，gcc -std=c99 应报错
     * （discards qualifiers / passing argument 1 discards 'const' qualifier）。 */
    {
        const char dst[16];
        const char *src = "x";
        strxfrm(dst, src, sizeof dst); /* 错误：dst 为 const char[]，不能作为 char* 实参 */
    }

    /* 违反约束「strxfrm 的第二个参数类型为 const char *」：
     * 传入非字符串指针（如 int*）类型不兼容，应报错。 */
    {
        char dst[16];
        int nums[4] = {0};
        strxfrm(dst, nums, sizeof dst); /* 错误：int* 与 const char* 不兼容 */
    }

    /* 违反约束「strxfrm 的第三个参数类型为 size_t」：
     * 传入指针类型无法隐式转换为 size_t，应报错。 */
    {
        char dst[16];
        const char *src = "x";
        int *p = 0;
        strxfrm(dst, src, p); /* 错误：int* 不能转换为 size_t */
    }

    /* 违反约束「strxfrm 返回 size_t」：
     * 将返回值赋给不兼容的指针类型，应报错。 */
    {
        char dst[16];
        const char *src = "x";
        int *bad = strxfrm(dst, src, sizeof dst); /* 错误：size_t 不能初始化 int* */
        (void)bad;
    }

    /* 违反约束「调用 strxfrm 需要可见的原型声明」：
     * 若未包含 <string.h> 且无自身声明，C99 不允许隐式函数声明，应报错
     * （implicit declaration of function 'strxfrm'）。 */
    {
        char dst[16];
        const char *src = "x";
        /* 假设此处 <string.h> 未包含：strxfrm(dst, src, sizeof dst); */
    }

#endif /* 负向测试结束 */
}