// drivers.h - Hardware Input/Output, Mouse, and Motherboard Clock Drivers
#ifndef DRIVERS_H
#define DRIVERS_H

// Low-Level Hardware Assembly Port Communication Wrappers
inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// CMOS Real-Time Motherboard Clock Driver
inline unsigned char get_update_in_progress_flag() {
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

inline unsigned char get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

inline void read_rtc(int* hours, int* minutes) {
    while (get_update_in_progress_flag());
    *minutes = get_rtc_register(0x02);
    *hours = get_rtc_register(0x04);

    unsigned char registerB = get_rtc_register(0x0B);
    if (!(registerB & 0x04)) {
        *minutes = (*minutes & 0x0F) + ((*minutes / 16) * 10);
        *hours = (*hours & 0x0F) + (((*hours & 0x70) / 16) * 10) | (*hours & 0x80);
    }
}

// Global Mouse State Variables
extern int mouse_x;
extern int mouse_y;
extern unsigned char mouse_cycle;
extern char mouse_packet;
extern bool left_button_pressed;
extern int active_program;
extern unsigned char canvas_buffer[80 * 25];

inline void mouse_wait(unsigned char type) {
    unsigned int timeout = 100000;
    if (type == 0) { while (timeout--) { if ((inb(0x64) & 1) == 1) return; } } 
    else { while (timeout--) { if ((inb(0x64) & 2) == 0) return; } }
}

inline void mouse_write(unsigned char a_write) {
    mouse_wait(1); outb(0x64, 0xD4);
    mouse_wait(1); outb(0x60, a_write);
}

inline void init_mouse() {
    unsigned char status;
    mouse_wait(1); outb(0x64, 0xA8);
    mouse_wait(1); outb(0x64, 0x20);
    mouse_wait(0); status = (inb(0x60) | 2);
    mouse_wait(1); outb(0x64, 0x60);
    mouse_wait(1); outb(0x60, status);
    mouse_write(0xF6); inb(0x60);
    mouse_write(0xF4); inb(0x60);
}

#endif
