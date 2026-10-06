/*
 * 测试目标：C99 7.20.8.2  wcstombs 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdlib.h>
 *   [2] 转换语义：宽字符序列 -> 多字节序列，从初始移位状态开始，
 *       遇到 n 字节上限或写入空字符即停止；每个宽字符如同调用 wctomb，
 *       但不影响 wctomb 的转换状态。
 *   [3] 最多修改 n 字节；重叠对象行为未定义（UB，不作为负向测试）。
 *   [4] 返回值：遇到无法转换的宽字符返回 (size_t)(-1)；
 *       否则返回写入的字节数（不含结尾空字符）。
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型与头文件：声明存在且签名匹配 */
static size_t (*fp_wcstombs)(char * restrict, const wchar_t * restrict, size_t)
    = wcstombs;

int main(void)
{
    /* 使用 C locale，保证宽字符到多字节的映射可预测（ASCII 子集） */
    setlocale(LC_ALL, "C");

    /* [1] 函数指针可用，签名正确 */
    assert(fp_wcstombs == wcstombs);

    /* ---------- [2][4] 基本转换：ASCII 宽字符 -> 单字节 ---------- */
    {
        const wchar_t src[] = L"Hello";
        char dst[16];
        memset(dst, 0x7f, sizeof dst);

        size_t r = wcstombs(dst, src, sizeof dst);
        /* [4] 返回写入字节数，不含结尾空字符 */
        assert(r == 5);
        /* [2] 内容正确，且写入了结尾空字符 */
        assert(strcmp(dst, "Hello") == 0);
        /* [3] 未超过 n 字节：第 6 字节是 '\0'，第 7 字节未被修改 */
        assert(dst[5] == '\0');
        assert((unsigned char)dst[6] == 0x7f);
    }

    /* ---------- [2][3] n 限制：截断到 n 字节，不写结尾空字符 ---------- */
    {
        const wchar_t src[] = L"Hello";
        char dst[16];
        memset(dst, 0x7f, sizeof dst);

        /* n = 3：最多写 3 字节，不写结尾空字符 */
        size_t r = wcstombs(dst, src, 3);
        assert(r == 3);
        assert(dst[0] == 'H' && dst[1] == 'e' && dst[2] == 'l');
        /* [3] 第 4 字节未被修改 */
        assert((unsigned char)dst[3] == 0x7f);
    }

    /* ---------- [2][3] n = 0：不修改任何字节 ---------- */
    {
        const wchar_t src[] = L"Hello";
        char dst[4] = { 'X', 'Y', 'Z', '\0' };

        size_t r = wcstombs(dst, src, 0);
        assert(r == 0);
        /* [3] 一个字节都没改 */
        assert(dst[0] == 'X' && dst[1] == 'Y' && dst[2] == 'Z');
    }

    /* ---------- [2] 遇到空宽字符即停止，并写入结尾空字符 ---------- */
    {
        const wchar_t src[] = L"AB\0CD";   /* 显式内嵌空宽字符 */
        char dst[16];
        memset(dst, 0x7f, sizeof dst);

        size_t r = wcstombs(dst, src, sizeof dst);
        /* [4] 返回 2，不含结尾空字符 */
        assert(r == 2);
        assert(dst[0] == 'A' && dst[1] == 'B');
        /* [2] 空字符被存储 */
        assert(dst[2] == '\0');
        /* 空字符之后不再转换 */
        assert((unsigned char)dst[3] == 0x7f);
    }

    /* ---------- [2] 空宽字符串：只写结尾空字符，返回 0 ---------- */
    {
        const wchar_t src[] = L"";
        char dst[4];
        memset(dst, 0x7f, sizeof dst);

        size_t r = wcstombs(dst, src, sizeof dst);
        assert(r == 0);
        assert(dst[0] == '\0');
        assert((unsigned char)dst[1] == 0x7f);
    }

    /* ---------- [2] 不影响 wctomb 的转换状态 ---------- */
    {
        /* 先让 wctomb 处于某个状态（C locale 下为初始状态） */
        char mb[MB_CUR_MAX];
        int st = wctomb(mb, L'A');
        assert(st == 1 && mb[0] == 'A');

        const wchar_t src[] = L"Z";
        char dst[8];
        size_t r = wcstombs(dst, src, sizeof dst);
        assert(r == 1 && dst[0] == 'Z');

        /* wctomb 状态未被 wcstombs 影响：仍可正常转换 */
        int st2 = wctomb(mb, L'B');
        assert(st2 == 1 && mb[0] == 'B');
    }

    /* ---------- [4] 无法转换的宽字符：返回 (size_t)(-1) ---------- */
    {
        /* 在 "C" locale 下，值超出可表示范围的宽字符无法转换。
         * 使用一个明显非法的宽字符值（如 0x110000，超出 Unicode 范围）。 */
        const wchar_t src[] = { (wchar_t)0x110000, L'A', L'\0' };
        char dst[16];
        memset(dst, 0x7f, sizeof dst);

        size_t r = wcstombs(dst, src, sizeof dst);
        /* [4] 返回 (size_t)(-1) */
        assert(r == (size_t)(-1));
    }

    /* ---------- [4] 部分转换后遇到非法字符，仍返回 (size_t)(-1) ---------- */
    {
        const wchar_t src[] = { L'A', (wchar_t)0x110000, L'\0' };
        char dst[16];
        memset(dst, 0x7f, sizeof dst);

        size_t r = wcstombs(dst, src, sizeof dst);
        assert(r == (size_t)(-1));
    }

    /* ---------- [2][3] 多字节字符（若 locale 支持）----------
     * 在 "C" locale 下 MB_CUR_MAX == 1，仅测试单字节路径。
     * 若环境支持 UTF-8 locale，可额外验证多字节转换；此处保持可移植。 */
    {
        assert(MB_CUR_MAX >= 1);
    }

    printf("wcstombs: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcstombs 的第一个参数类型为 char * restrict」：
 * 传入 const char * 会丢弃 const 限定，gcc -std=c99 应报错
 * （discards qualifiers / assignment from incompatible pointer type）。 */
{
    const wchar_t src[] = L"x";
    const char dst[8];
    wcstombs(dst, src, sizeof dst);   /* 期望报错：丢弃 const 限定 */
}

/* 违反约束「wcstombs 的第二个参数类型为 const wchar_t * restrict」：
 * 传入 char * 类型不匹配，gcc -std=c99 应报错。 */
{
    char dst[8];
    char src[8] = "x";
    wcstombs(dst, src, sizeof dst);   /* 期望报错：参数类型不兼容 */
}

/* 违反约束「wcstombs 的第三个参数类型为 size_t」：
 * 传入指针类型，gcc -std=c99 应报错。 */
{
    char dst[8];
    const wchar_t src[] = L"x";
    int *p = 0;
    wcstombs(dst, src, p);            /* 期望报错：参数类型不兼容 */
}

/* 违反约束「wcstombs 返回 size_t，调用需 3 个实参」：
 * 实参个数不足，gcc -std=c99 应报错。 */
{
    char dst[8];
    wcstombs(dst);                    /* 期望报错：实参太少 */
}

/* 违反约束「wcstombs 调用需 3 个实参」：
 * 实参个数过多，gcc -std=c99 应报错。 */
{
    char dst[8];
    const wchar_t src[] = L"x";
    wcstombs(dst, src, 8, 0);         /* 期望报错：实参太多 */
}

/* 违反约束「wcstombs 的 restrict 限定：s 与 pwcs 不得指向重叠对象」——
 * 注意：重叠本身是 UB（[3]），不是约束，故不在此处作为负向测试。
 * 这里仅演示「对非左值结果赋值」的通用约束（与 7.20.8.2 返回值相关）：
 * wcstombs 的返回值是右值，不能赋值。 */
{
    char dst[8];
    const wchar_t src[] = L"x";
    wcstombs(dst, src, 8) = 0;        /* 期望报错：赋值目标不是左值 */
}

#endif /* 负向测试结束 */