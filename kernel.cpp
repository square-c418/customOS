// kernel.cpp - Interactive OS with a Clickable Mouse GUI Button

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
volatile char* video_memory = (volatile char*)0xB8000;

// Mouse Tracking Variables
int mouse_x = 40;  
int mouse_y = 12;
unsigned char mouse_cycle = 0;
char mouse_packet[3];
bool left_button_pressed = false;

// OS App State
bool is_app_open = false;

// Draw a single character with specific colors anywhere on screen
void draw_cell(int x, int y, char c, char color_attribute) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        int index = (y * SCREEN_WIDTH + x) * 2;
        video_memory[index] = c;
        video_memory[index + 1] = color_attribute;
    }
}

// Fill a specific rectangle area with a background color
void draw_rect(int start_x, int start_y, int width, int height, char color) {
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) {
            draw_cell(x, y, ' ', color);
        }
    }
}

// Print text directly into our GUI layout
void draw_string(int x, int y, const char* str, char color) {
    for (int i = 0; str[i] != '\0'; i++) {
        draw_cell(x + i, y, str[i], color);
    }
}

// Create a simple bordered window
void draw_window(int start_x, int start_y, int width, int height, const char* title, char window_color, char border_color) {
    draw_rect(start_x, start_y, width, height, window_color);
    for (int x = start_x + 1; x < start_x + width - 1; x++) {
        draw_cell(x, start_y, (char)205, border_color);              
        draw_cell(x, start_y + height - 1, (char)205, border_color); 
    }
    for (int y = start_y + 1; y < start_y + height - 1; y++) {
        draw_cell(start_x, y, (char)186, border_color);             
        draw_cell(start_x + width - 1, y, (char)186, border_color); 
    }
    draw_cell(start_x, start_y, (char)201, border_color);                       
    draw_cell(start_x + width - 1, start_y, (char)187, border_color);             
    draw_cell(start_x, start_y + height - 1, (char)200, border_color);           
    draw_cell(start_x + width - 1, start_y + height - 1, (char)188, border_color); 

    if (title != nullptr) {
        draw_string(start_x + 3, start_y, title, border_color);
    }
}

// Check if mouse coordinates are inside a bounding box area
bool is_mouse_hovering(int x, int y, int w, int h) {
    return (mouse_x >= x && mouse_x < (x + w) && mouse_y >= y && mouse_y < (y + h));
}

// Main operational rendering sequence loop
void render_desktop() {
    // 1. Blue Desktop Wallpaper Base
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x1F); 

    // 2. Top Header Status Bar
    draw_rect(0, 0, SCREEN_WIDTH, 1, 0x70);
    draw_string(2, 0, " Chromebook MouseOS v1.3 ", 0x70);

    // 3. Bottom Taskbar layout panel (Light Gray background 0x70)
    draw_rect(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1, 0x70);
    
    // RENDER BUTTON: [ START ] Button bounds: X=1, Y=24, Width=9, Height=1
    // Changes to dark green (0x2F) when hovering, otherwise light slate (0x8F)
    char button_color = is_mouse_hovering(1, SCREEN_HEIGHT - 1, 9, 1) ? 0x2F : 0x8F;
    draw_rect(1, SCREEN_HEIGHT - 1, 9, 1, button_color);
    draw_string(2, SCREEN_HEIGHT - 1, "[ START ]", button_color);

    // Dynamic app window logic
    if (is_app_open) {
        draw_window(20, 6, 40, 10, "Click App Alpha", 0x4F, 0x4F); // Dark Red panel
        draw_string(23, 8, "Success! You clicked the button.", 0x4F);
        draw_string(23, 10, "Click [ START ] again to close.", 0x4F);
    } else {
        draw_string(15, 12, "Hover over [ START ] and click to execute.", 0x1E);
    }

    // RENDER THE MOUSE CURSOR LAST (Flashing white block indicator 'X' 0x0F)
    draw_cell(mouse_x, mouse_y, 'X', 0x0F); 
}

// Hardware I/O Ports
void outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void mouse_wait(unsigned char type) {
    unsigned int timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
    }
}

void mouse_write(unsigned char a_write) {
    mouse_wait(1); outb(0x64, 0xD4);
    mouse_wait(1); outb(0x60, a_write);
}

void init_mouse() {
    unsigned char status;
    mouse_wait(1); outb(0x64, 0xA8);
    mouse_wait(1); outb(0x64, 0x20);
    mouse_wait(0); status = (inb(0x60) | 2);
    mouse_wait(1); outb(0x64, 0x60);
    mouse_wait(1); outb(0x60, status);
    mouse_write(0xF6); inb(0x60);
    mouse_write(0xF4); inb(0x60);
}

// Scrape hardware port packets and check button clicks
void process_mouse_input() {
    if ((inb(0x64) & 1) == 1) {
        unsigned char data = inb(0x60);
        
        if (mouse_cycle == 0) {
            if ((data & 0x08) == 0x08) { 
                mouse_packet[0] = data;
                mouse_cycle = 1;
            }
        } else if (mouse_cycle == 1) {
            mouse_packet[1] = data; 
            mouse_cycle = 2;
        } else if (mouse_cycle == 2) {
            mouse_packet[2] = data; 
            mouse_cycle = 0;

            // Check if Left Mouse Button is actively down (Bit 0 of packet 0)
            bool clicked = (mouse_packet[0] & 0x01);

            // Handle horizontal and vertical movement math
            int move_x = (int)mouse_packet[1];
            if (mouse_packet[0] & 0x10) move_x |= 0xFFFFFF00;
            int move_y = (int)mouse_packet[2];
            if (mouse_packet[0] & 0x20) move_y |= 0xFFFFFF00;

            mouse_x += (move_x / 2);
            mouse_y -= (move_y / 2);

            // Bounds boundary clipping
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= SCREEN_WIDTH) mouse_x = SCREEN_WIDTH - 1;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= SCREEN_HEIGHT) mouse_y = SCREEN_HEIGHT - 1;

            // Trigger click action on button press state transition
            if (clicked && !left_button_pressed) {
                left_button_pressed = true;

                // BUTTON CLICK INTERSECTION TARGET
                // Check if cursor location overlaps the bounds of the [ START ] layout
                if (is_mouse_hovering(1, SCREEN_HEIGHT - 1, 9, 1)) {
                    is_app_open = !is_app_open; // Open or close window block
                }
            } else if (!clicked) {
                left_button_pressed = false; // Reset toggle flag when releasing trackpad
            }
        }
    }
}

extern "C" void kernel_main() {
    init_mouse();       
    render_desktop();   

    while (1) {
        int old_x = mouse_x;
        int old_y = mouse_y;
        bool old_app_state = is_app_open;

        process_mouse_input(); 

        // Rerender desktop frame if location changes or button gets triggered
        if (mouse_x != old_x || mouse_y != old_y || is_app_open != old_app_state) {
            render_desktop();
        }

        for (volatile int i = 0; i < 4000; i++);
    }
}
