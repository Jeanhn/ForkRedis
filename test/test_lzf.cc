#include <lzfse.h>
#include <cstring>
#include <cstdio>
#include <iostream>
int main()
{
    char hello[] = "lfkjdsakfjahdlkfahdslkjfdsalfkjdsakfjahdlkfahdslkjfdsalfkjdsakfjahdlkfahdslkjfdsalfkjdsakfjahdlkfahdslkjfdsalfkjdsakfjahdlkfahdslkjfdsalfkjdsakfjahdlkfahdslkjfdsa";
    uint8_t dst[200];
    auto s = lzfse_encode_buffer(dst, 200, reinterpret_cast<uint8_t *>(hello), 163, 0);
    std::cout << s << std::endl;

    char res[200];
    lzfse_decode_buffer(reinterpret_cast<uint8_t *>(res), 200, dst, 200, 0);
    printf("%d\n", strcmp(hello, res));
}