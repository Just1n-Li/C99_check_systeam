/*
 * 测试 C99 7.11.1.1 —— setlocale 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：[1] 原型、[2] 语义、[3] "C"/"" 语义、[4] 启动时默认 locale、
 *           [5] 库函数不调用 setlocale、[6] 成功/失败返回、[7] NULL 查询、
 *           [8] 返回值可恢复且不可修改。
 */

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：setlocale 的签名必须是
 *     char *setlocale(int category, const char *locale);
 *     通过函数指针赋值来静态验证返回类型与参数类型。 */
static char *(*fp_setlocale)(int, const char *) = setlocale;

int main(void)
{
    /* [1] 原型：返回 char*，参数为 (int, const char*) */
    assert(fp_setlocale == setlocale);

    /* [4] 程序启动时等价于执行 setlocale(LC_ALL, "C");
     *     因此查询当前 locale 应得到 "C"（或至少非 NULL）。 */
    {
        char *cur = setlocale(LC_ALL, NULL);   /* [7] NULL 查询 */
        assert(cur != NULL);
        /* 启动默认应为 "C" 环境 */
        assert(strcmp(cur, "C") == 0);
    }

    /* [2][3] 设置整个 locale 为 "C"（最小 C 翻译环境） */
    {
        char *r = setlocale(LC_ALL, "C");
        assert(r != NULL);
        /* [6] 成功时返回指向新 locale 关联字符串的指针 */
        assert(strcmp(r, "C") == 0);
    }

    /* [2] 分别设置各个 category 为 "C" */
    {
        char *r;
        r = setlocale(LC_COLLATE, "C");  assert(r != NULL);
        r = setlocale(LC_CTYPE,   "C");  assert(r != NULL);
        r = setlocale(LC_MONETARY,"C");  assert(r != NULL);
        r = setlocale(LC_NUMERIC, "C");  assert(r != NULL);
        r = setlocale(LC_TIME,    "C");  assert(r != NULL);
    }

    /* [7] NULL 查询：不改变 locale，返回当前 category 关联字符串 */
    {
        char *before = setlocale(LC_NUMERIC, NULL);
        assert(before != NULL);
        char *after = setlocale(LC_NUMERIC, NULL);
        assert(after != NULL);
        /* 两次查询结果应一致（locale 未被改变） */
        assert(strcmp(before, after) == 0);
    }

    /* [8] 返回值可用于恢复：保存返回值，改变 locale，再用保存值恢复 */
    {
        char *saved = setlocale(LC_ALL, NULL);
        assert(saved != NULL);
        /* 复制一份，因为返回的字符串可能被后续调用覆盖 */
        char saved_copy[64];
        strncpy(saved_copy, saved, sizeof saved_copy - 1);
        saved_copy[sizeof saved_copy - 1] = '\0';

        /* 改变 locale（仍用 "C"，但演示恢复流程） */
        char *r1 = setlocale(LC_ALL, "C");
        assert(r1 != NULL);

        /* [8] 用保存的字符串恢复该部分 locale */
        char *r2 = setlocale(LC_ALL, saved_copy);
        assert(r2 != NULL);
        /* 恢复后查询应与保存值一致 */
        char *now = setlocale(LC_ALL, NULL);
        assert(now != NULL);
        assert(strcmp(now, saved_copy) == 0);
    }

    /* [3] "" 指定本地环境：实现可能不支持，若返回 NULL 则 locale 不变；
     *     若返回非 NULL 则成功。两种情况都合法，只验证不崩溃且语义一致。 */
    {
        char *before = setlocale(LC_ALL, NULL);
        assert(before != NULL);
        char before_copy[64];
        strncpy(before_copy, before, sizeof before_copy - 1);
        before_copy[sizeof before_copy - 1] = '\0';

        char *r = setlocale(LC_ALL, "");
        if (r == NULL) {
            /* [6] 选择无法满足：返回 NULL 且 locale 不变 */
            char *after = setlocale(LC_ALL, NULL);
            assert(after != NULL);
            assert(strcmp(after, before_copy) == 0);
        } else {
            /* 成功：返回非 NULL */
            assert(r != NULL);
        }
        /* 恢复为 "C" 以便后续测试稳定 */
        assert(setlocale(LC_ALL, "C") != NULL);
    }

    /* [6] 无法满足的选择：传入一个几乎肯定无效的 locale 名，
     *     期望返回 NULL 且 locale 不变。 */
    {
        char *before = setlocale(LC_ALL, NULL);
        assert(before != NULL);
        char before_copy[64];
        strncpy(before_copy, before, sizeof before_copy - 1);
        before_copy[sizeof before_copy - 1] = '\0';

        char *r = setlocale(LC_ALL, "no_such_locale_xyz_12345");
        if (r == NULL) {
            /* [6] 失败：locale 不变 */
            char *after = setlocale(LC_ALL, NULL);
            assert(after != NULL);
            assert(strcmp(after, before_copy) == 0);
        }
        /* 若实现恰好支持该名字，则 r != NULL，也合法，不做断言 */
    }

    /* [2] LC_NUMERIC 影响格式化 I/O 的小数点字符。
     *     在 "C" locale 下小数点必须是 '.'。 */
    {
        assert(setlocale(LC_NUMERIC, "C") != NULL);
        char buf[32];
        snprintf(buf, sizeof buf, "%.1f", 3.5);
        assert(strcmp(buf, "3.5") == 0);
    }

    /* [5] 实现行为如同没有库函数调用 setlocale：
     *     即调用其他库函数不会隐式改变 locale。
     *     这里通过调用若干库函数后 locale 保持不变来间接验证。 */
    {
        char *before = setlocale(LC_ALL, NULL);
        assert(before != NULL);
        char before_copy[64];
        strncpy(before_copy, before, sizeof before_copy - 1);
        before_copy[sizeof before_copy - 1] = '\0';

        /* 调用一些可能受 locale 影响的库函数 */
        char b[64];
        snprintf(b, sizeof b, "%d", 42);
        (void)strcoll("a", "b");
        (void)strxfrm(b, "abc", sizeof b);

        char *after = setlocale(LC_ALL, NULL);
        assert(after != NULL);
        assert(strcmp(after, before_copy) == 0);
    }

    printf("All positive tests for C99 7.11.1.1 setlocale passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「setlocale 的第一个参数类型为 int」：
 * 传入结构体类型，gcc -std=c99 应报错（参数类型不兼容）。 */
struct Cat { int x; } cat;
setlocale(cat, "C");

/* 违反约束「setlocale 的第二个参数类型为 const char *」：
 * 传入 int，gcc -std=c99 应报错（参数类型不兼容）。 */
setlocale(LC_ALL, 123);

/* 违反约束「setlocale 返回 char *」：
 * 将其返回值赋给不兼容的指针类型（如 int*），
 * gcc -std=c99 应报错（赋值类型不兼容）。 */
int *p = setlocale(LC_ALL, "C");

/* 违反约束「setlocale 需要两个参数」：
 * 参数个数不匹配，gcc -std=c99 应报错。 */
setlocale(LC_ALL);

/* 违反约束「setlocale 需要两个参数」：
 * 参数过多，gcc -std=c99 应报错。 */
setlocale(LC_ALL, "C", "extra");

/* 违反约束「setlocale 返回 char *，不可对其返回值解引用赋值」：
 * 对函数返回值（非左值）赋值，gcc -std=c99 应报错。 */
setlocale(LC_ALL, "C")[0] = 'X';

/* 违反约束「setlocale 返回的字符串不得被程序修改」：
 * 通过返回指针写入，属于对 const 语义的违反（此处演示对返回指针的写操作，
 * 编译器可能不报错，但标准禁止；作为约束性说明保留）。 */
char *s = setlocale(LC_ALL, "C");
s[0] = 'X';

#endif