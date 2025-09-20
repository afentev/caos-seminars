uint32_t test1(uint32_t arg) {
    union {
        uint32_t arg;
        uint16_t arg2[2];
    }c = {.arg = arg};

    c.arg2[0] = 0x1;
    c.arg2[1] = 0x1;
    return (c.arg);
}

uint32_t test2(uint32_t arg) {
    unsigned char *ptr = &arg;
    memcpy(ptr, (uint16_t[]){1}, sizeof(uint16_t));
    memcpy(ptr + 2, (uint16_t[]){1}, sizeof(uint16_t));
    return arg;
}

uint32_t test3(uint32_t arg) {
    uint16_t* const sp = (uint16_t*)&arg;  // dereferencing type-punned pointer will break strict-aliasing rules (ub)
    uint16_t*       hi = &sp[0];
    uint16_t*       lo = &sp[1];

    *hi = 0x1;
    *lo = 0x1;

    return arg;
} 
