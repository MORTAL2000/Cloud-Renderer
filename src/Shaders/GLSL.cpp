//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.
//    Modified by sueda 10/15/15.
//

#include "GLSL.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <cassert>

namespace GLSL {

const char * errorString(GLenum err)
{
	switch(err) {
	case GL_NO_ERROR:
		return "No error";
	case GL_INVALID_ENUM:
		return "Invalid enum";
	case GL_INVALID_VALUE:
		return "Invalid value";
	case GL_INVALID_OPERATION:
		return "Invalid operation";
	case GL_STACK_OVERFLOW:
		return "Stack overflow";
	case GL_STACK_UNDERFLOW:
		return "Stack underflow";
	case GL_OUT_OF_MEMORY:
		return "Out of memory";
	default:
		return "No error";
	}
}

void checkError(const char *str)
{
	GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR) {
		if(str) {
			printf("%s: ", str);
		}
		printf("GL_ERROR = %s.\n", errorString(glErr));
		assert(false);
	}
}

void printOpenGLErrors(const char * const Function, char const * const File, char const * const Line) {
	GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR) {
        const char *const ErrorString = errorString(glErr);
        printf("OpenGL error in file '%s' at line %d calling function '%s': '%s' '%d 0x%X'\n", File, Line, Function, ErrorString, glErr, glErr);
	}
}

void printShaderInfoLog(GLuint shader)
{
	GLint infologLength = 0;
	GLint charsWritten  = 0;
	GLchar *infoLog;
	
	checkError(GET_FILE_LINE);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
	checkError(GET_FILE_LINE);
	
	if(infologLength > 0) {
		infoLog = (GLchar *)malloc(infologLength);
		if(infoLog == NULL) {
			puts("ERROR: Could not allocate InfoLog buffer");
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
		printf("Shader InfoLog:\n%s\n\n", infoLog);
		free(infoLog);
	}
	checkError(GET_FILE_LINE);
}

void printProgramInfoLog(GLuint program)
{
	GLint infologLength = 0;
	GLint charsWritten  = 0;
	GLchar *infoLog;
	
	checkError(GET_FILE_LINE);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
	checkError(GET_FILE_LINE);
	
	if(infologLength > 0) {
		infoLog = (GLchar *)malloc(infologLength);
		if(infoLog == NULL) {
			puts("ERROR: Could not allocate InfoLog buffer");
			exit(1);
		}
		glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
		printf("Program InfoLog:\n%s\n\n", infoLog);
		free(infoLog);
	}
	checkError(GET_FILE_LINE);
}

void checkVersion()
{
	int major, minor;
	major = minor = 0;
	const char *verstr = (const char *)glGetString(GL_VERSION);
	
	if((verstr == NULL) || (sscanf(verstr, "%d.%d", &major, &minor) != 2)) {
		printf("Invalid GL_VERSION format %d.%d\n", major, minor);
	}
	if(major < 2) {
		printf("This shader example will not work due to the installed Opengl version, which is %d.%d.\n", major, minor);
		exit(0);
	}
}

char *textFileRead(const char *fn)
{
	FILE *fp;
	char *content = NULL;
	int count = 0;
	if(fn != NULL) {
		fp = fopen(fn,"rt");
		if(fp != NULL) {
			fseek(fp, 0, SEEK_END);
			count = (int)ftell(fp);
			rewind(fp);
			if(count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = (int)fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		} else {
			printf("error loading %s\n", fn);
		}
	}
	return content;
}

int textFileWrite(const char *fn, char *s)
{
	FILE *fp;
	int status = 0;
	if(fn != NULL) {
		fp = fopen(fn,"w");
		if(fp != NULL) {
			if(fwrite(s,sizeof(char),strlen(s),fp) == strlen(s)) {
				status = 1;
			}
			fclose(fp);
		}
	}
	return(status);
}
}

GLuint GLSL::createShader(std::string name, GLenum type) {
    GLint rc;
    GLuint shaderId = glCreateShader(type);

    const char *shader = textFileRead(name.c_str());
    glShaderSource(shaderId , 1, &shader, NULL);

    // Compile shader
    glCompileShader(shaderId);
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &rc);
    if (!rc) {
        printShaderInfoLog(shaderId);
        std::cerr << "Error compiling " << name << std::endl;
        return -1;
    }

    return shaderId;
}

