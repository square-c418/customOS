// kernel.cpp - Main Operating System Entry and Interactive Processing Loop
#include "drivers.h"
#include "gui.h"

// Define Global Mouse Variables
int mouse_x = 40;  
int mouse_y = 12;
unsigned char mouse_cycle = 0;
char mouse_packet;
bool left_button_pressed = false;
int active_program = 0;
unsigned char canvas_buffer[80 * 25];

// Define Text Buffers
char text_editor_storage[256] = "Type letters here...";
int text_len = 20;

void process_mouse_input() {
    if ((inb(0x64) & 1) == 1) {
        unsigned char data = inb(0x60);
        if (mouse_cycle == 0) {
            if ((data & 0x08) == 0x08) { mouse_packet = data; mouse_cycle = 1; }
        } else if (mouse_cycle == 1) { mouse_packet = data; mouse_cycle = 2; } 
        else if (mouse_cycle == 2) {
            mouse_packet = data; mouse_cycle = 0;

            bool clicked = (mouse_packet & 0x01);
            int move_x = (int)mouse_packet; if (mouse_packet & 0x10) move_x |= 0xFFFFFF00;
            int move_y = (int)mouse_packet; if (mouse_packet & 0x20) move_y |= 0xFFFFFF00;

            mouse_x += (move_x / 2); mouse_y -= (move_y / 2);
            if (mouse_x < 0) mouse_x = 0; if (mouse_x >= SCREEN_WIDTH) mouse_x = SCREEN_WIDTH - 1;
            if (mouse_y < 0) mouse_y = 0; if (mouse_y >= SCREEN_HEIGHT) mouse_y = SCREEN_HEIGHT - 1;

            if (clicked) {
                left_button_pressed = true;
                if (mouse_y == SCREEN_HEIGHT - 1) {
                    if (is_mouse_hovering(2, SCREEN_HEIGHT - 1, 11, 1)) {
                        active_program = 0;
                        for(int i = 0; i < 80 * 25; i++) canvas_buffer[i] = 0x1F;
                    }
                    else if (is_mouse_hovering(16, SCREEN_HEIGHT - 1, 17, 1)) active_program = 1;
                    else if (is_mouse_hovering(36, SCREEN_HEIGHT - 1, 16, 1)) active_program = 2;
                } 
                else if (active_program == 2 && mouse_y > 0 && mouse_y < SCREEN_HEIGHT - 1) {
                    canvas_buffer[mouse_y * SCREEN_WIDTH + mouse_x] = 0x4F;
                }
            } else {
                left_button_pressed = false;
            }
        }
    }
}

char get_ascii_char(unsigned char scancode) {
    if (scancode == 0x1E) return 'a'; if (scancode == 0x30) return 'b'; if (scancode == 0x2E) return 'c';
    if (scancode == 0x20) return 'd'; if (scancode == 0x12) return 'e'; if (scancode == 0x21) return 'f';
    if (scancode == 0x22) return 'g'; if (scancode == 0x23) return 'h'; if (scancode == 0x17) return 'i';
    if (scancode == 0x24) return 'j'; if (scancode == 0x25) return 'k'; if (scancode == 0x26) return 'l';
    if (scancode == 0x32) return 'm'; if (scancode == 0x31) return 'n'; if (scancode == 0x18) return 'o';
    if (scancode == 0x19) return 'p'; if (scancode == 0x10) return 'q'; if (scancode == 0x13) return 'r';
    if (scancode == 0x1F) return 's'; if (scancode == 0x14) return 't'; if (scancode == 0x16) return 'u';
    if (scancode == 0x2F) return 'v'; if (scancode == 0x11) return 'w'; if (scancode == 0x2D) return 'x';
    if (scancode == 0x15) return 'y'; if (scancode == 0x2C) return 'z'; if (scancode == 0x39) return ' ';
    return 0;
}

extern "C" void kernel_main() {
    for (int i = 0; i < 80 * 25; i++) canvas_buffer[i] = 0x1F;
    init_mouse();
    render_desktop();

    unsigned char last_scancode = 0;
    int clock_tick_counter = 0;

    while (1) {
        process_mouse_input();

        if (active_program == 1) {
            if ((inb(0x64) & 1) == 1) {
                unsigned char scancode = inb(0x60);
                if (scancode != last_scancode && !(scancode & 0x80)) {
                    last_scancode = scancode;
                    char ascii = get_ascii_char(scancode);
                    if (ascii != 0 && text_len < 250) {
                        text_editor_storage[text_len++] = ascii;
                        text_editor_storage[text_len] = '\0';
                    }
                }
            }
        }

        clock_tick_counter++;
        if (clock_tick_counter > 30000) {
            clock_tick_counter = 0;
            render_desktop();
        }
        
        for (volatile int i = 0; i < 1000; i++);
    }
}
