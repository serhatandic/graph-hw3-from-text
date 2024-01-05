#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

int activeProgramIndex = 2;

GLuint gProgram[4];
GLint gIntensityLoc;
float gIntensity = 1000;
int gWidth = 640, gHeight = 480;

GLint modelingMatrixLoc[4];
GLint viewingMatrixLoc[4];
GLint projectionMatrixLoc[4];
GLint eyePosLoc[4];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

// cube globals
float cubeXPositions[] = { -2.3f, 0.0f, 2.3f };
float initialCubeXPositions[] = { -2.f, 0.0f, 2.f };
float cubeZPosition = -16.4f; // The cube's Z position
int colorArray[] = { 0, 1, 0 }; // 0 for red and 1 for yellow
float cubeDepth = 0.55f; // The cube's depth

// bunny globals
float bunnyZCoord = -1.8f;
float bunnyForwardSpeed = -0.05f; // The bunny's forward speed
float bunnyYCoord = -1.3f; // The bunny's Y coordinate
float bunnyXCoord = 0.0f; // The bunny's X coordinate
float hopHeight = 0.01f; // Maximum height of the bunny's hop
float direction = 1; // 1 for up and -1 for down
float bunnyReplacement = 0.0f; // The bunny's replacement value
static float bunnyRotationAngle = 0;
bool shouldRotate = false;
bool bunnyFainted = false;

float cameraZPosition = 0.0f; // The camera's Z position
bool randomizeColor = false;
struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct MeshData {
    vector<Vertex> vertices;
    vector<Normal> normals;
    vector<Face> faces;
	vector<Texture> textures;
	int index;
	int bufferId;
};

struct VBOData {
    GLuint vao;
    GLuint vertexBuffer;
    GLuint indexBuffer;
	size_t indexCount;
};

struct Mesh {
	VBOData vboData;
	// set each to none
	glm::mat4 modelingMatrix = glm::mat4(1.0f); 
	glm::mat4 viewingMatrix = glm::mat4(1.0f); 
	glm::mat4 projectionMatrix = glm::mat4(1.0f); 
	glm::vec3 eyePos = glm::vec3(0.0f); // Zero vector
	int color = -1;
	int zCoordinate;
	int xCoordinate;
    GLuint shaderProgramIndex; // Index of the shader program for this mesh
};

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
vector<Mesh> meshes;
std::map <string, vector<Mesh>> meshMap;
std::map <string, vector<Mesh>> initialMeshMap;

MeshData ParseObj(const string& fileName)
{
	fstream myfile;
	MeshData meshData;
	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						meshData.textures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						meshData.normals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						meshData.vertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					meshData.faces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		cout << "Cannot find file name: " + fileName << endl;
		exit(-1);
	}


	return meshData;
}



bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram(); // platform
    gProgram[1] = glCreateProgram(); // bunny
    gProgram[2] = glCreateProgram(); // text
    gProgram[3] = glCreateProgram(); // cube

    createVS(gProgram[0], "platform_v.glsl");
    createFS(gProgram[0], "platform_f.glsl");

    createVS(gProgram[1], "bunny_v.glsl");
    createFS(gProgram[1], "bunny_f.glsl");

    createVS(gProgram[2], "vert_text.glsl");
    createFS(gProgram[2], "frag_text.glsl");

    createVS(gProgram[3], "cube_v.glsl");    
    createFS(gProgram[3], "cube_f.glsl");

    glBindAttribLocation(gProgram[0], 0, "inVertex");
    glBindAttribLocation(gProgram[0], 1, "inNormal");
    glBindAttribLocation(gProgram[1], 0, "inVertex");
    glBindAttribLocation(gProgram[1], 1, "inNormal");
    glBindAttribLocation(gProgram[2], 2, "vertex");

    glLinkProgram(gProgram[0]);
    glLinkProgram(gProgram[1]);
    glLinkProgram(gProgram[2]);
    glLinkProgram(gProgram[3]);
    glUseProgram(gProgram[0]);

    gIntensityLoc = glGetUniformLocation(gProgram[0], "intensity");
    cout << "gIntensityLoc = " << gIntensityLoc << endl;
    glUniform1f(gIntensityLoc, gIntensity);

    for (int i = 0; i < 4; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

VBOData initVBO(const MeshData& mesh) {
    VBOData vboData;

    // Create and bind the VAO
    glGenVertexArrays(1, &vboData.vao);
    assert(vboData.vao > 0);
    glBindVertexArray(vboData.vao);
    cout << "vao = " << vboData.vao << endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    // Create vertex buffer
    glGenBuffers(1, &vboData.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vboData.vertexBuffer);

    int vertexDataSizeInBytes = mesh.vertices.size() * 3 * sizeof(GLfloat);
    int normalDataSizeInBytes = mesh.normals.size() * 3 * sizeof(GLfloat);
    GLfloat* vertexData = new GLfloat[mesh.vertices.size() * 3];
    GLfloat* normalData = new GLfloat[mesh.normals.size() * 3];

    // Fill vertex and normal data
    for (int i = 0; i < mesh.vertices.size(); ++i) {
        vertexData[3 * i] = mesh.vertices[i].x;
        vertexData[3 * i + 1] = mesh.vertices[i].y;
        vertexData[3 * i + 2] = mesh.vertices[i].z;
    }
    for (int i = 0; i < mesh.normals.size(); ++i) {
        normalData[3 * i] = mesh.normals[i].x;
        normalData[3 * i + 1] = mesh.normals[i].y;
        normalData[3 * i + 2] = mesh.normals[i].z;
    }

    glBufferData(GL_ARRAY_BUFFER, vertexDataSizeInBytes + normalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, vertexDataSizeInBytes, normalDataSizeInBytes, normalData);

    // Create index buffer
    glGenBuffers(1, &vboData.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboData.indexBuffer);

    int indexDataSizeInBytes = mesh.faces.size() * 3 * sizeof(GLuint);
    GLuint* indexData = new GLuint[mesh.faces.size() * 3];

    for (int i = 0; i < mesh.faces.size(); ++i) {
        indexData[3 * i] = mesh.faces[i].vIndex[0];
        indexData[3 * i + 1] = mesh.faces[i].vIndex[1];
        indexData[3 * i + 2] = mesh.faces[i].vIndex[2];
    }
	vboData.indexCount = mesh.faces.size() * 3;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // Free CPU memory
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertexDataSizeInBytes));

    return vboData;
}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void initPlatform(){
    MeshData meshdata = ParseObj("quad.obj");
	Mesh quad;
	quad.vboData = initVBO(meshdata);
	float fovyRad = (float)(90.0 / 180.0) * M_PI;
	int w = 1000, h = 800;
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);
	quad.projectionMatrix = projectionMatrix;

	for (int i = 0; i < 550; i++){ // length
		for (int j = -12; j < 13; j++){ // width
			glm::mat4 qmatT = glm::translate(glm::mat4(1.0), glm::vec3(float(j) * 0.3, -1.5f, 10.f - 0.3 *i));
			glm::mat4 qmatS = glm::scale(glm::mat4(1.0), glm::vec3(0.3f, 0.3f, 0.3f));
			quad.modelingMatrix = qmatT * qmatS;
			quad.zCoordinate = 10.f - 0.3 *i;
			quad.xCoordinate = float(j) * 0.3;
			quad.shaderProgramIndex = 0;
			meshMap["platform"].push_back(quad);

		}
	}
}			

void initBunny(){
	MeshData meshdata = ParseObj("bunny.obj");
	Mesh bunny;
	bunny.vboData = initVBO(meshdata);

	float rotationAngle = -90.0f;
	glm::mat4 bmatT = glm::translate(glm::mat4(1.0), glm::vec3(0.f, -1.3f, -1.8f));
	glm::mat4 bmatS = glm::scale(glm::mat4(1.0), glm::vec3(0.3, 0.3, 0.3));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the
	bunny.modelingMatrix =  bmatT * rotationMatrix * bmatS;
	bunny.shaderProgramIndex = 1;
	meshMap["bunny"].push_back(bunny);
}

void initCubes(){
	MeshData meshdata = ParseObj("cube.obj");
	Mesh cube;
	cube.vboData = initVBO(meshdata);
	cube.shaderProgramIndex = 3;

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 0.8, 0.3));
	glm::mat4 translationMatrixCube0 = glm::translate(glm::mat4(1.0f), glm::vec3(cubeXPositions[0], -1.0, cubeZPosition)); // Translate to the starting position
	glm::mat4 translationMatrixCube1 = glm::translate(glm::mat4(1.0f), glm::vec3(cubeXPositions[1], -1.0, cubeZPosition)); // Translate to the starting position
	glm::mat4 translationMatrixCube2 = glm::translate(glm::mat4(1.0f), glm::vec3(cubeXPositions[2], -1.0, cubeZPosition)); // Translate to the starting position


	cube.modelingMatrix = translationMatrixCube0 * scaleMatrix;
	cube.color = colorArray[0];
	meshMap["cube"].push_back(cube);
	cube.modelingMatrix = translationMatrixCube1 * scaleMatrix;
	cube.color = colorArray[1];
	meshMap["cube"].push_back(cube);
	cube.modelingMatrix = translationMatrixCube2 * scaleMatrix;
	cube.color = colorArray[2];
	meshMap["cube"].push_back(cube);

}

void init() 
{
	vector<Mesh> platformMeshes;
	vector<Mesh> bunnyMeshes;
	vector<Mesh> cubeMeshes;
	
	meshMap["platform"] = platformMeshes;
	meshMap["bunny"] = bunnyMeshes;
	meshMap["cube"] = cubeMeshes;


    initBunny();
	initCubes();
	initPlatform();

    initialMeshMap = meshMap;

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initFonts(gWidth, gHeight);
    //initVBO();
}


void bunnyRotateX(){
	float angleRad = (float)(bunnyRotationAngle / 180.0) * M_PI;
	cout << angleRad << endl;

	glm::mat4 matR = glm::rotate<float>(glm::mat4(1.0), (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 matRz = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));

	glm::mat4 modelingMatrix = matRz * matR;
	meshMap["bunny"][0].modelingMatrix *= modelingMatrix;

}

void resetGame(){
	// reset bunny position
	bunnyZCoord = -1.8f;
	bunnyForwardSpeed = -0.03f; // The bunny's forward speed
	bunnyYCoord = -1.3f; // The bunny's Y coordinate
	bunnyXCoord = 0.0f; // The bunny's X coordinate
	hopHeight = 0.01f; // Maximum height of the bunny's hop
	direction = 1; // 1 for up and -1 for down
	bunnyReplacement = 0.0f; // The bunny's replacement value

	// reset camera position
	cameraZPosition = 0.0f; // The camera's Z position

	// reset cube positions
	for (int i = 0; i < 3; i++) {
		cubeXPositions[i] = initialCubeXPositions[i];
	}

	meshMap = initialMeshMap;
}

void drawPlatform(){
	vector<Mesh> quads = meshMap["platform"];
	int numElementsToRemove = 25;
	for (int i = 0; i < numElementsToRemove; i++){
		// update modeling matrix to move by 0.3 in Z at every call
		int zCoord = quads[i].zCoordinate;
		int xCoord = quads[i].xCoordinate;

		float newZCoord = zCoord - (quads.size() / 25) * 0.3f - 0.3f;
		glm::mat4 qmatT = glm::translate(glm::mat4(1.0), glm::vec3(xCoord, -1.5f, newZCoord));
		quads[i].modelingMatrix = qmatT;

		quads[i].zCoordinate = newZCoord;
	}
	std::rotate(quads.begin(), quads.begin() + numElementsToRemove, quads.end());
	meshMap["platform"] = quads;
	
}

void drawBunny(){
	Mesh bunny = meshMap["bunny"].back();

	meshMap["bunny"].pop_back();
	float rotationAngle = -90.0f;
	// update the modeling matrix to move by 0.01 in Z at every call
	glm::mat4 bmatT = glm::translate(glm::mat4(1.0), glm::vec3(bunnyXCoord, bunnyYCoord, bunnyZCoord));
	glm::mat4 bmatS = glm::scale(glm::mat4(1.0), glm::vec3(0.3, 0.3, 0.3));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the
	bunny.modelingMatrix =  bmatT * rotationMatrix * bmatS;
	bunnyZCoord += bunnyForwardSpeed;
	bunnyReplacement += -bunnyForwardSpeed;
	// update y coord to give a hopping animation
	if (bunnyYCoord > -1.0f + hopHeight) {
        direction = -1; // Start moving down
    } else if (bunnyYCoord < -1.3f) {
        direction = 1; // Start moving up
        bunnyYCoord = -1.3f; // Reset to ground level to avoid sinking below the ground
    }
	if (direction == -1){
		bunnyYCoord += bunnyForwardSpeed;
	}else if (direction == 1){
		bunnyYCoord -= bunnyForwardSpeed;
	}
	
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, cameraZPosition);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); 
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);    
	glm::mat4 viewingMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	bunny.viewingMatrix = viewingMatrix;
	cameraZPosition += bunnyForwardSpeed;
	meshMap["bunny"].push_back(bunny);
}

void drawCubes(){
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 0.8, 0.3));
	for (int i = 0; i < 3; i++){
		glm::mat4 modelingMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cubeXPositions[i], -1.0, cubeZPosition)) * scaleMatrix;
		meshMap["cube"][i].modelingMatrix = modelingMatrix;
		meshMap["cube"][i].color = colorArray[i];
	}	
}

void checkCollision(){
	// check if bunny is collided with a cube
	for (int i = 0; i < 3; i++) {
		float xPosDiff = cubeXPositions[i] - bunnyXCoord;
		float zPosDiff = bunnyZCoord - cubeZPosition;
		if (xPosDiff <= 0.25f && xPosDiff >= -0.25f  && zPosDiff < 0.8f && zPosDiff > -0.8f) {
			if (colorArray[i] == 0){
				// red collision
				// if collided, rotate bunny sideways
				float rotationAngle = -90.0f;
				meshMap["bunny"][0].modelingMatrix = glm::rotate(meshMap["bunny"][0].modelingMatrix, glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
				bunnyForwardSpeed = 0;
				// move collided object so the user wont see it
				meshMap["cube"][i].modelingMatrix = glm::translate(meshMap["cube"][i].modelingMatrix, glm::vec3(0.0f, 0.0f, 100.0f));	 
		
				return;
			}else{
				// yellow collision
				cubeZPosition -= 16.4;
				randomizeColor = true;
				// bunny should rotate 360 in x axis
				shouldRotate = true;
				bunnyForwardSpeed -= 0.01;

			}

		}else if (zPosDiff < -0.8f){
			cubeZPosition -= 16.4;
			randomizeColor = true;
		}
	}
}

void drawModel()
{
	for (const Mesh& mesh: meshMap["bunny"]){
		if (mesh.modelingMatrix != glm::mat4(1.0f)) {
			modelingMatrix = mesh.modelingMatrix;
		}

		if (mesh.viewingMatrix != glm::mat4(1.0f)) {
			viewingMatrix = mesh.viewingMatrix;
		}

		if (mesh.projectionMatrix != glm::mat4(1.0f)) {
			projectionMatrix = mesh.projectionMatrix;
		}

		activeProgramIndex = mesh.shaderProgramIndex;

		glUseProgram(gProgram[activeProgramIndex]);

		glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
		glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
		glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));	
                assert(glGetError() == GL_NO_ERROR);

		glBindVertexArray(mesh.vboData.vao);
		glDrawElements(GL_TRIANGLES, mesh.vboData.indexCount, GL_UNSIGNED_INT, 0);	
	}


	for (const Mesh& mesh: meshMap["cube"]){
		if (mesh.modelingMatrix != glm::mat4(1.0f)) {
			modelingMatrix = mesh.modelingMatrix;
		}

		if (mesh.viewingMatrix != glm::mat4(1.0f)) {
			viewingMatrix = mesh.viewingMatrix;
		}

		if (mesh.projectionMatrix != glm::mat4(1.0f)) {
			projectionMatrix = mesh.projectionMatrix;
		}

		activeProgramIndex = mesh.shaderProgramIndex;

		glUseProgram(gProgram[activeProgramIndex]);
		glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
		glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
		glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

		GLint objectColorLocation = glGetUniformLocation(gProgram[2], "objectColor");
		glm::vec3 redColor = glm::vec3(1.0f, 0.0f, 0.0f); // RGB for red
		glm::vec3 yellowColor = glm::vec3(1.0f, 1.0f, 0.0f); // RGB for yellow

		if (mesh.color == 0){
			glUniform3fv(objectColorLocation, 1, glm::value_ptr(redColor));
		}else if (mesh.color == 1){
			glUniform3fv(objectColorLocation, 1, glm::value_ptr(yellowColor));
		}

		glBindVertexArray(mesh.vboData.vao);
		glDrawElements(GL_TRIANGLES, mesh.vboData.indexCount, GL_UNSIGNED_INT, 0);	
	}

	for (const Mesh& mesh: meshMap["platform"]){
		if (mesh.modelingMatrix != glm::mat4(1.0f)) {
			modelingMatrix = mesh.modelingMatrix;
		}

		if (mesh.viewingMatrix != glm::mat4(1.0f)) {
			viewingMatrix = mesh.viewingMatrix;
		}

		if (mesh.projectionMatrix != glm::mat4(1.0f)) {
			projectionMatrix = mesh.projectionMatrix;
		}

		activeProgramIndex = mesh.shaderProgramIndex;

		glUseProgram(gProgram[activeProgramIndex]);
		glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
		glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
		glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

		glBindVertexArray(mesh.vboData.vao);
		glDrawElements(GL_TRIANGLES, mesh.vboData.indexCount, GL_UNSIGNED_INT, 0);	
	}
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[2]);
    glUniform3f(glGetUniformLocation(gProgram[2], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}



void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    renderText("CENG 477 - 2022", 0, 0, 1, glm::vec3(0, 1, 1));
    drawModel();
    assert(glGetError() == GL_NO_ERROR);

}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        cout << "F pressed" << endl;
        glUseProgram(gProgram[1]);
    }
    else if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        cout << "V pressed" << endl;
        glUseProgram(gProgram[0]);
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        cout << "D pressed" << endl;
        gIntensity /= 1.5;
        cout << "gIntensity = " << gIntensity << endl;
        glUseProgram(gProgram[0]);
        glUniform1f(gIntensityLoc, gIntensity);
    }
    else if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        cout << "B pressed" << endl;
        gIntensity *= 1.5;
        cout << "gIntensity = " << gIntensity << endl;
        glUseProgram(gProgram[0]);
        glUniform1f(gIntensityLoc, gIntensity);
    }
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(gWidth, gHeight, "Simple Example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

