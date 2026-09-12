// kernel.cpp

// 'extern "C"' prevents C++ from changing the function name behind the scenes
extern "C" void kernel_main() {
    // 0xB8000 is the memory location where the screen text data begins
    volatile char* video_memory = (volatile char*)0xB8000;
    
    const char* message = "Hello from your custom OS! Made By square-c418";
    int i = 0;

    // Loop through the string and output it to screen memory
    while (message[i] != '\0') {
        *video_memory++ = message[i]; // Write the letter character
        *video_memory++ = 0x0A;       // Write the color attribute (0x0A = Light Green text)
        i++;
    }

    // Hang the OS indefinitely so it doesn't crash or reboot
    while(1);
}
