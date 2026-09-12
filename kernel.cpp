// kernel.cpp - Bare-metal GUI with an Interactive PS/2 Text Mode Mouse Driver

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
volatile char* video_memory = (volatile char*)0xB8000;

// Mouse Tracking Variables
int mouse_x = 40;  // Start cursor in the middle of the screen
int mouse_y = 12;
unsigned char mouse_cycle = 0;
char mouse_packet[3];

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

// Basic Hardware I/O Ports assembly wrappers
void outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Wait for the PS/2 controller to be ready to accept a command
void mouse_wait(unsigned char type) {
    unsigned int timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return; // Data is ready to read
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return; // Ready for a write command
        }
    }
}

// Write data directly to the mouse device inside the PS/2 controller
void mouse_write(unsigned char a_write) {
    mouse_wait(1);
    outb(0x64, 0xD4); // Tell controller to route next byte straight to the mouse
    mouse_wait(1);
    outb(0x60, a_write);
}

// Initialize and wake up the PS/2 Mouse hardware interface
void init_mouse() {
    unsigned char status;

    mouse_wait(1);
    outb(0x64, 0xA8); // Enable auxiliary mouse device port

    mouse_wait(1);
    outb(0x64, 0x20); // Command: Read current controller status byte
    mouse_wait(0);
    status = (inb(0x60) | 2); // Modify status to enable mouse interrupts

    mouse_wait(1);
    outb(0x64, 0x60); // Command: Write updated controller status byte
    mouse_wait(1);
    outb(0x60, status);

    mouse_write(0xF6); // Command: Tell mouse to load default configuration settings
    inb(0x60);         // Read acknowledgment byte back from mouse hardware

    mouse_write(0xF4); // Command: Enable packet streaming (mouse turns on!)
    inb(0x60);         // Read final acknowledgment byte
}

// Poll the mouse hardware port to see if a movement packet has arrived
void process_mouse_input() {
    // Check if the data bit on port 0x64 is active
    if ((inb(0x64) & 1) == 1) {
        unsigned char data = inb(0x60);
        
        // Step through the 3-byte mouse protocol packet
        if (mouse_cycle == 0) {
            if ((data & 0x08) == 0x08) { // Bit 3 must always be 1 for a valid 1st packet byte
                mouse_packet[0] = data;
                mouse_cycle = 1;
            }
        } else if (mouse_cycle == 1) {
            mouse_packet[1] = data; // X-axis movement relative delta byte
            mouse_cycle = 2;
        } else if (mouse_cycle == 2) {
            mouse_packet[2] = data; // Y-axis movement relative delta byte
            mouse_cycle = 0;

            // Handle horizontal X relative movement tracking
            int move_x = (int)mouse_packet[1];
            if (mouse_packet[0] & 0x10) move_x |= 0xFFFFFF00; // Sign-extend negative movement value
            
            // Handle vertical Y relative movement tracking (PS/2 Y axis inverted)
            int move_y = (int)mouse_packet[2];
            if (mouse_packet[0] & 0x20) move_y |= 0xFFFFFF00;

            // Update mouse coordinates (divide movement scale to keep mouse speed smooth)
            mouse_x += (move_x / 2);
            mouse_y -= (move_y / 2);

            // Force screen layout constraints boundary clipping
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= SCREEN_WIDTH) mouse_x = SCREEN_WIDTH - 1;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= SCREEN_HEIGHT) mouse_y = SCREEN_HEIGHT - 1;
        }
    }
}

// Main operational rendering sequence loop
void render_desktop() {
    // Desktop canvas base
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x1F); // Blue Desktop Wallpaper

    // Header Status Bar Layout
    draw_rect(0, 0, SCREEN_WIDTH, 1, 0x70);
    draw_string(2, 0, " Chromebook MouseOS v1.2 ", 0x70);

    // Decorative static desk app component frame
    draw_window(15, 6, 50, 10, "Mouse System Status Window", 0x1E, 0x1E);
    draw_string(18, 8, "Move trackpad/mouse to translate cursor block.", 0x1E);
    draw_string(18, 10, "-> Active Driver: Core PS/2 Pointer Pipeline", 0x1E);
    draw_string(18, 12, "-> Mode Type    : TUI Coordinate Layer", 0x1E);

    // Dynamic Tracking Coordinate Debug Readout String
    draw_string(2, SCREEN_HEIGHT - 1, "Trackpad X/Y Address Block Locator: ", 0x1F);
    
    // RENDER THE MOUSE POINTER LAST (Draws an inverse flashing magenta visual marker block 'X')
    draw_cell(mouse_x, mouse_y, 'X', 0x5F); 
}

extern "C" void kernel_main() {
    init_mouse();       // Fire up mouse interface hardware
    render_desktop();   // Initialize base desktop canvas frame layout

    // Operational Core Main Dynamic Execution Loop
    while (1) {
        int old_x = mouse_x;
        int old_y = mouse_y;

        process_mouse_input(); // Continually scrape hardware packets

        // If the mouse variables changed, redraft frame update to trace movements smoothly
        if (mouse_x != old_x || mouse_y != old_y) {
            render_desktop();
        }

        // Minor core breathing gap performance cycle delay trace
        for (volatile int i = 0; i < 5000; i++);
    }
}
