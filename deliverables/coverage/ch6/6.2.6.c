/*
 * 验证 C99 条款 6.2.6 (Representations of types)
 * 预期行为：
 * 正向测试：应能编译并运行通过，所有 assert 成立。
 * 负向测试：违反约束，应编译报错。
 */

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [6.2.6.1-2] 对象由 CHAR_BIT 位的 sizeof(type) 个字节组成 */
    assert(sizeof(unsigned char) * CHAR_BIT == CHAR_BIT);
    assert(sizeof(int) * CHAR_BIT >= 16); /* C99 保证 int 至少 16 位 */

    /* [6.2.6.1-5] 通过字符类型 (unsigned char) 访问对象不会产生陷阱表示 */
    int val = 12345;
    unsigned char *p = (unsigned char *)&val;
    unsigned char checksum = 0;
    for (size_t i = 0; i < sizeof(int); i++) {
        checksum += p[i]; /* 读取任意对象的字节表示是安全的 */
    }
    assert(checksum > 0);

    /* [6.2.6.1-6] 使用 memcpy 复制位模式 */
    int src = -42;
    int dst = 0;
    memcpy(&dst, &src, sizeof(int));
    assert(dst == -42); /* 复制后值应相同 */

    /* [6.2.6.1-7] 联合体存储值后，对象表示被创建为该成员的表示 */
    union { int i; unsigned char c[sizeof(int)]; } u;
    u.i = 0;
    /* 覆盖联合体的字节表示 */
    memset(u.c, 0xAA, sizeof(u));
    /* 此时 u.i 的对象表示已被修改为 0xAA 填充，读取 u.i 可能是陷阱表示或特定值，
       但对象表示确实变为了 c 的表示。这里仅验证 c 的值被正确写入 */
    assert(u.c[0] == 0xAA);

    /* [6.2.6.2-1] 无符号整数类型使用纯二进制表示法 */
    unsigned int u_max = 0;
    u_max = ~u_max; /* 按位取反 */
    assert(u_max == UINT_MAX); /* 纯二进制下，全1即为最大值 */
    unsigned int u_zero = 0;
    for (size_t i = 0; i < sizeof(unsigned int) * CHAR_BIT; i++) {
        u_zero = (u_zero << 1) | 1u; /* 逐位置1 */
    }
    assert(u_zero == UINT_MAX);

    /* [6.2.6.2-2, 6.2.6.2-3] 符号整数类型的符号位 */
    int pos = 1;
    int neg = -1;
    assert(pos > 0);  /* 符号位为0表示非负 */
    assert(neg < 0);  /* 符号位为1表示负 */

    printf("6.2.6 All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [6.2.6.1-1] 约束：除了位域外，对象由连续字节序列组成。
   位域不是可独立寻址的对象（没有完整的字节表示），因此不能对位域使用 sizeof 或取地址。
   违反此约束（对应 6.5.3.4 和 6.5.3.2 的约束），gcc -std=c99 应报错 */
struct S {
    int a : 4;
};
struct S s;
int sz = sizeof(s.a);  /* 约束违反：不能对位域使用 sizeof */
int *p = &s.a;         /* 约束违反：不能对位域取地址 */

/* [6.2.6.1-2] 约束：对象表示必须有确定的字节数。
   不完整类型（如未定义的数组或 void）没有已知的对象表示，不能用于创建对象。
   违反此约束（对应 6.7.5.2 的约束），gcc -std=c99 应报错 */
extern int incomplete_arr[];
int arr_sz = sizeof(incomplete_arr); /* 约束违反：不能对不完整类型使用 sizeof */
#endif