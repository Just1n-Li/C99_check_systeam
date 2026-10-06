/*
 * 测试 C99 7.20.3.3 —— malloc 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：void *malloc(size_t size);  需包含 <stdlib.h>
 *   [2] 分配 size 字节的对象空间，其值不确定（indeterminate）
 *   [3] 返回空指针或指向所分配空间的指针
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：包含 <stdlib.h> 后 malloc 可被调用，返回 void* */
    {
        void *p = malloc(16);
        assert(p != NULL);          /* [3] 要么返回空指针，要么返回有效指针 */
        free(p);
    }

    /* [1] 返回类型为 void*，可隐式转换为任意对象指针类型 */
    {
        int *ip = malloc(sizeof(int));   /* void* -> int* 隐式转换 */
        assert(ip != NULL);
        *ip = 42;
        assert(*ip == 42);
        free(ip);
    }

    /* [2] 分配的空间大小至少为请求的 size 字节，可安全写入 size 字节 */
    {
        size_t n = 100;
        unsigned char *buf = malloc(n);
        assert(buf != NULL);
        memset(buf, 0xAB, n);            /* 写入 n 字节，不应越界 */
        for (size_t i = 0; i < n; ++i)
            assert(buf[i] == 0xAB);
        free(buf);
    }

    /* [2] 分配对象的初始值不确定（indeterminate）——不能假设为 0。
     *     这里只验证“可读”，不验证具体值（值不确定，读取是允许的）。 */
    {
        int *arr = malloc(4 * sizeof(int));
        assert(arr != NULL);
        /* 不读取未初始化值做断言（其值不确定），仅确认指针有效 */
        arr[0] = 1; arr[1] = 2; arr[2] = 3; arr[3] = 4;
        assert(arr[0] + arr[1] + arr[2] + arr[3] == 10);
        free(arr);
    }

    /* [2] size 为 0 时：malloc(0) 的行为是实现定义的（可能返回 NULL 或唯一指针），
     *     但无论哪种，返回值都满足 [3]（空指针或有效指针），且若返回非空则可 free。 */
    {
        void *p = malloc(0);
        /* 两种结果都符合 [3]，不做强制断言，只验证可安全处理 */
        if (p != NULL) {
            free(p);
        }
    }

    /* [3] 返回值可赋给 void* 并用于比较；分配失败时返回空指针 */
    {
        void *p = malloc((size_t)-1);   /* 极大尺寸，通常分配失败 */
        /* 若失败则 p == NULL；若实现成功则 p != NULL。两者都符合 [3]。 */
        if (p != NULL) {
            free(p);
        }
    }

    /* [1][3] 多次调用返回不同（或可独立释放）的指针 */
    {
        void *a = malloc(8);
        void *b = malloc(8);
        assert(a != NULL && b != NULL);
        assert(a != b);                 /* 两次分配应得到不同对象 */
        free(a);
        free(b);
    }

    printf("正向测试全部通过。\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「[1] 原型要求参数为 size_t 类型」：
     * 传入结构体（非算术/指针可转换类型），gcc -std=c99 应报错。 */
    {
        struct S { int x; } s;
        void *p = malloc(s);            /* error: incompatible type for argument 1 */
        (void)p;
    }

    /* 违反约束「[1] 参数个数必须匹配」：
     * malloc 原型只接受 1 个参数，传 2 个应报错。 */
    {
        void *p = malloc(1, 2);         /* error: too many arguments to function 'malloc' */
        (void)p;
    }

    /* 违反约束「[1] 参数个数必须匹配」：
     * 不传参数应报错。 */
    {
        void *p = malloc();             /* error: too few arguments to function 'malloc' */
        (void)p;
    }

    /* 违反约束「[1] 返回类型为 void*，不能直接解引用」：
     * 对 void* 解引用应报错（void 是不完整类型）。 */
    {
        void *p = malloc(4);
        *p = 0;                         /* error: dereferencing 'void *' pointer */
        free(p);
    }

    /* 违反约束「[1] 返回类型为 void*，不能对其做算术运算」：
     * void* 上的指针算术应报错。 */
    {
        void *p = malloc(4);
        void *q = p + 1;                /* error: pointer arithmetic on 'void *' */
        (void)q;
        free(p);
    }

#endif

    return 0;
}