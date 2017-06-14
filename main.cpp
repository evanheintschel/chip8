//#include OpenGL graphics and input
#include "chip8.h"

chip8 myChip8;

int main (int argc, int **argv) {
    // Set up render system and register input callbacks
    setupGraphics();
    setupInput();

    // Initialize the Chip8 system and load the game into memory
    myChip8.initialize();
    //myChip8.loadGame("pong");

    // Emulation loop
    for (;;) {
	myChip8.emulateCycle();
	if (myChip8.drawFlag)
	    drawGraphics();
        myChip8.setKeys();
    }
    return 0;
}

void setupGraphics() {
}

void setupInput() {
}

void drawGraphics() {
}