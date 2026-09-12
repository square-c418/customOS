// gui.h - Text Mode Window and Interface Rendering Engine
#ifndef GUI_H
#define GUI_H

#include "drivers.h"

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
volatile char* const video_memory = (volatile char*)0xB8000;

// Shared Text Buffer reference
extern char text_editor_storage[256];

inline void draw_cell(int x, int y, char c, char color_attribute) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        int index = (y * SCREEN_WIDTH + x) * 2;
        video_memory[index] = c;
        video_memory[index + 1] = color_attribute;
    }
}

inline void draw_rect(int start_x, int start_y, int width, int height, char color) {
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) {
            draw_cell(x, y, ' ', color);
        }
    }
}

inline void draw_string(int x, int y, const char* str, char color) {
    for (int i = 0; str[i] != '\0'; i++) {
        draw_cell(x + i, y, str[i], color);
    }
}

inline bool is_mouse_hovering(int x, int y, int w, int h) {
    return (mouse_x >= x && mouse_x < (x + w) && mouse_y >= y && mouse_y < (y + h));
}

inline void draw_window(int start_x, int start_y, int width, int height, const char* title, char win_color) {
    draw_rect(start_x, start_y, width, height, win_color);
    for (int x = start_x + 1; x < start_x + width - 1; x++) {
        draw_cell(x, start_y, (char)205, 0x70);              
        draw_cell(x, start_y + height - 1, (char)205, 0x70); 
    }
    for (int y = start_y + 1; y < start_y + height - 1; y++) {
        draw_cell(start_x, y, (char)186, 0x70);             
        draw_cell(start_x + width - 1, y, (char)186, 0x70); 
    }
    draw_cell(start_x, start_y, (char)201, 0x70);                       
    draw_cell(start_x + width - 1, start_y, (char)187, 0x70);             
    draw_cell(start_x, start_y + height - 1, (char)200, 0x70);           
    draw_cell(start_x + width - 1, start_y + height - 1, (char)188, 0x70); 
    draw_string(start_x + 2, start_y, title, 0x70);
}

inline void render_desktop() {
    for (int y = 1; y < SCREEN_HEIGHT - 1; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            draw_cell(x, y, ' ', canvas_buffer[y * SCREEN_WIDTH + x]);
        }
    }

    draw_rect(0, 0, SCREEN_WIDTH, 1, 0x70);
    draw_string(1, 0, "ChromebookOS GUI Engine", 0x70);

    int rtc_hours = 0, rtc_minutes = 0;
    read_rtc(&rtc_hours, &rtc_minutes);
    char time_str[] = {'0', '0', ':', '0', '0', '\0'};
    time_str[0] += (rtc_hours / 10);   time_str[1] += (rtc_hours % 10);
    time_str[3] += (rtc_minutes / 10); time_str[4] += (rtc_minutes % 10);
    draw_string(72, 0, time_str, 0x74);

    draw_rect(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1, 0x70);
    char col_c = (active_program == 0) ? 0x2F : 0x8F;
    char col_e = (active_program == 1) ? 0x2F : 0x8F;
    char col_p = (active_program == 2) ? 0x2F : 0x8F;

    draw_string(2, SCREEN_HEIGHT - 1, " [ Clear ] ", col_c);
    draw_string(16, SCREEN_HEIGHT - 1, " [ Text Editor ] ", col_e);
    draw_string(36, SCREEN_HEIGHT - 1, " [ Paint Tool ] ", col_p);

    if (active_program == 1) {
        draw_window(10, 4, 60, 14, " Notepad.exe ", 0x0F);
        draw_string(12, 6, text_editor_storage, 0x0F);
        draw_string(12, 16, "Click active app window panel area and type via keyboard...", 0x08);
    } 
    else if (active_program == 2) {
        draw_window(50, 2, 28, 4, " Paint Active ", 0x4F);
        draw_string(52, 4, "Drag mouse around background!", 0x4F);
    }

    draw_cell(mouse_x, mouse_y, 'X', 0x5F);
}

#endif
