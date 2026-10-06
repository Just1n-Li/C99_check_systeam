/*
 * 测试条款：C99 7.24.4.5.7  wcstok 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译（gcc -std=c99）并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器应报错（本文件因 #if 0 而不编译它们）。
 *
 * 覆盖段落：[1] 原型  [2] 描述  [3] 首次/后续调用  [4] 首个非分隔符  [5] 分隔符覆盖
 *           [6] ptr 保存状态  [7] 返回值  [8] EXAMPLE
 */

#include <wchar.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型一致 */
static wchar_t *(*fp_wcstok)(wchar_t * restrict, const wchar_t * restrict,
                             wchar_t ** restrict) = wcstok;

static int wcseq(const wchar_t *a, const wchar_t *b)
{
    return wcscmp(a, b) == 0;
}

int main(void)
{
    /* [1] 原型可用性 */
    assert(fp_wcstok == wcstok);

    /* [8] EXAMPLE 逐行验证 */
    {
        static wchar_t str1[] = L"?a???b,,,#c";
        static wchar_t str2[] = L"\t \t";
        wchar_t *t, *ptr1, *ptr2;

        /* [4][5][7] 首次调用：跳过前导 '?'，token 为 L"a" */
        t = wcstok(str1, L"?", &ptr1);
        assert(t != NULL);
        assert(wcseq(t, L"a"));

        /* [3][6] 后续调用：s1 为 NULL，分隔符改为 L","，token 为 L"??b" */
        t = wcstok(NULL, L",", &ptr1);
        assert(t != NULL);
        assert(wcseq(t, L"??b"));

        /* [4] 全是分隔符：无 token，返回 NULL */
        t = wcstok(str2, L" \t", &ptr2);
        assert(t == NULL);

        /* [3] 分隔符再次改变为 L"#,"，token 为 L"c" */
        t = wcstok(NULL, L"#,", &ptr1);
        assert(t != NULL);
        assert(wcseq(t, L"c"));

        /* [5][7] 已到串尾：后续搜索返回 NULL */
        t = wcstok(NULL, L"?", &ptr1);
        assert(t == NULL);
    }

    /* [2] 描述：把宽字符串拆成由 s2 中字符分隔的 token 序列 */
    {
        wchar_t s[] = L"alpha,beta,gamma";
        wchar_t *p;
        wchar_t *tok;

        tok = wcstok(s, L",", &p);
        assert(tok != NULL && wcseq(tok, L"alpha"));
        tok = wcstok(NULL, L",", &p);
        assert(tok != NULL && wcseq(tok, L"beta"));
        tok = wcstok(NULL, L",", &p);
        assert(tok != NULL && wcseq(tok, L"gamma"));
        tok = wcstok(NULL, L",", &p);
        assert(tok == NULL);
    }

    /* [5] 分隔符被覆盖为 L'\0'，token 就地终止 */
    {
        wchar_t s[] = L"one two";
        wchar_t *p;
        wchar_t *tok = wcstok(s, L" ", &p);
        assert(tok != NULL && wcseq(tok, L"one"));
        /* 原串中 ' ' 位置已被写为 L'\0' */
        assert(s[3] == L'\0');
        tok = wcstok(NULL, L" ", &p);
        assert(tok != NULL && wcseq(tok, L"two"));
        tok = wcstok(NULL, L" ", &p);
        assert(tok == NULL);
    }

    /* [3] 分隔符字符串可在每次调用时不同 */
    {
        wchar_t s[] = L"a;b,c";
        wchar_t *p;
        wchar_t *tok = wcstok(s, L";", &p);
        assert(tok != NULL && wcseq(tok, L"a"));
        tok = wcstok(NULL, L",", &p);   /* 分隔符改变 */
        assert(tok != NULL && wcseq(tok, L"b"));
        tok = wcstok(NULL, L",", &p);
        assert(tok != NULL && wcseq(tok, L"c"));
        tok = wcstok(NULL, L",", &p);
        assert(tok == NULL);
    }

    /* [4] 前导分隔符全部跳过，找到第一个非分隔符作为 token 起点 */
    {
        wchar_t s[] = L"***hello***world";
        wchar_t *p;
        wchar_t *tok = wcstok(s, L"*", &p);
        assert(tok != NULL && wcseq(tok, L"hello"));
        tok = wcstok(NULL, L"*", &p);
        assert(tok != NULL && wcseq(tok, L"world"));
        tok = wcstok(NULL, L"*", &p);
        assert(tok == NULL);
    }

    /* [4] 空串：无 token，返回 NULL */
    {
        wchar_t s[] = L"";
        wchar_t *p;
        wchar_t *tok = wcstok(s, L",", &p);
        assert(tok == NULL);
    }

    /* [6] ptr 保存继续扫描所需信息：连续调用可正确推进 */
    {
        wchar_t s[] = L"1|2|3|4";
        wchar_t *p;
        const wchar_t *expect[] = { L"1", L"2", L"3", L"4" };
        int i;
        wchar_t *tok = wcstok(s, L"|", &p);
        for (i = 0; i < 4; i++) {
            assert(tok != NULL && wcseq(tok, expect[i]));
            tok = wcstok(NULL, L"|", &p);
        }
        assert(tok == NULL);
    }

    /* [7] 返回值：指向 token 首字符的指针 */
    {
        wchar_t s[] = L"  xyz";
        wchar_t *p;
        wchar_t *tok = wcstok(s, L" ", &p);
        assert(tok != NULL);
        assert(tok == &s[2]);   /* 指向 token 首字符 */
        assert(*tok == L'x');
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcstok 原型要求 3 个参数」：参数个数不匹配，gcc -std=c99 应报错 */
void bad_argc(void)
{
    wchar_t s[] = L"a b";
    wchar_t *p;
    wcstok(s, L" ");            /* error: too few arguments to function 'wcstok' */
}

/* 违反约束「第 2 个参数类型为 const wchar_t *」：传入 int * 不兼容，应报错 */
void bad_arg2(void)
{
    wchar_t s[] = L"a b";
    int seps[2] = { ' ', 0 };
    wchar_t *p;
    wcstok(s, seps, &p);        /* error: incompatible pointer type */
}

/* 违反约束「第 3 个参数类型为 wchar_t **」：传入 wchar_t * 不兼容，应报错 */
void bad_arg3(void)
{
    wchar_t s[] = L"a b";
    wchar_t buf[8];
    wcstok(s, L" ", buf);       /* error: incompatible pointer type */
}

/* 违反约束「第 1 个参数类型为 wchar_t *」：传入 char * 不兼容，应报错 */
void bad_arg1(void)
{
    char s[] = "a b";
    wchar_t *p;
    wcstok(s, L" ", &p);        /* error: incompatible pointer type */
}

/* 违反约束「restrict 限定：s1 与 s2 不得指向同一对象」的静态可判情形：
   同一指针同时作为 s1 与 s2 传入，违反 restrict 语义（此处演示类型层面
   的 restrict 限定，编译器在 -Wrestrict 下可诊断） */
void bad_restrict(void)
{
    wchar_t s[] = L"a b";
    wchar_t *p;
    wcstok(s, s, &p);           /* warning/error: passing argument 2 discards 'restrict' */
}

#endif /* 负向测试结束 */