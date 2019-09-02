/*********************************************************
FILE : Source.cpp (csci3260 2018-2019 Final Project)
*********************************************************/
/*********************************************************
Student Information
Student ID: 1155072307, 1155072655
Student Name: Mega Gunawan, Manisha Tamilmani Kamatchi
*********************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::perspective;
using glm::translate;
using glm::rotate;
using glm::scale;
using glm::normalize;

GLuint screenWidth = 1500;
GLuint screenHeight = 1000;

GLint programID;
GLint skyBoxProgramID;

GLuint vertexArrayID[7];
GLuint vertexBuffer[7];
GLuint uvBuffer[7];
GLuint normalBuffer[7];
GLuint textureBuffer[7];
GLuint texture[7];
GLuint normalTexture;
GLuint skyBoxVAO, skyBoxVBO, skyBoxCubeMapTextureID;
bool res[7];
int drawSize[7];
float rockModelMatrices[200][5];
GLuint rockAmount = 200;

glm::vec3 spaceCraftInitialPos = glm::vec3(60, 0, 60);
glm::vec3 spaceCraftTranslation = glm::vec3(0, 0, 0);

glm::vec4 spaceCraftWorldFrontDirection;
glm::vec4 spaceCraftWorldRightDirection;
glm::vec4 spaceCraftWorldPosition;

glm::vec3 spaceCraftLocalPosition = glm::vec3(0, 0, 0);
glm::vec3 spaceCraftLocalFront = glm::vec3(0, 0, -1);
glm::vec3 spaceCraftLocalRight = glm::vec3(1, 0, 0);

glm::mat4 spaceCraftRotationMatrix;
glm::mat4 spaceCraftTransformMatrix;
glm::mat4 spaceCraftScaleMatrix;

glm::vec3 cameraLocalPosition = glm::vec3(5, 5, 5);
glm::vec4 cameraPointPosition;
glm::vec4 cameraWorldPosition;

glm::mat4 cameraRotationMatrix;
glm::mat4 cameraPointRotationMatrix;
glm::mat4 cameraTransformMatrix;
glm::mat4 cameraTransformMatrix2;

float spaceCraftRotateAngle = -110.0f;
float cameraRotateAngle = 25.0f;
float earthRotateAngle = 0.0f;
float wonderStarRotateAngle = 0.0f;
float rockRotateAngle = 0.0f;
float ringRotateAngle = 0.0f;

float oldx = 256;

glm::vec2 mouseInitialPosition = glm::vec2(256, 256);

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

int installShaders(const char* vertexShader, const char* fragmentShader)
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode(vertexShader);
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode(fragmentShader);
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID)) {
		printf("Cannot create program with %s, %s\n", vertexShader, fragmentShader);
		return -1;
	}

	int program = glCreateProgram();
	glAttachShader(program, vertexShaderID);
	glAttachShader(program, fragmentShaderID);
	glLinkProgram(program);

	if (!checkProgramStatus(program)) {
		printf("Cannot create program with %s, %s\n", vertexShader, fragmentShader);
		return -1;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return program;
}

void SpecialKeys(int key, int x, int y)
{
	if(key == GLUT_KEY_DOWN)
	{
		spaceCraftTranslation[0] += 0.5* spaceCraftWorldFrontDirection[0];
		spaceCraftTranslation[2] += 0.5* spaceCraftWorldFrontDirection[2];
	}
	if(key == GLUT_KEY_UP)
	{
		spaceCraftTranslation[0] -= 0.5* spaceCraftWorldFrontDirection[0];
		spaceCraftTranslation[2] -= 0.5* spaceCraftWorldFrontDirection[2];
	}
	if(key == GLUT_KEY_LEFT)
	{
		spaceCraftTranslation[0] += 0.5* spaceCraftWorldRightDirection[0];
		spaceCraftTranslation[2] += 0.5* spaceCraftWorldRightDirection[2];
	}
	if(key == GLUT_KEY_RIGHT)
	{
		spaceCraftTranslation[0] -= 0.5* spaceCraftWorldRightDirection[0];
		spaceCraftTranslation[2] -= 0.5* spaceCraftWorldRightDirection[2];
	}
}

void PassiveMouse(int x, int y) {
	if (x < oldx)
	{
		spaceCraftRotateAngle += 2.5f;
		spaceCraftRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(spaceCraftRotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		cameraRotateAngle += 2.5f;
		cameraRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		cameraPointRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateAngle + 45), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (x > oldx)
	{
		spaceCraftRotateAngle -= 2.5f;
		spaceCraftRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(spaceCraftRotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		cameraRotateAngle -= 2.5f;
		cameraRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		cameraPointRotationMatrix= glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateAngle + 45), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	oldx = x;
}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 6 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width * height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);


	GLuint textureID;
	//TODO: Create one OpenGL texture and set the texture parameter 
	glGenTextures(1, &textureID);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
		GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	// OpenGL has now copied the data. Free our own version
	delete[] data;


	return textureID;
}

GLuint loadBMP_data(const char * imagepath, unsigned char *& img_data, int & img_w, int & img_h)
{
	printf("Reading image %s\n", imagepath);
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;

	// Open the file
	FILE * file = fopen(imagepath, "rb");
	if (!file)
	{
		printf("%s could not be opened! \n", imagepath);
		getchar();
		return 0;
	}
	// Read the header, i.e. the 54 first bytes
	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54)
	{
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M')
	{
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	img_w = *(int*)&(header[0x12]);
	img_h = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = img_w * img_h * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way
										 // Create a buffer
	img_data = new unsigned char[imageSize];
	// Read the actual data from the file into the buffer
	fread(img_data, 1, imageSize, file);
	// Everything is in memory now, the file can be closed
	fclose(file);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_w, img_h, 0, GL_BGR, GL_UNSIGNED_BYTE, img_data);
	delete[] img_data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	return textureID;
}

GLuint loadCubemap(vector<const GLchar*> faces) {
	unsigned int width, height;
	const GLchar* imagepath;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE5); //we use texture 5 for skybox
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (GLuint i = 0; i < faces.size(); i++) {
		imagepath = faces.at(i);
		printf("Reading image %s\n", imagepath);

		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
		unsigned char * data;

		FILE * file = fopen(imagepath, "rb");
		if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

		if (fread(header, 1, 54, file) != 54) {
			printf("Not a correct BMP file\n");
			return 0;
		}
		if (header[0] != 'B' || header[1] != 'M') {
			printf("Not a correct BMP file\n");
			return 0;
		}
		if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
		if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

		dataPos = *(int*)&(header[0x0A]);
		imageSize = *(int*)&(header[0x22]);
		width = *(int*)&(header[0x12]);
		height = *(int*)&(header[0x16]);
		if (imageSize == 0) imageSize = width * height * 3;
		if (dataPos == 0) dataPos = 54;

		data = new unsigned char[imageSize];
		fread(data, 1, imageSize, file);
		fclose(file);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}


	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return textureID;
}

void sendEarthData() { // Earth = 0
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	glGenVertexArrays(1, &vertexArrayID[0]);
	glBindVertexArray(vertexArrayID[0]);
	res[0] = loadOBJ("planet.obj", vertices, uvs, normals);
	texture[0] = loadBMP_custom("texture/earthTexture.bmp");

	normalTexture = loadBMP_custom("texture/earth_normal.bmp");

	glGenBuffers(1, &vertexBuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalBuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[0]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[0]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	drawSize[0] = vertices.size();
}

void sendSpaceCraftData() { // spacecraft = 1
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	res[1] = loadOBJ("spaceCraft.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &vertexArrayID[1]);
	glBindVertexArray(vertexArrayID[1]);
	texture[1] = loadBMP_custom("texture/spacecraftTexture.bmp");

	glGenBuffers(1, &vertexBuffer[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer[1]);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalBuffer[1]);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[1]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	drawSize[1] = vertices.size();
}

void sendWonderStarData() { // wonderstar = 2
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	glGenVertexArrays(1, &vertexArrayID[2]);
	glBindVertexArray(vertexArrayID[2]);
	res[2] = loadOBJ("planet.obj", vertices, uvs, normals);
	texture[2] = loadBMP_custom("texture/WonderStarTexture.bmp");

	glGenBuffers(1, &vertexBuffer[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer[2]);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalBuffer[2]);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[2]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	drawSize[2] = vertices.size();
}

void sendRing1Data() { // ring1 = 3
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	glGenVertexArrays(1, &vertexArrayID[3]);
	glBindVertexArray(vertexArrayID[3]);
	res[3] = loadOBJ("Ring.obj", vertices, uvs, normals);
	texture[3] = loadBMP_custom("texture/ringTexture.bmp");

	glGenBuffers(1, &vertexBuffer[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer[3]);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalBuffer[3]);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[3]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[3]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	drawSize[3] = vertices.size();
}

void sendRing2Data() { // ring2 = 4
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	glGenVertexArrays(1, &vertexArrayID[4]);
	glBindVertexArray(vertexArrayID[4]);
	res[4] = loadOBJ("Ring.obj", vertices, uvs, normals);
	texture[4] = loadBMP_custom("texture/ringTexture.bmp");

	glGenBuffers(1, &vertexBuffer[4]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[4]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer[4]);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[4]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalBuffer[4]);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[4]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[4]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	drawSize[4] = vertices.size();
}

void sendRing3Data() { // ring3 = 5
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	glGenVertexArrays(1, &vertexArrayID[5]);
	glBindVertexArray(vertexArrayID[5]);
	res[5] = loadOBJ("Ring.obj", vertices, uvs, normals);
	texture[5] = loadBMP_custom("texture/ringTexture.bmp");

	glGenBuffers(1, &vertexBuffer[5]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[5]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer[5]);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[5]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalBuffer[5]);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[5]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[5]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[5]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	drawSize[5] = vertices.size();
}

void CreateRandModelM() {
	//rockModelMatrices = new mat4[rockAmount];
	srand(glutGet(GLUT_ELAPSED_TIME));
	GLfloat radius = 6.0f;
	GLfloat offset = 1.2f;

	for (GLuint i = 0; i < rockAmount; i++)
	{
		//mat4 model;
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		GLfloat angle = (GLfloat)i / (GLfloat)rockAmount * 360.0f;
		GLfloat displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
		GLfloat x = sin(angle) * radius + displacement;
		displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
		GLfloat y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % (GLint)(2 * offset * 200)) / 100.0f - offset;
		GLfloat z = cos(angle) * radius + displacement;
		//model = translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		rockModelMatrices[i][0] = x * 5.0f;
		rockModelMatrices[i][1] = (y - 1.0f)*5.0f;
		rockModelMatrices[i][2] = z * 5.0f;
		GLfloat scale = (rand() % 10) / 100.0f + 0.05;
		rockModelMatrices[i][3] = scale;
		rockRotateAngle = rand() % 360;
		rockModelMatrices[i][4] = rockRotateAngle;
	}
}

void sendRockData() { // rock = 6
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	glGenVertexArrays(1, &vertexArrayID[6]);
	glBindVertexArray(vertexArrayID[6]);
	res[6] = loadOBJ("rock.obj", vertices, uvs, normals);
	texture[6] = loadBMP_custom("texture/RockTexture.bmp");

	glGenBuffers(1, &vertexBuffer[6]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[6]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer[6]);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[6]);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glGenBuffers(1, &normalBuffer[6]);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[6]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[6]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);	// 2nd attribute buffer : UVs
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer[6]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2); //3rd normal buffer: normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer[6]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	drawSize[6] = vertices.size();
}

void sendSkyboxData() {
	float skyBoxVertices[] = {
		// positions          
		-1.0f, +1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,

		-1.0f, -1.0f, +1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,

		+1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,

		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f
	};

	glGenVertexArrays(1, &skyBoxVAO);
	glGenBuffers(1, &skyBoxVBO);
	glBindVertexArray(skyBoxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyBoxVertices), &skyBoxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	vector<const GLchar*>earth_faces;

	earth_faces.push_back("texture/universe_skybox/purplenebula_rt.bmp");
	earth_faces.push_back("texture/universe_skybox/purplenebula_lf.bmp");
	earth_faces.push_back("texture/universe_skybox/purplenebula_up.bmp");
	earth_faces.push_back("texture/universe_skybox/purplenebula_dn.bmp");
	earth_faces.push_back("texture/universe_skybox/purplenebula_bk.bmp");
	earth_faces.push_back("texture/universe_skybox/purplenebula_ft.bmp");
	
	skyBoxCubeMapTextureID = loadCubemap(earth_faces);
}

void sendDataToOpenGL()
{
	sendSkyboxData();
	sendEarthData();
	sendSpaceCraftData();
	sendWonderStarData();
	sendRing1Data();
	sendRing2Data();
	sendRing3Data();
	sendRockData();
}

void UpdateStatus() {
	float scale = 0.005;
	spaceCraftScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
	glm::mat4 spaceCraftTransMatrix = glm::translate
	(
		glm::mat4(1.0f),
		glm::vec3(spaceCraftInitialPos[0] + spaceCraftTranslation[0], spaceCraftInitialPos[1] + spaceCraftTranslation[1], spaceCraftInitialPos[2] + spaceCraftTranslation[2])
	);
	glm::mat4 cameraTransMatrix = glm::translate
	(
		glm::mat4(1.0f),
		glm::vec3(cameraLocalPosition[0], cameraLocalPosition[1], cameraLocalPosition[2])
	);
	glm::mat4 cameraTransMatrix2 = glm::translate(
		glm::mat4(1.0f),
		glm::vec3(spaceCraftTransMatrix[3].x, spaceCraftTransMatrix[3].y, spaceCraftTransMatrix[3].z)
	);
	glm::mat4 cameraPointMatrix1 = glm::translate(
		glm::mat4(1.0f),
		glm::vec3(0.0f, 0.0f, -10.0f)
	);
	glm::mat4 cameraPointMatrix2 = glm::translate(
		glm::mat4(1.0f),
		glm::vec3(spaceCraftTransformMatrix[3].x, spaceCraftTransformMatrix[3].y, spaceCraftTransformMatrix[3].z)
	);

	spaceCraftTransformMatrix = spaceCraftTransMatrix * spaceCraftRotationMatrix * spaceCraftScaleMatrix;
	cameraTransformMatrix = cameraTransMatrix2 * cameraRotationMatrix * cameraTransMatrix2;
	cameraPointPosition = cameraPointMatrix2 * cameraPointRotationMatrix * cameraPointMatrix1 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	spaceCraftWorldPosition = spaceCraftTransMatrix * glm::vec4(spaceCraftLocalPosition, 1.0f);
	spaceCraftWorldFrontDirection = spaceCraftTransformMatrix * glm::vec4(spaceCraftLocalFront, 0.0f);
	spaceCraftWorldRightDirection = spaceCraftTransformMatrix * glm::vec4(spaceCraftLocalRight, 0.0f);
	spaceCraftWorldFrontDirection = normalize(spaceCraftWorldFrontDirection);
	spaceCraftWorldRightDirection = normalize(spaceCraftWorldRightDirection);
	cameraWorldPosition = cameraTransformMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void paintGL(void)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);

	mat4 ViewMatrix = glm::lookAt(glm::vec3(cameraWorldPosition), glm::vec3(cameraPointPosition), glm::vec3(0.0f, 1.0f, 0.0f));
	mat4 ProjectionMatrix = glm::perspective(20.0f, 1.0f, 1.0f, 200.0f);

	glUseProgram(skyBoxProgramID);

	mat4 Skb_ModelMatrix = mat4(1.0f);
	Skb_ModelMatrix = glm::scale(Skb_ModelMatrix, vec3(100.0f));

	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgramID, "M"), 1, GL_FALSE, &Skb_ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgramID, "view"), 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgramID, "projection"), 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glBindVertexArray(skyBoxVAO);
	glUniform1i(glGetUniformLocation(skyBoxProgramID, "skybox"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxCubeMapTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
	
	glUseProgram(programID);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::mat4 projectionTransformMatrix = ProjectionMatrix * ViewMatrix;
	GLuint projectionTransformMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");

	GLint ambientLightUniformLocation1 = glGetUniformLocation(programID, "ambientLight1");
	glm::vec3 ambientLight1(1.0f, 1.0f, 1.0f);
	glUniform3fv(ambientLightUniformLocation1, 1, &ambientLight1[0]);

	GLint ambientLightUniformLocation2 = glGetUniformLocation(programID, "ambientLight2");
	glm::vec3 ambientLight2(1.0f, 1.0f, 1.0f);
	glUniform3fv(ambientLightUniformLocation2, 1, &ambientLight2[0]);

	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition = vec3(cameraWorldPosition);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	GLint modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	GLint normalMap = glGetUniformLocation(programID, "normalMap");
	float normalMapfloat = 0.0f;
	glUniform1f(normalMap, normalMapfloat);

	GLuint TextureID, TextureID_0, TextureID_1;
	GLint normalMappingUniformLocation;

	// earth
	glBindVertexArray(vertexArrayID[0]);
	TextureID_0 = glGetUniformLocation(programID, "myTextureSampler");
	TextureID_1 = glGetUniformLocation(programID, "myTextureSampler1");

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glUniform1i(TextureID_0, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glUniform1i(TextureID_1, 2);

	normalMapfloat = 1.0f;
	glUniform1f(normalMap, normalMapfloat);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, -150));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), glm::radians(earthRotateAngle), glm::vec3(0, 1, 0));
	modelTransformMatrix = modelTransformMatrix * glm::scale(glm::mat4(), glm::vec3(15, 15, 15));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[0]);
	earthRotateAngle += 0.5f;

	// spacecraft
	glBindVertexArray(vertexArrayID[1]);

	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(TextureID, 3);
	
	UpdateStatus();

	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &spaceCraftTransformMatrix[0][0]);
	if ((glm::distance(glm::vec3(spaceCraftWorldPosition), glm::vec3(0.0f, 0.0f, -150.0f)) > 40.0f) && (glm::distance(glm::vec3(spaceCraftWorldPosition), glm::vec3(-40.0f, 0.0f, 40.0f)) > 40.0f)) {
		glDrawArrays(GL_TRIANGLES, 0, drawSize[1]);
	}
	
	// wonder star
	glBindVertexArray(vertexArrayID[2]);
	TextureID = glGetUniformLocation(programID, "myTextureSampler");

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glUniform1i(TextureID, 4);

	normalMapfloat = 0.0f;
	glUniform1f(normalMap, normalMapfloat);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(-40, 0, 40));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), glm::radians(wonderStarRotateAngle), glm::vec3(0, 1, 0));
	modelTransformMatrix = modelTransformMatrix * glm::scale(glm::mat4(), glm::vec3(5, 5, 5));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[2]);

	// ring1
	glBindVertexArray(vertexArrayID[3]);
	TextureID = glGetUniformLocation(programID, "myTextureSampler");

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glUniform1i(TextureID, 5);

	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, -5));
	modelTransformMatrix = modelTransformMatrix * glm::scale(glm::mat4(), glm::vec3(0.05, 0.05, 0.05));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), 2.0f, glm::vec3(1, 0, 0));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), glm::radians(ringRotateAngle), glm::vec3(0, 1, 0));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[3]);

	// ring2
	glBindVertexArray(vertexArrayID[4]);
	TextureID = glGetUniformLocation(programID, "myTextureSampler");

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glUniform1i(TextureID, 6);

	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, -15));
	modelTransformMatrix = modelTransformMatrix * glm::scale(glm::mat4(), glm::vec3(0.05, 0.05, 0.05));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), 2.0f, glm::vec3(1, 0, 0));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), glm::radians(ringRotateAngle), glm::vec3(0, 1, 0));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[4]);


	// ring3
	glBindVertexArray(vertexArrayID[5]);
	TextureID = glGetUniformLocation(programID, "myTextureSampler");

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glUniform1i(TextureID, 7);

	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, -25));
	modelTransformMatrix = modelTransformMatrix * glm::scale(glm::mat4(), glm::vec3(0.05, 0.05, 0.05));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), 2.0f, glm::vec3(1, 0, 0));
	modelTransformMatrix = modelTransformMatrix * glm::rotate(glm::mat4(), glm::radians(ringRotateAngle), glm::vec3(0, 1, 0));
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[5]);
	ringRotateAngle += 0.5f;

	// rock 
	// glBindVertexArray(vertexArrayID[6]);
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glUniform1i(TextureID, 8);
	glm::mat4 translateRock = glm::translate(glm::mat4(), glm::vec3(-40, 0, 40));
	glm::mat4 rotateRock = glm::rotate(translateRock, rockRotateAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	rockRotateAngle += 0.05f;

	for (GLuint i = 0; i < rockAmount; i++)
	{
		modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(rockModelMatrices[i][0], rockModelMatrices[i][1], rockModelMatrices[i][2]));
		modelTransformMatrix = rotateRock * modelTransformMatrix;
		modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
		glBindVertexArray(vertexArrayID[6]);
		glDrawArrays(GL_TRIANGLES, 0, drawSize[6]);
	} 

	glFlush();
	glutPostRedisplay();
}

void initializedGL(void)
{
	glewInit();
	programID = installShaders("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
	skyBoxProgramID = installShaders("SkyboxVertexShaderCode.glsl", "SkyboxFragmentShaderCode.glsl");
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(screenWidth, screenHeight);
	glutCreateWindow("Graphics Final Project");
	CreateRandModelM();
	initializedGL();
	glutDisplayFunc(paintGL);
	glutSpecialFunc(SpecialKeys);
	glutPassiveMotionFunc(PassiveMouse);
	glutMainLoop();
	return 0;
} 
