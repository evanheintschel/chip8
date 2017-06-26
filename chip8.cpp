#include "chip8.h"
#include <stdio.h>
#include <iostream>
#include <assert.h>

#define FONT_SIZE 80

unsigned char chip8_fontset[FONT_SIZE] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

chip8::chip8() {
}

chip8::~chip8() {
}

// Initialize registers and memory once
void chip8::init() {
    const unsigned short INITIAL_PC = 0x200;
    PC = INITIAL_PC;
    I = 0;
    opcode = 0;
    sp = 0;

    // Clear display
    for (int i = 0; i < GFX_WIDTH * GFX_HEIGHT; i++)
	gfx[i] = 0;
    draw_flag = false;

    // Clear stack
    for (int i = 0; i < STACK_SIZE; i++)
	stack[i] = 0;

    // Clear registers V0-VF
    for (int i = 0; i < NUM_REG; i++)
	V[i] = 0;

    // Clear memory
    for (int i = 0; i < MEM_SIZE; i++)
	memory[i] = 0;
    
    // Load fontset
    for (int i = 0; i < 80; i++)
        memory[i] = chip8_fontset[i];

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;
}

bool chip8::loadApplication(const char * filename) {
    std::cout << "Loading application: " << filename << std::endl;
    int buffer_size = fileSize(filename);
    if (buffer_size == -1)
        return false;
    FILE * app_file = fopen(filename, "rb");
    if (app_file == NULL) {
        std::cerr << "File error" << std::endl;
        return false;
    }
    char * buffer = (char *)malloc(sizeof(char) * buffer_size);
    if (buffer == NULL) {
        std::cerr << "Memory error" << std::endl;
        fclose(app_file);
        return false;;
    }
    int read_result = fread(buffer, 1, buffer_size, app_file);    
    if (read_result != buffer_size) {
        std::cerr << "Reading error" << std::endl;
        std::cerr << "\tread_result: " << read_result << std::endl;
        std::cerr << "\tbuffer_size: " << buffer_size << std::endl;
        fclose(app_file);
        free(buffer);
        return false;
    }
    const int START_LOCATION = 0x200;
    const int MAX_SIZE = MEM_SIZE - START_LOCATION;
    if (buffer_size > MAX_SIZE) {
        std::cerr << "File too large: " << buffer_size << std::endl;
        fclose(app_file);
        free(buffer);
        return false;
    }
    std::cout << "buffer_size: " << buffer_size << std::endl;
    for (int i = 0; i < buffer_size; i++) {
        memory[i + START_LOCATION] = buffer[i];
        std::cout << "memory[" << i + START_LOCATION << "] = " << buffer[i] << std::endl;
    }
    fclose(app_file);
    free(buffer);
    return true;
}

// 
void chip8::emulateCycle() {
    opcode = memory[PC] << 8 | memory[PC + 1];
    if (PC != 512 && PC != 522)
        std::cout << "Emulating" << std::endl << "PC: " << PC << std::endl;;
    if (PC < 100) {
        std::cout << std::hex << opcode << std::endl;
    }
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x0FFF) {
                case 0x00E0: { // 00E0: Clears the screen
		            for (int i = 0; i < GFX_WIDTH * GFX_HEIGHT; i++)
			            gfx[i] = 0;
                    draw_flag = true;
		            PC += 2;
                    break;
                }
                case 0x00EE:  // 00EE: Returns from subroutine
                    PC = stackPop();
		            PC += 2;
                    break;
                default:
                    if (opcode != 0)
                        printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        case 0x100:  // 1NNN: Jumps to address NNN
	        //stackPush(PC);
            //goto NNN(opcode);
            PC = NNN(opcode);
            break;
        case 0x2000:  // 2NNN: Calls subroutine at NNN
	        stackPush(PC);
	        PC = NNN(opcode);
            break;
        case 0x3000:  // 3XNN: Skips the next instruction if VX equals NN
            if (V[X(opcode)] == (NN(opcode)))
                PC += 4;
            break;
        case 0x4000:  // 4XNN: Skips the next instruction if VX does not equal NN
            if (V[X(opcode)] != (NN(opcode)))
                PC += 4;
            break;
        case 0x5000:  // 5XY0: Skips the next instruction if VX equals VY
            if (V[X(opcode)] == V[Y(opcode)])
                PC += 4;
            break;
        case 0x6000:  // 6XNN: Sets VX to NN
            V[X(opcode)] = NN(opcode);
	        PC += 2;
            break;
        case 0x7000:  // 7XNN: Adds NN to VX
            V[X(opcode)] += NN(opcode);
            PC += 2;
	        break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:  // 8XY0: Sets VX to the value of VY
		            V[X(opcode)] = V[Y(opcode)];
		            PC += 2;
                    break;
                case 0x0001:  // 8XY1: Sets VX to VX OR VY
		            V[X(opcode)] = V[X(opcode)] | V[Y(opcode)];
                    PC += 2;
		            break;
                case 0x0002:  // 8XY2: Sets VX to VX AND VY
		            V[X(opcode)] = V[X(opcode)] & V[Y(opcode)];
                    PC += 2;
		            break;
                case 0x0003:  // 8XY3: Sets VX to VX XOR VY
		            V[X(opcode)] = V[X(opcode)] ^ V[Y(opcode)];
                    PC += 2;
		            break;
                case 0x0004:  // 8XY4: Adds VY to VX
                              // Sets VF to 01 if a carry occurs
                              // Sets VF to 00 if a carry does not occur
		            if (V[Y(opcode)] > 0xFF - V[X(opcode)])
			            V[0xF] = 1;
		            else
			            V[0xF] = 0;
		            V[X(opcode)] += V[Y(opcode)];
                    PC += 2;
		            break;
                case 0x0005:  // 8XY5: Subtracts VY from VX
                              // Sets VF to 01 if a borrow occurs
                              // Sets VF to 00 if a borrow does not occur
		            if (V[Y(opcode)] > V[X(opcode)])
			            V[0xF] = 1;
		            else
			            V[0xF] = 0;
		            V[X(opcode)] -= V[Y(opcode)];
		            PC += 2;
                    break;
                case 0x0006:  // 8XY6: Sets VX to VY shifted right by one
                              // Sets VF to least significant bit of VX prior to shift
                    V[0xF] = V[X(opcode)] & 1;
		            V[X(opcode)] = V[Y(opcode)] >> 1;
		            PC += 2;
                    break;
                case 0x0007:  // 8XY7: Sets VX to VY minus VX
                              // Sets VF to 01 if a borrow occurs
                              // Sets VF to 00 if a borrow does not occur
		            if (V[X(opcode)] > V[Y(opcode)])
			            V[0xF] = 1;
		            else
			            V[0xF] = 0;
		            V[X(opcode)] = V[Y(opcode)] - V[X(opcode)];
		            PC += 2;
                    break;
                case 0x000E:  // 8XYE: Sets VX to VY shifted left by one
                              // Sets VF to the most significant bit of VX (or VY?) prior to shift
		            V[0xF] = (V[X(opcode)] & 0x8000) >> 15;
		            V[X(opcode)] = V[Y(opcode)] << 1;
		            PC += 2;
                    break;
                default:
                    if (opcode != 0)
                        printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        case 0x9000:  // 9XY0: Skips the next instruction if VX does not equal VY
            if (V[X(opcode)] != V[Y(opcode)])
                PC += 4;
            break;
        case 0xA000:  // ANNN: Sets I to the address NNN
            I = NNN(opcode);
            PC += 2;
            break;
        case 0xB000:  // BNNN: Jumps to the address NNN plus V0
	        //stackPush(PC);
            //goto NNN(opcode) + V[0x0];
            PC = (NNN(opcode)) + V[0x0];
            break;
        case 0xC000:  // CXNN: Sets VX to the result of a bitwise AND operation on a random number [0, 255] and NN
            V[X(opcode)] = (rand() % 256) & (NN(opcode));
	        PC += 2;
            break;
	    case 0xD000: { // DXYN: Draws a sprite at (VX, VY) with width of 8 pixels and height of N pixels starting at address stored in I
                       // Sets VF to 01 if any set pixels are changed to unset, and 00 otherwise
	        const unsigned short X = V[X(opcode)];
	        const unsigned short Y = V[Y(opcode)];
	        const unsigned short HEIGHT = N(opcode);
	        const unsigned short WIDTH = 8;
	    
            V[0xF] = 0;
	        for (unsigned short y_line = 0; y_line < HEIGHT; y_line++) {
		        unsigned short pixel = memory[I + y_line];
		        for (unsigned short x_line = 0; x_line < WIDTH; x_line++) {
		            if (pixel & (0x80 >> x_line)) {
			            if (gfx[X + x_line + (Y + y_line) * GFX_WIDTH] == 1)
			                V[0xF] = 1;
			            gfx[X + x_line + (Y + y_line) * GFX_WIDTH] ^= 1;
		            }
		        }
	        }
	        draw_flag = true;
	        PC += 2;
            break;
        }
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E:  // EX9E: Skips the next instruction if the key stored in VX is pressed
		            if (key[V[X(opcode)]])
			            PC += 4;
		            else
			            PC += 2;
                    break;
                case 0x00A1:  // EXA1: Skips the next instruction if the key stored in VX is not pressed
                    if (!key[V[X(opcode)]])
			            PC += 4;
		            else
			            PC += 2;
		            break;
                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:  // FX07: Sets VX to the value of the delay timer
		            V[X(opcode)] = delay_timer;
		            PC += 2;
                    break;
		        // ***********************
                case 0x000A: { // FX0A: Waits for a keypress and stores the result in VX
                    bool key_pressed = false;
                    for (int i = 0; i < KEY_SIZE; i++) {
                        if (key[i] != 0) {
                            key_pressed = true;
                            V[X(opcode)] = i;
                        }
                    }
                    if (!key_pressed)
                        return;
                    PC += 2;
		            break;
                }
                case 0x0015:  // FX15: Sets the delay timer to VX
		            delay_timer = V[X(opcode)];
                    PC += 2;
		            break;
                case 0x0018:  // FX18: Sets the sound timer to VX
		            sound_timer = V[X(opcode)];
                    PC += 2;
		            break;
                case 0x001E:  // FX1E: Adds VX to I
		            I += V[X(opcode)];
                    PC += 2;
		            break;
                case 0x0029:  // FX29: Sets I to the location of the sprite corresponding to the value stored in VX
                    PC += 2;
		            break;
                case 0x0033:  // FX33: Stores the binary-coded decimal equivalent of VX at address I, I + 1, I + 2
                    memory[I] = V[X(opcode)] / 100;
		            memory[I + 1] = (V[X(opcode)] / 10) % 10;
		            memory[I + 2] = (V[X(opcode)] % 100) % 10;
		            PC += 2;
		            break;
                case 0x0055: { // FX55: Stores the values of registers [V0, VX] in memory starting at address I
                               // Sets I to I plus X plus 1 after operation
		            for (unsigned short i = 0; i <= X(opcode); i++)
			            memory[I + i] = V[i];
		            I += (X(opcode)) + 1;
                    PC += 2;
		            break;
                }
                case 0x0066: { // FX66: Fills registers [V0, VX] with values stored in memory starting at address I
                               // Sets I to I plus X plus 1 after operation
		            for (unsigned short i = 0; i <= X(opcode); i++)
			            V[i] = memory[I + i];
		            I += (X(opcode)) + 1;
                    PC += 2;
		            break;
                }
                default:
                    if (opcode != 0)
                        printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;
        default:
            if (opcode != 0)
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

bool chip8::drawFlagIsSet() {
	return draw_flag;
}

void chip8::setKeys() {
    
}

int chip8::fileSize(const char *filename) {
    FILE * pFile;
    long size;

    pFile = fopen(filename, "rb");
    if (pFile == NULL) {
        std::cerr << "Error opening file in fileSize()" << std::endl;
        return -1;
    }
    else {
        fseek(pFile, 0, SEEK_END);   // non-portable
        size = ftell(pFile);
        fclose(pFile);
        printf("Size of %s: %ld bytes.\n", filename, size);
    }
    return size;
}

void chip8::stackPush(unsigned short val) {
    assert (sp < STACK_SIZE);
    stack[sp] = val;
    sp++;
}

unsigned short chip8::stackPop() {
    assert (sp > 0);
    sp--;
    return stack[sp];
}
