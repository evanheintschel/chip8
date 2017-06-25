#define NNN(x) x & 0x0FFF
#define NN(x) x & 0x00FF
#define N(x) x & 0x000F
#define X(x) (x & 0x0F00) >> 8
#define Y(x) (x & 0x00F0) >> 4

#define MEM_SIZE 4096  // 4KB
#define NUM_REG 16
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define STACK_SIZE 16
#define KEY_SIZE 16

class chip8 {
    public:
	chip8();
	~chip8();
	
	void emulateCycle();
	void debugRender();
	bool loadApplication(const char *filename);
	bool drawFlagIsSet();
	
    private:

	unsigned short opcode;  // current opcode
	unsigned char memory[MEM_SIZE];
	unsigned char V[NUM_REG];  // general purpose registers
	unsigned short I;  // 16-bit index register
	unsigned short PC;  // program counter
	unsigned char gfx[GFX_WIDTH * GFX_HEIGHT];
	unsigned bool draw_flag;
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned short stack[STACK_SIZE];
	unsigned short sp;  // stack pointer
	unsigned char key[KEY_SIZE];

	void init();
	void stackPush(unsigned short val);
	bool stackPop();
}
