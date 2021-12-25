#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/glad.h>

GLuint createShaderFromFile(const GLenum shaderType, const char* filename);
GLuint createShaderProgram(const GLuint* shaderArray, const unsigned int num);
GLuint createTextureFromBMP(const char* filename, const GLenum wrap, const GLenum min, const GLenum max);

#endif