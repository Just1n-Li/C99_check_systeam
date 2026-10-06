/*
 * 测试条款：ISO/IEC 9899:1999 (C99) 7.19.4.4 —— tmpnam 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明 char *tmpnam(char *s);
 *   [2] 生成合法文件名，且不与已存在文件同名；最多 TMP_MAX 个不同串
 *   [3] 每次调用生成不同的字符串
 *   [4] 实现行为如同没有库函数调用 tmpnam
 *   [5] 返回值语义：NULL 指针参数 -> 内部静态对象；非 NULL 参数 -> 写入数组并返回该参数
 *   [6] TMP_MAX >= 25
 *   脚注 236：生成的文件名需用 remove 删除
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 原型：char *tmpnam(char *s);  —— 通过取函数指针类型验证 */
    {
        char *(*fp)(char *) = tmpnam;
        assert(fp != NULL);
    }

    /* [6] TMP_MAX 至少为 25 */
    {
        assert(TMP_MAX >= 25);
    }

    /* [5] 参数为 NULL：结果留在内部静态对象中，返回指向该对象的指针 */
    {
        char *p1 = tmpnam(NULL);
        assert(p1 != NULL);                 /* 能生成则非空 */
        assert(p1[0] != '\0');              /* 是有效字符串 */
        /* 后续调用可能修改同一对象，因此这里只检查指针非空与内容非空 */
    }

    /* [5] 参数非 NULL：假定指向至少 L_tmpnam 个 char 的数组，
     *     函数把结果写入该数组，并返回该参数本身 */
    {
        char buf[L_tmpnam];
        char *ret = tmpnam(buf);
        assert(ret == buf);                 /* 返回值就是传入的参数 */
        assert(buf[0] != '\0');             /* 写入了有效文件名 */
        assert(strlen(buf) < L_tmpnam);     /* 未越界 */
    }

    /* [2] 生成的是合法文件名：不与已存在文件同名。
     *     这里用 fopen 以 "r" 打开，若成功说明文件已存在（不应发生）。 */
    {
        char buf[L_tmpnam];
        char *ret = tmpnam(buf);
        assert(ret == buf);
        FILE *f = fopen(buf, "r");
        assert(f == NULL);                  /* 该名字不应指向已存在的文件 */
        if (f != NULL) fclose(f);
    }

    /* [3] 每次调用生成不同的字符串（连续多次调用比较） */
    {
        char a[L_tmpnam], b[L_tmpnam];
        char *ra = tmpnam(a);
        char *rb = tmpnam(b);
        assert(ra == a && rb == b);
        assert(strcmp(a, b) != 0);          /* 两次结果不同 */
    }

    /* [3] 多次调用（含 NULL 形式）应产生不同字符串。
     *     注意：NULL 形式返回内部静态对象，需立即拷贝再比较。 */
    {
        char copy1[L_tmpnam], copy2[L_tmpnam];
        char *p1 = tmpnam(NULL);
        assert(p1 != NULL);
        strcpy(copy1, p1);                  /* 立即拷贝，避免被下次调用覆盖 */

        char *p2 = tmpnam(NULL);
        assert(p2 != NULL);
        strcpy(copy2, p2);

        assert(strcmp(copy1, copy2) != 0);  /* 不同 */
    }

    /* [2] 最多可生成 TMP_MAX 个不同字符串：这里只做有限次抽样，
     *     验证在若干次调用内都能得到互不相同的名字。 */
    {
        enum { N = 10 };
        char names[N][L_tmpnam];
        int i, j;
        for (i = 0; i < N; ++i) {
            char *r = tmpnam(names[i]);
            assert(r == names[i]);
            assert(names[i][0] != '\0');
        }
        for (i = 0; i < N; ++i) {
            for (j = i + 1; j < N; ++j) {
                assert(strcmp(names[i], names[j]) != 0);
            }
        }
    }

    /* [4] 实现行为如同没有库函数调用 tmpnam：
     *     即调用其它库函数不会“偷偷”消耗 tmpnam 的名字序列。
     *     这里做行为性检查：在两次 tmpnam 之间插入若干库函数调用，
     *     结果仍应互不相同且有效。 */
    {
        char a[L_tmpnam], b[L_tmpnam];
        char *ra = tmpnam(a);
        assert(ra == a);

        /* 调用一些无关的库函数 */
        {
            char tmp[64];
            sprintf(tmp, "%d", 12345);
            assert(strcmp(tmp, "12345") == 0);
            assert(strlen(tmp) == 5);
            (void)strchr(tmp, '3');
            (void)abs(-7);
        }

        char *rb = tmpnam(b);
        assert(rb == b);
        assert(strcmp(a, b) != 0);
    }

    /* 脚注 236：生成的名字对应的文件需用 remove 删除。
     * 这里创建一个临时文件并删除，验证 remove 可用（与 tmpnam 配合的用法）。 */
    {
        char buf[L_tmpnam];
        char *ret = tmpnam(buf);
        assert(ret == buf);

        FILE *f = fopen(buf, "w");
        assert(f != NULL);                  /* 应能创建该名字的文件 */
        if (f != NULL) {
            fputs("x", f);
            fclose(f);

            /* 现在该文件存在，tmpnam 不应再返回同名（此处仅验证 remove 流程） */
            int rc = remove(buf);
            assert(rc == 0);                /* 删除成功 */
        }
    }

    printf("All positive tests for C99 7.19.4.4 (tmpnam) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「tmpnam 的原型为 char *tmpnam(char *s);」：
 * 用不兼容的类型调用（参数个数/类型不符），gcc -std=c99 应报错。 */
void bad_call_arity(void)
{
    tmpnam();                 /* 参数个数不匹配：应报错 */
    tmpnam(NULL, NULL);       /* 参数个数过多：应报错 */
}

/* 违反约束「参数类型必须为 char *」：
 * 传入不兼容的指针类型（如 int *），应报错。 */
void bad_call_type(void)
{
    int x;
    int *ip = &x;
    tmpnam(ip);               /* int * 与 char * 不兼容：应报错 */
}

/* 违反约束「返回值类型为 char *」：
 * 把返回值赋给不兼容的指针类型，应报错。 */
void bad_return_type(void)
{
    int *p = tmpnam(NULL);    /* char * 赋给 int *：应报错 */
    (void)p;
}

/* 违反约束「函数返回非左值」：
 * tmpnam 的返回值是右值，不能对它赋值，应报错。 */
void bad_assign_to_call(void)
{
    tmpnam(NULL) = (char *)0; /* 对函数调用结果赋值：应报错 */
}

/* 违反约束「函数返回非左值」：
 * 对函数返回指针的解引用结果赋值虽合法（*p 是左值），
 * 但对函数调用本身取地址/赋值不合法，这里演示对返回值取地址：应报错。 */
void bad_addr_of_call(void)
{
    char **pp = &tmpnam(NULL); /* 对函数调用结果取地址：应报错 */
    (void)pp;
}

#endif /* 负向测试结束 */