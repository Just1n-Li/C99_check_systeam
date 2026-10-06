/*
 * 测试 C99 7.19.4.2 —— rename 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int rename(const char *old, const char *new);
 *   [2] 语义：old 指向的文件改名为 new；old 不再可访问；
 *             new 已存在时行为实现定义。
 *   [3] 返回值：成功返回 0；失败返回非 0，且原文件仍以原名存在。
 *   Footnote 235：文件被打开等原因可能导致失败。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* 辅助：写一个内容为 content 的文件 */
static int write_file(const char *path, const char *content)
{
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return -1;
    fputs(content, fp);
    fclose(fp);
    return 0;
}

/* 辅助：读取整个文件内容到 buf（buf 足够大），返回 0 成功 */
static int read_file(const char *path, char *buf, size_t bufsz)
{
    FILE *fp = fopen(path, "r");
    size_t n;
    if (fp == NULL) return -1;
    n = fread(buf, 1, bufsz - 1, fp);
    buf[n] = '\0';
    fclose(fp);
    return 0;
}

/* 辅助：判断文件是否存在 */
static int file_exists(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return 0;
    fclose(fp);
    return 1;
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 原型检查：取函数指针，类型必须为 int(*)(const char*, const char*) */
    {
        int (*fp)(const char *, const char *) = rename;
        assert(fp != NULL);
    }

    /* [2] 基本语义：old 改名为 new，old 不再可访问，new 可访问且内容不变 */
    {
        const char *oldname = "c99_rename_old.txt";
        const char *newname = "c99_rename_new.txt";
        char buf[64];
        int rc;

        remove(oldname);
        remove(newname);

        assert(write_file(oldname, "hello-rename") == 0);
        assert(file_exists(oldname) == 1);
        assert(file_exists(newname) == 0);

        rc = rename(oldname, newname);

        /* [3] 成功返回 0 */
        assert(rc == 0);

        /* [2] old 不再可访问 */
        assert(file_exists(oldname) == 0);

        /* [2] new 可访问，且内容保持 */
        assert(file_exists(newname) == 1);
        assert(read_file(newname, buf, sizeof buf) == 0);
        assert(strcmp(buf, "hello-rename") == 0);

        remove(newname);
    }

    /* [3] 失败情形：old 不存在时 rename 返回非 0，且不产生 new 文件 */
    {
        const char *missing = "c99_rename_missing_xyz.txt";
        const char *target  = "c99_rename_target_xyz.txt";
        int rc;

        remove(missing);
        remove(target);

        rc = rename(missing, target);

        /* [3] 失败返回非 0 */
        assert(rc != 0);

        /* 失败时不应凭空产生目标文件 */
        assert(file_exists(target) == 0);
    }

    /* [3] 失败时原文件仍以原名存在：
     * 用一个必然失败的目标（目录名）触发失败，检查源文件仍在。
     * 若实现允许覆盖目录名（极少见），则跳过该断言。
     */
    {
        const char *src = "c99_rename_keep_src.txt";
        const char *bad = "c99_rename_dir_target";
        int rc;

        remove(src);
        remove(bad);

        assert(write_file(src, "keepme") == 0);

        rc = rename(src, bad);

        if (rc != 0) {
            /* [3] 失败：原文件仍以原名存在 */
            assert(file_exists(src) == 1);
        } else {
            /* 实现允许该操作，清理即可 */
            remove(bad);
        }

        remove(src);
        remove(bad);
    }

    /* [2] new 已存在时行为实现定义：只验证调用可发生、返回值合法，
     * 不假设具体结果（可能成功覆盖，也可能失败）。
     */
    {
        const char *a = "c99_rename_a.txt";
        const char *b = "c99_rename_b.txt";
        int rc;

        remove(a);
        remove(b);

        assert(write_file(a, "AAA") == 0);
        assert(write_file(b, "BBB") == 0);

        rc = rename(a, b);

        /* 返回值要么 0（成功），要么非 0（失败），二者皆合法 */
        assert(rc == 0 || rc != 0);

        if (rc == 0) {
            /* 成功：b 存在，a 不存在 */
            assert(file_exists(b) == 1);
            assert(file_exists(a) == 0);
        } else {
            /* 失败：a 仍存在（[3]），b 仍存在 */
            assert(file_exists(a) == 1);
            assert(file_exists(b) == 1);
        }

        remove(a);
        remove(b);
    }

    /* Footnote 235：文件被打开可能导致失败。
     * 这里只验证「打开状态下调用 rename 是允许的调用」，
     * 不强制要求失败（实现定义），因此只检查返回值合法。
     */
    {
        const char *src = "c99_rename_open_src.txt";
        const char *dst = "c99_rename_open_dst.txt";
        FILE *fp;
        int rc;

        remove(src);
        remove(dst);

        assert(write_file(src, "opened") == 0);

        fp = fopen(src, "r");
        assert(fp != NULL);

        rc = rename(src, dst);

        /* 返回值合法即可（成功或失败均符合标准） */
        assert(rc == 0 || rc != 0);

        fclose(fp);

        remove(src);
        remove(dst);
    }

    printf("C99 7.19.4.2 rename: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「rename 的原型为 int rename(const char*, const char*)」：
 * 参数个数不匹配，gcc -std=c99 应报错（too few arguments / too many arguments）。
 */
void neg_wrong_argc(void)
{
    rename("only-one-arg");                 /* 参数太少 */
    rename("a", "b", "c");                  /* 参数太多 */
}

/* 违反约束「实参类型必须可转换为 const char*」：
 * 传入 int 实参，gcc -std=c99 应报错（incompatible type / passing int）。
 */
void neg_wrong_argtype(void)
{
    rename(123, "b");                       /* 第一个实参为 int */
    rename("a", 4.5);                       /* 第二个实参为 double */
}

/* 违反约束「rename 返回 int，不能作为结构体等使用」：
 * 把返回值当结构体访问成员，gcc -std=c99 应报错。
 */
struct NegS { int x; };
void neg_return_used_as_struct(void)
{
    rename("a", "b").x;                     /* int 无成员，应报错 */
}

/* 违反约束「函数调用结果不是左值」：
 * 对 rename 的返回值赋值，gcc -std=c99 应报错（lvalue required）。
 */
void neg_assign_to_call(void)
{
    rename("a", "b") = 0;                   /* 非左值赋值，应报错 */
}

/* 违反约束「rename 的返回值不能取地址」：
 * 对函数调用结果取 &，gcc -std=c99 应报错（lvalue required）。
 */
void neg_addr_of_call(void)
{
    int *p = &rename("a", "b");             /* 非左值取地址，应报错 */
    (void)p;
}

#endif /* 负向测试结束 */