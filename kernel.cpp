// kernel.cpp - Simple Interactive Command Shell Engine

// VGA Layout constants
const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
volatile char* video_memory = (volatile char*)0xB8000;

// Tracker state variables 
int cursor_x = 0;
int cursor_y = 0;
char command_buffer[256];
int command_length = 0;

// Helper function to print a character to the next available slot
void print_char(char c, char color = 0x0F) { // 0x0F = High-contrast White Text
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        int index = (cursor_y * SCREEN_WIDTH + cursor_x) * 2;
        video_memory[index] = c;
        video_memory[index + 1] = color;
        cursor_x++;
    }

    // Wrap-around screen buffer logic
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
}

// Function to print whole strings easily
void print_string(const char* str, char color = 0x0F) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i], color);
    }
}

// Basic memory comparison function (Since standard strcmp doesn't exist)
bool compare_strings(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return false;
        i++;
    }
    return (s1[i] == '\0' && s2[i] == '\0');
}

// Clear screen function (Fills screen with blank entries)
void clear_screen() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
    cursor_x = 0;
    cursor_y = 0;
}

// Command Processing Center
void process_command() {
    print_char('\n');
    command_buffer[command_length] = '\0'; // Cap off string securely

    if (compare_strings(command_buffer, "help")) {
        print_string("Commands: help, clear, hello\n", 0x0E); // Yellow text
    } else if (compare_strings(command_buffer, "clear")) {
        clear_screen();
    } else if (compare_strings(command_buffer, "hello")) {
        print_string("Greetings, Custom OS User!\n", 0x0A); // Light Green text
    } else if (command_length > 0) {
        print_string("Unknown Command. Type 'help'\n", 0x0C); // Red text
    }

    // Reset prompt row
    print_string("OS_Shell> ", 0x0B); // Cyan prompt text
    command_length = 0;
}

// Minimal In-Browser Keyboard polling system 
// Reads direct legacy IBM PC port status without an official interrupt handler
char read_keyboard_port() {
    unsigned char result;
    // Direct Assembly 'in' call to check x86 hardware keyboard controller
    __asm__ volatile("inb %%dx, %%al" : "=a" (result) : "d" (0x60));
    return result;
}

// Main execution entry point
extern "C" void kernel_main() {
    clear_screen();
    print_string("Welcome to your Functional OS Shell!\n", 0x0E);
    print_string("Type 'help' to see active instructions.\n\n");
    print_string("OS_Shell> ", 0x0B);

    unsigned char last_scancode = 0;

    // Direct infinite hardware execution cycle loop
    while (1) {
        unsigned char scancode = read_keyboard_port();

        // Check if a key was just pressed down (ignores key-release codes)
        if (scancode != last_scancode && !(scancode & 0x80)) {
            last_scancode = scancode;

            // Basic translation map from hardware keyboard scan-codes to ASCII
            char pressed_char = 0;
            if (scancode == 0x1C) { // Enter key code
                process_command();
                continue;
            }
            else if (scancode == 0x10) pressed_char = 'q';
            else if (scancode == 0x1E) pressed_char = 'a';
            else if (scancode == 0x2C) pressed_char = 'z';
            else if (scancode == 0x23) pressed_char = 'h';
            else if (scancode == 0x12) pressed_char = 'e';
            else if (scancode == 0x26) pressed_char = 'l';
            else if (scancode == 0x18) pressed_char = 'o';
            else if (scancode == 0x2E) pressed_char = 'c';
            else if (scancode == 0x20) pressed_char = 'd';
            else if (scancode == 0x13) pressed_char = 'r';

            // Add valid inputs to console command string array buffer
            if (pressed_char != 0 && command_length < 255) {
                print_char(pressed_char);
                command_buffer[command_length++] = pressed_char;
            }
        }
        
        // Brief CPU relaxation loop instruction block 
        for (volatile int i = 0; i < 10000; i++);
    }
}
