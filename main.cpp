//#include OpenGL graphics and input
#include "chip8.h"

chip8 myChip8;

int main (int argc, char **argv) {
    // Set up render system and register input callbacks
    //setupGraphics();
    //setupInput();

    // Initialize the Chip8 system and load the game into memory
    myChip8.init();
    if (!myChip8.loadApplication("PONG"))
      return -1;

    // Emulation loop
    for (;;) {
	myChip8.emulateCycle();
	if (myChip8.drawFlagIsSet())
	    //drawGraphics();
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
