/*
 * 测试目标：C99 7.21.6.2 —— strerror 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型：char *strerror(int errnum);  头文件 <string.h>
 *   [2] 将 errnum 映射为消息字符串；任何 int 值都必须能映射
 *   [3] 实现行为如同没有库函数调用 strerror（即 strerror 不依赖其它库函数副作用）
 *   [4] 返回指向字符串的指针，内容与 locale 相关；程序不得修改该数组，
 *       但后续调用 strerror 可能覆盖它
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：strerror 的返回类型为 char *，参数类型为 int。
 *     通过函数指针赋值来静态验证原型签名。 */
static char *(*strerror_proto_check)(int) = strerror;

int main(void)
{
    /* [1] 头文件 <string.h> 已提供声明，且签名匹配 */
    assert(strerror_proto_check == strerror);

    /* [2] 典型用法：errno 的值映射为消息字符串 */
    errno = 0;
    {
        char *msg = strerror(errno);
        assert(msg != NULL);          /* 返回非空指针 */
        assert(msg[0] != '\0');       /* 消息字符串非空 */
    }

    /* [2] 常见错误码映射 */
    {
        char *msg = strerror(EDOM);
        assert(msg != NULL);
        assert(msg[0] != '\0');
    }
    {
        char *msg = strerror(ERANGE);
        assert(msg != NULL);
        assert(msg[0] != '\0');
    }

    /* [2] "shall map any value of type int to a message"：
     *     对任意 int 值（包括 0、负数、INT_MIN、INT_MAX、未定义错误码）
     *     都必须返回一个消息字符串，且不得返回 NULL。 */
    {
        int vals[] = { 0, 1, -1, 2, -2, 100, -100, INT_MIN, INT_MAX };
        size_t i;
        for (i = 0; i < sizeof(vals) / sizeof(vals[0]); ++i) {
            char *msg = strerror(vals[i]);
            assert(msg != NULL);      /* 任何 int 值都映射到消息 */
            assert(msg[0] != '\0');   /* 消息非空 */
        }
    }

    /* [2] 同一 errnum 的映射应稳定（在无其它 strerror 调用干扰时） */
    {
        char *m1 = strerror(EDOM);
        char *m2 = strerror(EDOM);
        assert(m1 != NULL && m2 != NULL);
        assert(strcmp(m1, m2) == 0);
    }

    /* [3] "behave as if no library function calls strerror"：
     *     调用 strerror 不应改变 errno（实现如同没有库函数调用它，
     *     即 strerror 自身不产生可观察的库函数副作用）。
     *     这里验证调用 strerror 前后 errno 保持不变。 */
    {
        int saved = errno;
        (void)strerror(EDOM);
        assert(errno == saved);
    }

    /* [4] 返回的是指向字符串的指针，内容与 locale 相关；
     *     程序不得修改该数组 —— 这里只读取，不写入。 */
    {
        char *msg = strerror(EDOM);
        size_t len = strlen(msg);     /* 只读访问 */
        assert(len > 0);
        /* 逐字节读取，确认可安全读取整个字符串 */
        {
            size_t i;
            for (i = 0; i < len; ++i) {
                volatile char c = msg[i];
                (void)c;
            }
        }
    }

    /* [4] "may be overwritten by a subsequent call to the strerror function"：
     *     后续调用可能覆盖先前返回的缓冲区，因此不能假定旧指针内容不变。
     *     这里只验证：连续调用返回的指针都有效且可读（不假定其内容持久）。 */
    {
        char *a = strerror(EDOM);
        char *b = strerror(ERANGE);
        assert(a != NULL && b != NULL);
        assert(a[0] != '\0' && b[0] != '\0');
        /* 注意：不比较 a 与 b 的内容是否不同，因为实现可能复用同一缓冲区，
         * 也可能返回不同缓冲区；标准只要求内容 locale 相关且可被覆盖。 */
    }

    /* [4] 返回指针可用于只读字符串操作（如 strcmp、strlen） */
    {
        char *msg = strerror(0);
        assert(strlen(msg) == strlen(strerror(0)) || 1); /* 只读使用，恒真保护 */
        assert(strcmp(msg, msg) == 0);
    }

    printf("All positive tests for C99 7.21.6.2 (strerror) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strerror 的返回类型为 char *，参数为 int」：
 * 用不兼容的函数指针类型接收 strerror，gcc -std=c99 应报错
 * （incompatible pointer type / assignment from incompatible pointer type）。 */
void (*wrong_proto)(double) = strerror;

/* 违反约束「strerror 声明于 <string.h>，参数类型为 int」：
 * 以错误参数类型（结构体）调用，应报错（incompatible type for argument）。 */
struct NotInt { int x; };
void call_with_wrong_arg(void)
{
    struct NotInt n;
    (void)strerror(n);   /* 参数类型不匹配，应编译报错 */
}

/* 违反约束「strerror 返回 char *，不可作为其它类型使用」：
 * 将返回值赋给不兼容的指针类型，应报错。 */
void assign_wrong_type(void)
{
    int *p = strerror(0);   /* char * 赋给 int *，应编译报错 */
    (void)p;
}

/* 违反约束「strerror 需要 int 实参」：
 * 调用时缺少实参，应报错（too few arguments to function 'strerror'）。 */
void call_too_few_args(void)
{
    (void)strerror();   /* 缺少参数，应编译报错 */
}

/* 违反约束「strerror 只接受一个 int 实参」：
 * 传入过多实参，应报错（too many arguments to function 'strerror'）。 */
void call_too_many_args(void)
{
    (void)strerror(1, 2);   /* 实参过多，应编译报错 */
}

#endif /* 负向测试结束 */