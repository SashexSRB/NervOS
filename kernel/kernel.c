// kernel.c
void kernel_main(void) {
    volatile char *vga = (volatile char *)0xB8000;
    const char *msg = "Hello from kernel!";
    for (int i = 0; msg[i]; i++) {
        vga[i * 2] = msg[i];
        vga[i * 2 + 1] = 0x07;
    }

    while (1) __asm__("hlt");
}
