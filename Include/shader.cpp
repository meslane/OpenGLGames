#include "shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <glad/glad.h>

#include "image.h"

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

GLuint createTextureFromBMP(const char* filename, const GLenum wrap, const GLenum min, const GLenum mag) {
	GLuint texture;
	char* data = getBMPData(filename);
	const unsigned int width = getBMPDimension(filename, 'w');
	const unsigned int height = getBMPDimension(filename, 'h');

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "ERROR: failed to create texture from " << filename << std::endl;
		return 0;
	}

	delete data;
	return texture;
}