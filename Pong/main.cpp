#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>

#ifdef _WIN32
#include<Windows.h>
#pragma comment(lib, "winmm.lib")
#endif

#define H 72
#define W 128

#define PHEIGHT 4

union gameArray {
	unsigned char board[H][W];
	unsigned char flat[W * H];
};

typedef struct Player {
	unsigned char side; //1 = left, 0 = right
	unsigned char y;
	unsigned char score;
}player;

typedef struct Ball {
	unsigned char x;
	unsigned char y;
	char slopex;
	char slopey;
}ball;

void movePlayer(player* p, GLFWwindow* window, unsigned char keyUp, unsigned char keyDn) {
	if (glfwGetKey(window, keyDn) == GLFW_PRESS) {
		p->y = (p->y > PHEIGHT) ? p->y - 1 : PHEIGHT;
	}
	else if (glfwGetKey(window, keyUp) == GLFW_PRESS) {
		p->y = (p->y < (H - PHEIGHT)) ? p->y + 1 : (H - PHEIGHT);
	}
}

void clearColumn(gameArray *game, unsigned char column) {
	for (unsigned char i = 0; i < H; i++) {
		game->board[i][column] = 0;
	}
}

void drawPlayer(player p, gameArray *game) {
	unsigned char col;

	if (p.side == 1) {
		col = 8;
	}
	else {
		col = W - 8;
	}

	clearColumn(game, col);

	for (unsigned int i = (p.y - PHEIGHT); i <= (p.y + PHEIGHT); i++) {
		game->board[i][col] = 255;
	}
}

void moveAndDrawBall(ball *b, gameArray *game, player *p1, player *p2) {
	unsigned char bonk = 0;
	game->board[b->y][b->x] = 0;

	if (b->y == 0 || b->y >= (H - 1)) {
		b->slopey = b->slopey * -1;
		bonk = 1;
	}

	if (b->x == 0 || b->x >= (W - 1)) {
		if (b->x == 0) {
			b->slopex = 1;
			p2->score++;
		}
		else {
			b->slopex = -1;
			p1->score++;
		}

		if (p1->score > 9 || p2->score > 9) {
			p1->score = 0;
			p2->score = 0;
			p1->y = (H/2);
			p2->y = (H/2);
		}

		b->x = (W / 2);
		b->y = (H / 2);
		b->slopey = 0;
		bonk = 2;
	}

	if (b->x == 8) { //hitting left paddle
		if (b->y == p1->y) { //direct hit
			b->slopey = 0;
			b->slopex = 2;
			bonk = 1;
		}
		else if (b->y > p1->y && b->y <= (p1->y + PHEIGHT)) { //top
			b->slopey = 1;
			b->slopex = 1;
			bonk = 1;
		}
		else if (b->y < p1->y && b->y >= (p1->y - PHEIGHT)) { //bottom
			b->slopey = -1;
			b->slopex = 1;
			bonk = 1;
		}
	}

	if (b->x == (W - 8)) { //hitting right paddle
		if (b->y == p2->y) { //direct hit
			b->slopey = 0;
			b->slopex = -2;
			bonk = 1;
		}
		else if (b->y > p2->y && b->y <= (p2->y + PHEIGHT)) { //top
			b->slopey = 1;
			b->slopex = -1;
			bonk = 1;
		}
		else if (b->y < p2->y && b->y >= (p2->y - PHEIGHT)) { //bottom
			b->slopey = -1;
			b->slopex = -1;
			bonk = 1;
		}
	}

	#ifdef _WIN32
	if (bonk == 1) {
		PlaySound(TEXT("audio/ponghit.wav"), NULL, SND_FILENAME | SND_ASYNC);
	}
	else if (bonk == 2) {
		PlaySound(TEXT("audio/crashquieter.wav"), NULL, SND_FILENAME | SND_ASYNC);
	}
	#endif

	b->y += b->slopey;
	b->x += b->slopex;

	//if (b->y > H) b->y = H -1 ;
	//if (b->x >= W) b->x = W - 1;

	game->board[b->y][b->x] = 255;
}

void drawSegment(gameArray *game, unsigned char startX, unsigned char startY, char dir) { //0 = right, 1 = up
	for (unsigned int i = 0; i < 5; i++) {
		if (dir == 0) {
			game->board[startY][startX + i] = 128;
		}
		else if (dir == 1) {
			game->board[startY + i][startX] = 128;
		}
	}
}

/*   0
 * 2   1
 *   3
 * 5   4 
 *   6
*/
void drawDigit(gameArray *game, unsigned char digit, unsigned char x, unsigned char y) {
	for (unsigned int i = x; i <= (x + 4); i++) { //clear space
		for (unsigned int j = (y - 8); j <= y; j++) {
			game->board[j][i] = 0;
		}
	}

	switch (digit) { //0
		case 1:
		case 4:
			break;
		default:
			drawSegment(game, x, y, 0);
			break;
	}
	switch (digit) { //1
		case 5:
		case 6:
			break;
		default:
			drawSegment(game, x + 4, y - 4, 1);
			break;
	}
	switch (digit) { //2
		case 1:
		case 2:
		case 3:
		case 7:
			break;
		default:
			drawSegment(game, x, y - 4, 1);
			break;
	}
	switch (digit) { //3
		case 1:
		case 7:
		case 0:
			break;
		default:
			drawSegment(game, x, y - 4, 0);
			break;
	}
	switch (digit) { //4
		case 2:
			break;
		default:
			drawSegment(game, x + 4, y - 8, 1);
			break;
	}
	switch (digit) { //5
		case 2:
		case 6:
		case 8:
		case 0:
			drawSegment(game, x, y - 8, 1);
			break;
		default:
			break;
	}
	switch (digit) { //6
		case 1:
		case 4:
		case 7:
			break;
		default:
			drawSegment(game, x, y - 8, 0);
			break;
	}
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

static void sizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

GLuint createShaderFromFile(const GLenum shaderType, const char* filename) {
	GLuint shader = glCreateShader(shaderType);
	std::string fileText;

	std::ifstream inp;
	inp.open(filename);
	if (!inp) {
		std::cout << "ERROR: failed to open shader file " << filename << std::endl;
		return 0;
	}

	std::string temp = "";
	while (!inp.eof()) {
		std::getline(inp, temp);
		fileText.append(temp + "\n");
	}

	inp.close();

	const char* s = fileText.c_str();
	glShaderSource(shader, 1, &s, NULL);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		std::cout << "ERROR: failed to compile shader from " << filename << std::endl;
	}

	return shader;
}

GLuint createShaderProgram(const GLuint* shaderArray, const unsigned int num) {
	GLuint shaderProgram = glCreateProgram();

	for (unsigned int i = 0; i < num; i++) {
		glAttachShader(shaderProgram, shaderArray[i]);
	}

	glLinkProgram(shaderProgram);

	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		std::cout << "ERROR: failed to create shader program" << std::endl;
	}

	return shaderProgram;
}

int main(void) {
	glfwInit();

	/* create window */
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Pong", NULL, NULL);
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
		// positions                  // texture coords
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

	/* texture settings */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/* create game board and set to zero */
	gameArray game;
	memset(game.flat, 0, sizeof(game.flat));

	player p1;
	p1.side = 0;
	p1.y = H/2;
	p1.score = 0;

	player p2;
	p2.side = 1;
	p2.y = H/2;
	p2.score = 0;

	ball b;
	b.x = W/2;
	b.y = H/2;
	b.slopex = 1;
	b.slopey = 0;

	/* main loop */
	while (!glfwWindowShouldClose(window)) {
		/* draw background color */
		glClearColor(0.0f, 0.2f, 0.5f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* update texture */
		movePlayer(&p1, window, GLFW_KEY_I, GLFW_KEY_K);
		movePlayer(&p2, window, GLFW_KEY_W, GLFW_KEY_S);
		drawPlayer(p1, &game);
		drawPlayer(p2, &game);
		drawDigit(&game, p2.score, 20, H - 8);
		drawDigit(&game, p1.score, W - 24, H - 8);
		moveAndDrawBall(&b, &game, &p2, &p1);
		glTexImage2D(GL_TEXTURE_2D, 0, 1, W, H, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, game.flat);

		/* draw triangles */
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);

		/* this code should go last */
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
}