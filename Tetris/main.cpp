#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <cstdlib>

#include "../Include/shader.h"
#include "../Include/image.h"
#include "../Include/draw.h"

#define H 72
#define W 128

#define HOLDINT 10
#define HOLDSTEP 2

#define GRAVITY 20

enum drawMode {OR, AND};

union gameArray {
	unsigned char board[H][W][3];
	unsigned char flat[H * W * 3];
};

union rotArray {
	unsigned char matrix[32][16];
	unsigned char flat[32 * 16];
};

unsigned char tetColors[8][3] = { //BGR format
	{0, 0, 0},     //no piece
	{255, 255, 0}, //I piece
	{255, 0, 0},   //J piece
	{0, 128, 255}, //L piece
	{0, 216, 255}, //O piece
	{0, 255, 216}, //S piece
	{0, 0, 255},   //Z piece
	{255, 0, 216}  //T piece
};

typedef struct Piece {
	unsigned char type; //refer to tetColors table index
	char bottom; //coords of 4x4 containter, not nessecarily those of the piece itself
	char left;
	unsigned char rot; //rotation index (0-1 for I, 0 for O, 0-3 for all else);
}piece;

typedef struct Keys {
	char up;
	char dn;
	char left;
	char right;
}keys;

keys keypress;

void drawSquare(gameArray* game, unsigned char left, unsigned char bottom, unsigned char pixel[3]) {
	for (unsigned char i = 0; i < 3; i++) {
		for (unsigned char j = 0; j < 3; j++) {
			memcpy(game->board[bottom + j][left + i], pixel, 3);
		}
	}
}

//left, bottom = (50, 3)
//right, top = (77, 66)
void drawPlayfield(gameArray* g, unsigned char field[22][10]) {
	for (unsigned int i = 0; i < 10; i++) {
		for (unsigned int j = 0; j < 22; j++) {
			drawSquare(g, 50 + (3 * i), 3 + (3 * j), tetColors[field[j][i]]);
		}
	}
}

void drawNext(gameArray* g, rotArray r, unsigned char pieceType) {
	for (char i = 0; i < 6; i++) { //x
		for (char j = 0; j < 4; j++) { //y
			drawSquare(g, 94 + (3 * i), 44 + (3 * j), tetColors[0]);
		}
	}

	
	for (char i = 0; i < 4; i++) { //x
		for (char j = 0; j < 4; j++) { //y
			if (r.matrix[(4 * pieceType) + j][i] != 0) {
				if (pieceType == 1 || pieceType == 4) {
					drawSquare(g, 97 + (3 * i), 44 + (3 * j), tetColors[pieceType]);
				} 
				else {
					drawSquare(g, 97 + (3 * i), 47 + (3 * j), tetColors[pieceType]);
				}
			}
		}
	}
}

void drawPiece(piece p, unsigned char field[22][10], drawMode mode, rotArray r) {
	for (char i = 0; i < 4; i++) { //x
		for (char j = 0; j < 4; j++) { //y
			if (((p.bottom + j) >= 0) && ((p.left + i) >= 0)) { //bounds checking
				if (mode == OR) { //draw where rotation matrix specifies
					if (r.matrix[((4 * p.type)) + j][(4 * p.rot) + i] != 0) {
						field[p.bottom + j][p.left + i] = p.type;
					}
				}
				if (mode == AND) {
					if (r.matrix[((4 * p.type)) + j][(4 * p.rot) + i] != 0) {
						field[p.bottom + j][p.left + i] = 0;
					}
				}
			}
		}
	}
}

int checkBounds(piece p, unsigned char field[22][10], rotArray r, unsigned char rot, char bottom, char left, char i, char j, int checkType) { //types 1 = rot/lateral, 2 = bottom/piece
	switch (checkType) {
		case 1:
			/* rotation and lateral collisions */
			if ((r.matrix[((4 * p.type)) + j][(4 * rot) + i] != 0) && (((bottom + j) < 0) || ((left + i) < 0) || ((left + i) > 9))) { //wall collisions
				return 1; //violation
			}
			if ((r.matrix[((4 * p.type)) + j][(4 * rot) + i] != 0) && (field[bottom + j][left + i] != 0)) { //piece collisions
				return 1;
			}
			break;
		case 2:
			/* piece bottom collisions + bottom collisions */
			if ((r.matrix[((4 * p.type)) + j][(4 * rot) + i] != 0) && (field[bottom + j][left + i] != 0)) {
				return 1;
			}
			if ((r.matrix[((4 * p.type)) + j][(4 * rot) + i] != 0) && ((bottom + j) < 0)) {
				return 1;
			}
			break;
	}
																																			 
	return 0; //no violation
}

void rotate(piece* p, unsigned char field[22][10], rotArray r) { //rotate only if all occupied rotation matrix positions would be within the playfield
	unsigned char nextRot = (p->rot < 3) ? p->rot + 1 : 0;
	
	for (char i = 0; i < 4; i++) { //x
		for (char j = 0; j < 4; j++) { //y
			if (checkBounds(*p, field, r, nextRot, p->bottom, p->left, i, j, 1)) {
				return; //do not rotate if bounds are violated
			}
		}
	}

	p->rot = nextRot;
}

void moveLateral(piece* p, unsigned char field[22][10], rotArray r, char dir) {
	char nextPos = p->left + dir;

	for (char i = 0; i < 4; i++) { //x
		for (char j = 0; j < 4; j++) { //y
			if (checkBounds(*p, field, r, p->rot, p->bottom, nextPos, i, j, 1)) {
				return;
			}
		}
	}

	p->left = nextPos;
}

int moveDown(piece* p, unsigned char field[22][10], rotArray r) {
	char nextDown = p->bottom - 1;

	for (char i = 0; i < 4; i++) { //x
		for (char j = 0; j < 4; j++) { //y
			if (checkBounds(*p, field, r, p->rot, nextDown, p->left, i, j, 2)) { //if locked
				return 1;
			}
		}
	}

	p->bottom = nextDown;
	return 0;
}

void popRow(unsigned char field[22][10], unsigned char row) {
	for (unsigned int i = row; i < 22; i++) {
		if (i < 21) {
			memcpy(field[i], field[i + 1], 10);
		}
		else {
			memset(field[i], 0, 10);
		}
	}
}

void popIfFull(unsigned char field[22][10], unsigned short* score) {
	unsigned char total = 0;

	for (unsigned int i = 0; i < 4; i++) {
		for (unsigned int y = 0; y < 22; y++) {
			for (unsigned int x = 0; x < 10; x++) {
				if (field[y][x] == 0) {
					break;
				}
				if (x == 9) {
					popRow(field, y);
					total++;
				}
			}
		}
	}

	switch (total) { //scoring
		case 1:
			total = 4;
			break;
		case 2:
			total = 10;
			break;
		case 3:
			total = 30;
			break;
		case 4:
			total = 120;
			break;
		default:
			break;
	}

	*score = *score + total;
} 

void spawnPiece(piece* p, unsigned char type) {
	p->type = type;
	p->bottom = 20;
	p->left = 4;
	p->rot = 0;
}

float normalizeCoord(unsigned char coord, int dimension) {
	return (((float)coord) / ((float)dimension / 2)) - 1;
}

void drawScore(gameArray* game, unsigned short score, unsigned char digits, unsigned char x, unsigned char y) { //bottom left of last digit
	for (unsigned int i = 0; i < digits; i++) {
		drawDigit((char*)game->flat, ((score / (int)std::pow(10, i)) % 10), W, H, (x - (6 * i)), y);
	}
}

/* window functions */
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_UP:
				keypress.up = 1;
				break;
			case GLFW_KEY_DOWN:
				keypress.dn = 1;
				break;
		}
	}
}

static void sizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

int main(void) {
	glfwInit();
	srand(time(0));

	/* create window */
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Tetris", NULL, NULL);
	if (!window) {
		std::cout << "ERROR: failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetFramebufferSizeCallback(window, sizeCallback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	/* create shaders */
	GLuint vShader = createShaderFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	GLuint fShader = createShaderFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");

	const GLuint shaderList[] = { vShader, fShader };
	GLuint shaderProgram = createShaderProgram(shaderList, 2);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	/* define texture dimensions and VBO/VAO */
	float vertices[] = {
		// positions        // texture coords
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,   // top right
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom right
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,   // bottom left
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f    // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); //position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); //texture
	glEnableVertexAttribArray(1);

	/* game variables */
	gameArray game;
	rotArray rotationMatrix;
	unsigned char playfield[22][10] = { 0 };

	/* import background texture */
	char* background = getBMPData("images/board.bmp");
	memcpy((char*)game.flat, background, H * W * 3);
	delete background;

	/* import rotation matrix */
	char* rotBMP = getBMPData("images/rotmatrixflipped.bmp");
	char* greyRotBMP = greyScaleBMP(rotBMP, 32, 16);
	memcpy((char*)rotationMatrix.flat, greyRotBMP, 32 * 16);
	delete rotBMP;
	delete greyRotBMP;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* main loop variables */
	piece p;
	spawnPiece(&p, (rand() % 7) + 1);

	keypress.up = 0;

	unsigned short hold = 0;
	unsigned short fallRate = GRAVITY;
	unsigned short fRate = GRAVITY;
	unsigned short cycle = 0;
	unsigned short score = 0;
	unsigned char next = (rand() % 7) + 1;
	drawNext(&game, rotationMatrix, next);

	/* main loop */
	while (!glfwWindowShouldClose(window)) {
		/* texture stuff */
		drawPlayfield(&game, playfield);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_BGR, GL_UNSIGNED_BYTE, game.flat);

		/* falling */
		if ((cycle % fallRate) == 0) {
			drawPiece(p, playfield, AND, rotationMatrix);
			if (moveDown(&p, playfield, rotationMatrix) == 1) {
				drawPiece(p, playfield, OR, rotationMatrix);
				popIfFull(playfield, &score);
				if (p.bottom == 20) { //if piece can no longer move, end game
					memset(playfield, 0, 22 * 10);
					score = 0;
				} 
				else {
					spawnPiece(&p, next); //if locked
				}
				drawScore(&game, score, 4, 30, 53);
				next = (rand() % 7) + 1;
				drawNext(&game, rotationMatrix, next);
			}
		}

		/* fall rate setting */
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			fallRate = 3;
		}
		else {
			fallRate = fRate;
		}
		fRate = (fRate > 3) ? GRAVITY - (score / 50) : 3;
		drawScore(&game, GRAVITY - fRate, 2, 24, 21);

		/* rotation */
		if (keypress.up == 1) {
			drawPiece(p, playfield, AND, rotationMatrix);
			rotate(&p, playfield, rotationMatrix);
			keypress.up = 0;
		}

		/* lateral motion */
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			if (hold == 0 || (hold >= HOLDINT && (hold % HOLDSTEP == 0))) {
				drawPiece(p, playfield, AND, rotationMatrix);
				moveLateral(&p, playfield, rotationMatrix, -1);
			}
			hold = (hold < USHRT_MAX) ? hold + 1 : HOLDINT;
		}
		else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			if (hold == 0 || (hold >= HOLDINT && (hold % HOLDSTEP == 0))) {
				drawPiece(p, playfield, AND, rotationMatrix);
				moveLateral(&p, playfield, rotationMatrix, 1);
			}
			hold = (hold < USHRT_MAX) ? hold + 1 : HOLDINT;
		}
		else {
			hold = 0;
		}

		drawPiece(p, playfield, OR, rotationMatrix);

		/* draw triangles */
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);

		/* this code should go last */
		glfwPollEvents();
		glfwSwapBuffers(window);

		cycle = (cycle < USHRT_MAX) ? cycle + 1 : 0;
	}

	return 0;
}