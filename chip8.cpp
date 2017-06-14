#define MEM_SIZE 4096  // 4KB
#define NUM_REG 16
#define WIDTH 64
#define HEIGHT 32
#define STACK_SIZE 16
#define KEY_SIZE 16

unsigned short opcode;  // current opcode
unsigned char memory[MEM_SIZE];
unsigned char V[NUM_REG];  // general purpose registers
unsigned short I;  // 16-bit index register
unsigned short PC;  // program counter
unsigned char gfx[WIDTH * HEIGHT];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[STACK_SIZE];
unsigned short sp;  // stack pointer
unsigned char key[KEY_SIZE];

// Initialize registers and memory once
void chip8::initialize() {
    const unsigned short INITIAL_PC = 0x200;
    PC = INITIAL_PC;
    I = 0;
    opcode = 0;
    sp = 0;

    // Clear display
    // Clear stack
    // Clear registers V0-VF
    // Clear memory

    // Load fontset
    for (int i = 0; i < 80; i++)
        memory[i] = chip8_fontset[i];

    // Reset timers
}

void chip8::loadGame() {
    // User fopen in binary mode
    // Start filling at memory location 0x200 == 512
    
    for (int i = 0; i < buffer_size; i++)
        memory[i + 512] = buffer[i];
}

// 
void chip8::emulateCycle() {
    opcode = memory[PC] << 8 | memory[PC + 1];

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x0FFF) {
                case 0x00E0:  // 00E0: Clears the screen 
                    // Execute opcode
                    break;
                case 0x00EE:  // 00EE: Returns from subroutine
                    return;
                    break;
                default:
                     printf("Unknown opcode: 0x%X\n", opcode);

            }
            break;
        case 0x100:  // 1NNN: Jumps to address NNN
            goto opcode & 0x0FFF;
            break;
        case 0x2000:  // 2NNN: Calls subroutine at NNN
            *(opcode & 0x0FFF)();
            break;
        case 0x3000:  // 3XNN: Skips the next instruction if VX equals NN
            if (V[(opcode & 0x0F00) >> 8] == opcode & 0x00FF)
                // skip
            break;
        case 0x4000:  // 4XNN: Skips the next instruction if VX does not equal NN
            if (V[(opcode & 0x0F00) >> 8] != opcode & 0x00FF)
                // skip
            break;
        case 0x5000:  // 5XY0: Skips the next instruction if VX equals VY
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                // skip
            break;
        case 0x6000:  // 6XNN: Sets VX to NN
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        case 0x7000:  // 7XNN: Adds NN to VX
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:  // 8XY0: Sets VX to the value of VY
                    break;
                case 0x0001:  // 8XY1: Sets VX to VX OR VY
                    break;
                case 0x0002:  // 8XY2: Sets VX to VX AND VY
                    break;
                case 0x0003:  // 8XY3: Sets VX to VX XOR VY
                    break;
                case 0x0004:  // 8XY4: Adds VY to VX
                              // Sets VF to 01 if a carry occurs
                              // Sets VF to 00 if a carry does not occur
                    break;
                case 0x0005:  // 8XY5: Subtracts VY from VX
                              // Sets VF to 01 if a borrow occurs
                              // Sets VF to 00 if a borrow does not occur
 
                    break;
                case 0x0006:  // 8XY6: Sets VX to VY shifted right by one
                              // Sets VF to least significant bit of VX prior to shift
                    break;
                case 0x0007:  // 8XY7: Sets VX to VY minus VX
                              // Sets VF to 01 if a borrow occurs
                              // Sets VF to 00 if a borrow does not occur

                    break;
                case 0x000E:  // 8XYE: Sets VX to VY shifted left by one
                              // Sets VF to the most significant bit of VX prior to shift
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        case 0x9000:  // 9XY0: Skips the next instruction if VX does not equal VY
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                // skip
            break;
        case 0xA000:  // ANNN: Sets I to the address NNN
            I = opcode & 0x0FFF;
            PC += 2;
            break;
        case 0xB000:  // BNNN: Jumps to the address NNN plus V0
            goto opcode & 0x0FFF + V[0x0];
            break;
        case 0xC000:  // CXNN: Sets VX to the result of a bitwise AND operation on a random number [0, 255] and NN
            V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
            break;
        case 0xD000:  // DXYN: Draws a sprite at (VX, VY) with width of 8 pixels and height of N pixels starting at address stored in I
                      // Sets VF to 01 if any set pixels are changed to unset, and 00 otherwise
            break;
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E:  // EX9E: Skips the next instruction if the key stored in VX is pressed
                    break;
                case 0x00A1:  // EXA1: Skips the next instruction if the key stored in VX is not pressed
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:  // FX07: Sets VX to the value of the delay timer
                    break;
                case 0x000A:  // FX0A: Waits for a keypress and stores the result in VX
                    break;
                case 0x0015:  // FX15: Sets the delay timer to VX
                    break;
                case 0x0018:  // FX18: Sets the sound timer to VX
                    break;
                case 0x001E:  // FX1E: Adds VX to I
                    break;
                case 0x0029:  // FX29: Sets I to the location of the sprite corresponding to the value stored in VX
                    break;
                case 0x0033:  // FX33: Stores the binary-coded decimal equivalent of VX at address I, I + 1, I + 2
                    break;
                case 0x0055:  // FX55: Stores the values of registers [V0, VX] in memory starting at address I
                              // Sets I to I plus X plus 1 after operation
                    break;
                case 0x0066:  // FX66: Fills registers [V0, VX] with values stored in memory starting at address I
                              // Sets I to I plus X plus 1 after operation
                    break;
                default:
                    printf("Unknown opcode: 0x%X\n", opcode);

            }
            break;
        default:
            printf("Unknown opcode: 0x%X\n", opcode);
    }

    // Update timers
    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0) {
        if (sound_timer == 1)
            printf("BEEP!\n");
        sound_timer--;
    }
}

void chip8::setKeys() {
    
}
