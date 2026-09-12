// kernel.cpp - Bare-metal Text-Mode GUI Layout Engine

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
volatile char* video_memory = (volatile char*)0xB8000;

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

// Create an interactive, bordered window element
void draw_window(int start_x, int start_y, int width, int height, const char* title, char window_color, char border_color) {
    // 1. Draw the inner window body panel
    draw_rect(start_x, start_y, width, height, window_color);

    // 2. Draw horizontal border lines
    for (int x = start_x + 1; x < start_x + width - 1; x++) {
        draw_cell(x, start_y, (char)205, border_color);              // Top double line '═'
        draw_cell(x, start_y + height - 1, (char)205, border_color); // Bottom double line '═'
    }

    // 3. Draw vertical border lines
    for (int y = start_y + 1; y < start_y + height - 1; y++) {
        draw_cell(start_x, y, (char)186, border_color);             // Left double line '║'
        draw_cell(start_x + width - 1, y, (char)186, border_color); // Right double line '║'
    }

    // 4. Connect the border corner pieces
    draw_cell(start_x, start_y, (char)201, border_color);                       // Top-Left '╔'
    draw_cell(start_x + width - 1, start_y, (char)187, border_color);             // Top-Right '╗'
    draw_cell(start_x, start_y + height - 1, (char)200, border_color);           // Bottom-Left '╚'
    draw_cell(start_x + width - 1, start_y + height - 1, (char)188, border_color); // Bottom-Right '╝'

    // 5. Render Title text header directly onto the top border layout line
    if (title[0] != '\0') {
        draw_cell(start_x + 2, start_y, ' ', border_color);
        draw_string(start_x + 3, start_y, title, border_color);
        // Find title length to pad right side cleanly
        int len = 0;
        while(title[len] != '\0') len++;
        draw_cell(start_x + 3 + len, start_y, ' ', border_color);
    }
}

// Main operational entry point
extern "C" void kernel_main() {
    // Desktop Background: Fill entire screen workspace canvas with safe Blue (0x1F)
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x1F);

    // Top Header Desktop Status Bar (Light Gray background, dark text 0x70)
    draw_rect(0, 0, SCREEN_WIDTH, 1, 0x70);
    draw_string(2, 0, " MyCustomOS v1.0 ", 0x70);
    draw_string(65, 0, " [System OK] ", 0x72); // Green highlight text status

    // Bottom Action Taskbar Layout Panel
    draw_rect(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1, 0x70);
    draw_string(2, SCREEN_HEIGHT - 1, "F1: Help | F2: Open App | F10: Shutdown", 0x74);

    // Render an independent window component center-stage
    // Parameters: X, Y, Width, Height, Title, BodyColor, BorderColor
    draw_window(15, 5, 50, 12, "System Status Center", 0x1E, 0x1E);

    // Render status details inside the active popup box layout frame
    draw_string(18, 7, "-> Installed Memory Block: 1024 KB [ONLINE]", 0x1E);
    draw_string(18, 9, "-> Virtual CPU Threads   : x86 Core [ACTIVE]", 0x1E);
    draw_string(18, 11, "-> Graphics Pipeline    : VGA TUI  [READY]", 0x1E);
    
    // Draw an inner nested actionable button frame item
    draw_window(32, 13, 16, 3, "", 0x70, 0x70);
    draw_string(36, 14, "OK CLOSE", 0x74);

    // Enter a secure hardware execution sleep safe loop state
    while (1) {
        for (volatile int i = 0; i < 50000; i++);
    }
}
