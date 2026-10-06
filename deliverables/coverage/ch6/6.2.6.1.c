/*
 * 验证 C99 条款 6.2.6.1 (General)
 * 预期行为：正向测试运行通过，负向测试编译报错（本条款无约束，故无负向测试代码）。
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 未指定表示：所有类型的表示未指定，除非本子条款说明。
       此处不直接测试，通过后续测试间接验证对象表示的可访问性。 */

    /* [2] 连续字节序列：对象由连续的一个或多个字节组成 */
    int x = 0x12345678;
    unsigned char *p = (unsigned char*)&x;
    for (size_t i = 0; i < sizeof(x); i++) {
        volatile unsigned char byte = p[i]; /* 遍历每个字节，验证连续性 */
    }

    /* [3] 纯二进制表示：unsigned char 和 unsigned bit-fields */
    unsigned char c = 0;
    for (int i = 0; i < CHAR_BIT; i++) {
        c = (unsigned char)(1U << i);
        assert(c == (1U << i)); /* 验证每一位的权重是 2^i */
    }
    struct Bits { unsigned int b : 3; };
    struct Bits b;
    b.b = 5;
    assert(b.b == 5); /* 验证无符号位段能正确存储纯二进制值 */

    /* [4] n x CHAR_BIT 位，对象表示，比较相等的值可能有不同对象表示 */
    assert(sizeof(int) * CHAR_BIT > 0);
    int a = 42;
    unsigned char rep[sizeof(a)];
    memcpy(rep, &a, sizeof(a)); /* 复制到 unsigned char[n] 得到对象表示 */

    /* 测试 +0.0 和 -0.0 比较相等，但对象表示可能不同 (Footnote 43) */
    double pos_zero = 0.0;
    double neg_zero = -0.0;
    assert(pos_zero == neg_zero);
    if (memcmp(&pos_zero, &neg_zero, sizeof(double)) != 0) {
        printf("[4] +0.0 and -0.0 compare equal but have different object representations.\n");
    } else {
        printf("[4] +0.0 and -0.0 have the same object representation on this platform.\n");
    }

    /* [5] 陷阱表示：自动变量可未初始化（可能含陷阱表示），只要不读取即无 UB (Footnote 41) */
    unsigned int trap_var; /* 未初始化，可能是陷阱表示 */
    /* 不读取 trap_var，避免 UB */

    /* [6] 结构填充字节取未指定值，结构对象本身非陷阱表示 */
    struct S { char c; int i; };
    struct S s = {1, 2};
    unsigned char *sp = (unsigned char*)&s;
    volatile unsigned char padding_byte = sp[1]; /* 可能是填充字节，读取合法，值未指定 */

    /* [7] 联合成员存储后，其他成员对应字节取未指定值 */
    union U { int i; float f; };
    union U u;
    u.i = 42;
    unsigned char *up = (unsigned char*)&u;
    volatile unsigned char other_byte = up[0]; /* 读取合法，值未指定 */

    /* [8] 多个对象表示不影响结果，不生成陷阱表示 */
    /* +0.0 和 -0.0 有不同表示，但加 1.0 结果相同 */
    assert(pos_zero + 1.0 == neg_zero + 1.0);

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* C99 6.2.6.1 条款没有定义程序员代码层面的约束（Constraints）。
       本条款中的 "shall" 均针对实现（如 [3] 要求 unsigned char 使用纯二进制表示），
       而非程序员代码。[5] 中的陷阱表示读取是未定义行为（UB），而非约束错误。
       因此，本条款无负向测试代码。 */
#endif

    return 0;
}