#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
void draw(GLFWwindow*) ;
using namespace std;

void handleKeypress(unsigned char key , int x, int y){
switch (key){
	case 27:
		exit(0);
	}
}
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
VAO *triangle, *rectangle;
int gameflag=0;
VAO *redbox, *greenbox;
VAO *brick1 ,*brick2 ,*brick3 , *brick4 ,*brick5, *brick6, *brick7, *brick8, *brick9, *brick10, *brick11, *brick12, *brick13, *brick14,*brick15;
VAO *rectlaser1 , *rectlaser2, *laser;//my change
float collectingbox_xlength = 0.5; // length in x direction of collecting boxes
float collectingbox_ylength = 0.75;//length in y direction of collecting boxes
float redbox_x = -2.5 ;
float greenbox_x = 2.5;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
float laser1_xlength =0.5 ;
float laser1_ylength= 0.6 ;
float laser2_xlength= 1 ;
float laser2_ylength= 0.2;
float laser1_x= -7.4; //this is coordinate for both the laser and cannon2 i.e laser2 which
float laser1_y=0 , brick_x[16];
float brick_y[16] ;
float h[6], g[6], speed[16];
float laser2_rotation = 0;
float laser_xlength =0.1;
float laser_ylength = 0.05;
float laserflag=0;
int flag[16];
float laserx=-7.4, lasery =0;
float laserrotation ;
long long int points=0;
double last_update_time1 = glfwGetTime(), current_time1;
int mouseflag = 0;
float speedupper , speedlower ;
int f11=0, f12=0, f13=0, f14=0, f15=0,f16=0, f17=0;
int f21=0, f22=0, f23=0, f24=0, f25=0, f26=0, f27=0;
int f31=0, f32=0, f33 = 0, f34=0, f35=0, f36 =0 , f37=0;
int negativeflag =0;
float xpan = 0, ypan =0 ,zoom=1;
	double xpos, ypos;
void increasespeed()
{
	int i;
	for(i=1;i<10;i++)
	{
		if(speed[i] <3.5 )
		{
			speed[i]+=0.03;
		}
	}
	if(speedupper < 3.5)
	{
		speedupper += 0.03;
		speedlower += 0.03;
	}
}
void decreasespeed()
{
	int i;
	for(i=1;i<10;i++)
	{
		if(speed[i] > 0.001 )
		{
			speed[i]-=0.02;
		}
	}
		if(speedlower > 0.02 )
		{
			speedlower-=0.02;
			speedupper -= 0.02;
		}
}
void moveRedBoxleft ()
{
	if(redbox_x > -7.2)
		redbox_x -= 0.15;
}
void moveRedBoxright()
{
	if(redbox_x < 7.2)
		redbox_x += 0.15;
}
void moveGreenBoxleft ()
{
	if(greenbox_x > -7.2)
		greenbox_x -= 0.15;
}
void moveGreenBoxright()
{
	if(greenbox_x < 7.2)
		greenbox_x += 0.15;
}
void movecannonup()
{
	if(laser1_y < 7.0)
		laser1_y += 0.2;
}
void movecannondown()
{
	if(laser1_y > -5.3)
		laser1_y -= 0.2;
}
void rotatecannonright()
{
	if(laser2_rotation > -70.0)
		laser2_rotation -= 3;
}
void rotatecannonleft()
{
	if(laser2_rotation < 70.0)
		laser2_rotation += 3;
}
/*void incrementx()
{

	laser_xlength++;

}*/
#define GLFW_KEY_RIGHT   262
#define GLFW_KEY_RIGHT_ALT   346
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
		// printf("release %d\n", key);
    if (action == GLFW_RELEASE) {
	//		printf("release %d\n", key);
        switch (key) {
							case GLFW_KEY_RIGHT_ALT:
								break;
							case GLFW_KEY_SPACE:
								current_time1 = glfwGetTime(); // Time in seconds
								if ((current_time1 - last_update_time1) >= 0.7)  // atleast 0.5s elapsed since last frame
								{	// do something every 0.5 seconds ..
									last_update_time1 = current_time1;
									laserflag=1;
									laserrotation = laser2_rotation ;
									lasery= laser1_y;
									laserx= laser1_x;
								}
								break;
          /*  case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;*/
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {

        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
						case GLFW_KEY_RIGHT://Press alt+left or right to move redbox to left or right respectively
																//Press control+left or right to move the greenbox to left or right respectively

							if(glfwGetKey(window , GLFW_KEY_RIGHT_ALT)==GLFW_PRESS)
							{
								//printf("pressed %d\n", key);
								moveRedBoxright();
							}
							else if(glfwGetKey(window , GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS)
									moveGreenBoxright();
							else
							{
								xpan+=0.2;
							}
							break;

						case GLFW_KEY_LEFT:
							if(glfwGetKey(window , GLFW_KEY_RIGHT_ALT)==GLFW_PRESS)
									moveRedBoxleft();
							else if(glfwGetKey(window , GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS)
									moveGreenBoxleft();
							else{
								xpan -= 0.2;
							}
							break;
						case GLFW_KEY_UP:
							zoom +=0.1;
							break;

						case GLFW_KEY_DOWN:
							zoom -= 0.1;
							break;
            default:
                break;
        }
    }
}


/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		case 's':
			movecannonup();
			break;

		case 'f':
			movecannondown();
			break;
		case 'a':
			rotatecannonleft();
			break;
		case 'd':
			rotatecannonright();
			break;
		case 'n':
			increasespeed();
			break;
		case 'm':
			decreasespeed();
			break;
		case 'r':
			gameflag=0;
			points=0;
			negativeflag=0;
			break;
		case 'R':
			gameflag=0;
			points=0;
			negativeflag=0;
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
						if (action== GLFW_PRESS)
						{
							mouseflag=1;
						}
            else if (action == GLFW_RELEASE)
            {
							mouseflag=0;
							if(xpos >50 && ypos <660)
							{
								current_time1 = glfwGetTime(); // Time in seconds
								if ((current_time1 - last_update_time1) >= 0.7)  // atleast 0.5s elapsed since last frame
								{	// do something every 0.5 seconds ..
									last_update_time1 = current_time1;
									laserflag=1;
									laserrotation = laser2_rotation ;
									lasery= laser1_y;
									laserx= laser1_x;
								}
							}
						}
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-(8.0f), (8.0f), (-8.0f), (8.0f), 0.1f, 500.0f);
}


//creating redbox
void createRedbox ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1*(collectingbox_xlength),-1*collectingbox_ylength,0, // vertex 1
    collectingbox_xlength ,-1* collectingbox_ylength,0, // vertex 2
    collectingbox_xlength, collectingbox_ylength,0, // vertex 3

    collectingbox_xlength, collectingbox_ylength,0, // vertex 3
    -1*collectingbox_xlength, collectingbox_ylength,0, // vertex 4
    -1*collectingbox_xlength,-1* collectingbox_ylength,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  redbox= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createGreenbox ()
{
  // GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1*(collectingbox_xlength),-1*collectingbox_ylength,0, // vertex 1
		collectingbox_xlength ,-1* collectingbox_ylength,0, // vertex 2
		collectingbox_xlength, collectingbox_ylength,0, // vertex 3

		collectingbox_xlength, collectingbox_ylength,0, // vertex 3
		-1*collectingbox_xlength, collectingbox_ylength,0, // vertex 4
		-1*collectingbox_xlength,-1* collectingbox_ylength,0  // vertex 1
	};

  static const GLfloat color_buffer_data [] = {
    0,0.5,0, // color 1
    0,0.5,0, // color 2
    0,0.5,0, // color 3

    0,0.5,0, // color 3
    0,0.5,0, // color 4
    0,0.5,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  greenbox= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float linexlength= 7;
float lineylength = 0.1;
VAO *line;
void createline ()
{
  // GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1*(linexlength),-1*lineylength,0, // vertex 1
		 linexlength,-1* lineylength,0, // vertex 2
	linexlength, lineylength,0, // vertex 3

		linexlength,lineylength,0, // vertex 3
		-1*linexlength, lineylength,0, // vertex 4
		-1*linexlength,-1* lineylength,0  // vertex 1
	};

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  line = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createlaser1 () //cannon part1
{
  // GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1*(laser1_xlength),-1*laser1_ylength,0, // vertex 1
		laser1_xlength ,-1* laser1_ylength,0, // vertex 2
		laser1_xlength, laser1_ylength,0, // vertex 3

		laser1_xlength, laser1_ylength,0, // vertex 3
		-1*laser1_xlength, laser1_ylength,0, // vertex 4
		-1*laser1_xlength,-1* laser1_ylength,0  // vertex 1
	};

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectlaser1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createlaser2 () //cannon part 2
{
  // GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		0 ,-1*laser2_ylength,0, // vertex 1
		laser2_xlength ,-1* laser2_ylength,0, // vertex 2
		laser2_xlength, laser2_ylength,0, // vertex 3

		laser2_xlength, laser2_ylength,0, // vertex 3
		0, laser2_ylength,0, // vertex 4
		0,-1* laser2_ylength,0  // vertex 1
	};

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectlaser2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}



void createlaser()
{
	static const GLfloat vertex_buffer_data [] = {
		1 ,laser_ylength,0, // vertex 1
		1,-1* laser_ylength,0, // vertex 2
		laser_xlength, laser_ylength,0, // vertex 3

		laser_xlength, laser_ylength,0, // vertex 3
		laser_xlength,-1* laser_ylength,0, // vertex 4
		1,-1* laser_ylength,0  // vertex 1
	};

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  laser = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
float brick_xlength = 0.2;
float brick_ylength = 0.3;
void createBrick1 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1*brick_xlength,-1* brick_ylength,0, // vertex 1
    brick_xlength,-1*brick_ylength,0, // vertex 2
    brick_xlength,brick_ylength,0, // vertex 3

    brick_xlength, brick_ylength,0, // vertex 3
    -1*brick_xlength, brick_ylength,0, // vertex 4
    -1*brick_xlength,-1*brick_ylength,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  brick1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBrick2 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1*brick_xlength,-1* brick_ylength,0, // vertex 1
    brick_xlength,-1*brick_ylength,0, // vertex 2
    brick_xlength,brick_ylength,0, // vertex 3

    brick_xlength, brick_ylength,0, // vertex 3
    -1*brick_xlength, brick_ylength,0, // vertex 4
    -1*brick_xlength,-1*brick_ylength,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  brick2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBrick3 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1*brick_xlength,-1* brick_ylength,0, // vertex 1
    brick_xlength,-1*brick_ylength,0, // vertex 2
    brick_xlength,brick_ylength,0, // vertex 3

    brick_xlength, brick_ylength,0, // vertex 3
    -1*brick_xlength, brick_ylength,0, // vertex 4
    -1*brick_xlength,-1*brick_ylength,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
		0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  brick3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO *mirror1 , *mirror2;
void createmirror1 ()
{
	// GL3 accepts only Triangles. Quads are not supported static
	const GLfloat vertex_buffer_data [] = {
		-1,0.05,0, // vertex 1
		-1,-0.05,0, // vertex 2
		1, -0.05,0, // vertex 3

		1, -0.05,0, // vertex 3
		1, 0.05,0, // vertex 4
		-1,0.05,0,  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1

		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	mirror1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createmirror2 ()
{
	// GL3 accepts only Triangles. Quads are not supported static
	const GLfloat vertex_buffer_data [] = {
		-1,0.05,0, // vertex 1
		-1,-0.05,0, // vertex 2
		1, -0.05,0, // vertex 3

		1, -0.05,0, // vertex 3
		1, 0.05,0, // vertex 4
		-1,0.05,0,  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1

		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1
		0.52f,0.8f,0.98f, // color 1

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	mirror2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
struct VAO* scorehorizontal;
void createscorehorizontal()
{
// GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.4,0,0, // vertex 2
    0.4, 0.10,0, // vertex 3

   0.4, 0.10,0, // vertex 3
    0, 0.10,0, // vertex 4
    0,0,0//i,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  scorehorizontal = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
struct VAO* scorevertical;
void createscorevertical()
{
// GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.07,0,0, // vertex 2
    0.07, 0.4,0, // vertex 3

   0.07, 0.4,0, // vertex 3
    0, 0.4,0, // vertex 4
    0,0,0//i,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  scorevertical = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float redbox_rotation =0 ;
float greenbox_rotation= 0;
float laser1_rotation=0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
  // clear the color and depth in the frame buffer
	if(gameflag==0)
	{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Matrices.projection = glm::ortho(-(8.0f)/zoom+xpan, (8.0f)/zoom+xpan, (-8.0f)/zoom+ypan, (8.0f)/zoom+ypan, 0.1f, 500.0f);
int i ;
	double angle ;
	for(i=1;i<10;i++)
	{
if(brick_y[i] >=10)
{
	//cout << "Hello\n";
	flag[i]=0;
}
}
if(mouseflag==1)
{

glfwGetCursorPos(window, &xpos, &ypos);
//cout << xpos << "\t" << ypos << "\n";
if(ypos > 660 )
{
	if((redbox_x - collectingbox_xlength)*50 <= (xpos -400) && (redbox_x + collectingbox_xlength)* 50 >= (xpos-400))
	{
		if( (xpos/50) <15 && (xpos/50)>1 )
		{
		redbox_x= (xpos -400)/50;
	}
	}
	else if( (greenbox_x- collectingbox_xlength)*50 <= (xpos-400) && (greenbox_x +collectingbox_xlength)*50 >= (xpos-400))
	{
		if( (xpos/50) <15 && (xpos/50)>1 )
		{
			greenbox_x= (xpos -400)/50;
		}
	}
}
else if(xpos <=50 && (400 - ypos) >= (laser1_y- laser1_ylength)*50 && (400 - ypos) <= (laser1_y + laser1_ylength)*50)
{
	if( (ypos)< 660 && ypos >25)
	laser1_y = (400 - ypos)/50 ;
}
else{
	angle = atan ((400 - ypos - (laser1_y*50))/xpos);
	angle *= 180/M_PI;
}
laser2_rotation = angle ;
}
  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);
/*	if (laser_xlength < 6.4)
		cout << laser_xlength++;*/
  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

	//mirror1
	float mirror1x =6.0 , mirror1y= 0.0;
	float mirror2x = -4.0 , mirror2y = -4.5;
	float mirror1rotation = 90;
	float mirror2rotation = 120;
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translatemirror1 = glm::translate (glm::vec3(mirror1x, mirror1y, 0.0f)); // glTranslatef
		glm::mat4 rotatemirror1 = glm::rotate((float)(mirror1rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= translatemirror1*rotatemirror1;
		MVP = VP * Matrices.model; // MVP = p * V * M
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		srand(time(NULL));
		draw3DObject(mirror1);

		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translatemirror2 = glm::translate (glm::vec3(mirror2x, mirror2y, 0.0f)); // glTranslatef
		glm::mat4 rotatemirror2 = glm::rotate((float)(mirror2rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		Matrices.model *= translatemirror2*rotatemirror2;
		MVP = VP * Matrices.model; // MVP = p * V * M
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		srand(time(NULL));
		draw3DObject(mirror2);
	//	cout << laserrotation << endl;
//cout << lasery << "\t" << mirror1y+ sin(mirror1rotation) << endl;
		if(lasery+abs(sin(laserrotation*M_PI/180)) <( mirror1y + 1* sin(mirror1rotation*M_PI/180.0f)) && ( lasery+abs(sin(laserrotation*M_PI/180)) > mirror1y -1*sin(mirror1rotation*M_PI/180.0f)))
		{
		//	cout << "insidey\n";
			if(abs(mirror1x - laserx)<= laser_xlength + ( 0.5*cos(laserrotation*M_PI/180)))
			{
		//		cout << "insidex\n";
				laserrotation = 2*mirror1rotation -laserrotation;
			}
		}
		if((abs(mirror2x - laserx)<= laser_xlength +0.05) && (abs(mirror2y - lasery)<= laser_xlength+ 1 ))
		{
				laserrotation = 2*120 -laserrotation;
		}
///////// creating the red box
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRedbox = glm::translate (glm::vec3(redbox_x, -7, 0));        // glTranslatef
  glm::mat4 rotateRedbox = glm::rotate((float)(redbox_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRedbox * rotateRedbox);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(redbox);


///////// creating the green box
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateGreenbox = glm::translate (glm::vec3(greenbox_x, -7, 0));        // glTranslatef
  glm::mat4 rotateGreenbox = glm::rotate((float)(redbox_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateGreenbox * rotateGreenbox);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(greenbox);

	///////// creating the laser1
	  Matrices.model = glm::mat4(1.0f);
	  glm::mat4 translaterectlaser1 = glm::translate (glm::vec3(laser1_x, laser1_y, 0));        // glTranslatef
	  glm::mat4 rotaterectlaser1 = glm::rotate((float)(laser1_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	  Matrices.model *= (translaterectlaser1 * rotaterectlaser1);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  draw3DObject(rectlaser1);

		///////// creating the laser2
		  Matrices.model = glm::mat4(1.0f);
		  glm::mat4 translaterectlaser2 = glm::translate (glm::vec3(laser1_x, laser1_y, 0));        // glTranslatef
		  glm::mat4 rotaterectlaser2 = glm::rotate((float)(laser2_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		  Matrices.model *= (translaterectlaser2 * rotaterectlaser2);
		  MVP = VP * Matrices.model;
		  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		  draw3DObject(rectlaser2);
//laser_xlength +=1.5;
			///////// creating the laser
			if(laserflag==1 )
			{
				if(laserx>=8 || laserx<=-8 || lasery >=8 || lasery<= -6)
				{
					//cout << "hello world\n";
					laserflag=0;
					laserx = -7;
				}

				Matrices.model = glm::mat4(1.0f);
glm::mat4 translatelaser = glm::translate (glm::vec3(laserx, lasery, 0));  // glTranslatef
		//		glm::mat4 translatelaser = glm::translate (glm::vec3(-7.4, lasery, 0));        // glTranslatef
				glm::mat4 rotatelaser = glm::rotate((float)(laserrotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= ( translatelaser *rotatelaser );
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(laser);

				for(i=1;i<10; i++)
				{
					if(lasery <= brick_y[i]+brick_ylength && lasery >= brick_y[i] - brick_ylength)
					{
						if(abs(brick_x[i]-laserx) <=( laser_xlength*cos(laserrotation*M_PI/180)+ brick_xlength) || abs(laserx - brick_x[i]) <=( abs(laser_xlength*cos(laserrotation*M_PI/180))+ brick_xlength))
						{
							brick_y[i] =10;
							flag[i]=0;
							laserflag=0;
							laserx = -7;
								if((i%3)==2 || (i%3)==0)
								{
									//if(points>=2)
										points -= 2; //decreasing points on hitting green or red brick
								}
								else
								{
									points += 3;  //increasing points on hitting black brick
								}
						//	cout << "hit\n";
							break;
						}
					}
				}

				laserx+= 0.5 *cos(laserrotation*M_PI/180); //speed * cos to get distance in x direction
				lasery+= 0.5 *sin(laserrotation*M_PI/180);
				if(laserx>=8 || laserx<=-8 || lasery >=8 || lasery<= -6)
				{
					//cout << "hello world\n";
					laserflag=0;
					laserx = -7;
				}

			}
//creating brick1
		//		laserx+= 0.001;
h[1] =-5.0,h[2]=-0.99 , h[3]=3.01 ;
g[1]=-1 ,g[2] = 3.00 ,g[3]=7.00 ;

for(i=1;i<10;i++)
{
				if(flag[i]==0)
				{
					flag[i]=1;
					int j = ((i-1)/3) +1;
					/*srand (time(NULL));
				brick_x[i] = rand() %12 -6;*/
				speed[i]= 0.01 +static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/speedupper - speedlower));
				brick_x[i]= h[j]+ static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(g[j]-h[j])));
				//cout << speed[i] << endl;
		//		cout << "hello again\n" << brick_x[1];
			}
		}
				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick1 = glm::translate (glm::vec3(brick_x[1], brick_y[1], 0));        // glTranslatef
				glm::mat4 rotatebrick1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick1 * rotatebrick1);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick1);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick2 = glm::translate (glm::vec3(brick_x[2], brick_y[2], 0));        // glTranslatef
				glm::mat4 rotatebrick2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick2 * rotatebrick2);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick2);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick3 = glm::translate (glm::vec3(brick_x[3], brick_y[3], 0));        // glTranslatef
				glm::mat4 rotatebrick3 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick3 * rotatebrick3);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick3);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick4 = glm::translate (glm::vec3(brick_x[4], brick_y[4], 0));        // glTranslatef
				glm::mat4 rotatebrick4 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick4 * rotatebrick4);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick1);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick5 = glm::translate (glm::vec3(brick_x[5], brick_y[5], 0));        // glTranslatef
				glm::mat4 rotatebrick5 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick5 * rotatebrick5);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick2);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick6 = glm::translate (glm::vec3(brick_x[6], brick_y[6], 0));        // glTranslatef
				glm::mat4 rotatebrick6 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick6 * rotatebrick6);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick3);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick7 = glm::translate (glm::vec3(brick_x[7], brick_y[7], 0));        // glTranslatef
				glm::mat4 rotatebrick7 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick7 * rotatebrick7);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick1);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick8 = glm::translate (glm::vec3(brick_x[8], brick_y[8], 0));        // glTranslatef
				glm::mat4 rotatebrick8 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick8 * rotatebrick8);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick2);

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translatebrick9 = glm::translate (glm::vec3(brick_x[9], brick_y[9], 0));        // glTranslatef
				glm::mat4 rotatebrick9 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translatebrick9 * rotatebrick9);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(brick3);


				for(i=1;i<10 ;i++)
				{
					if(brick_x[i] > (redbox_x- collectingbox_xlength)  && brick_x[i] < (redbox_x + collectingbox_xlength) )
					{
						if(abs(-7 - brick_y[i] ) < (collectingbox_ylength + brick_ylength))
						{
							if((i%3)==2)
							{
								points += 2;
				//				cout << points << "red" << endl;
							}
							brick_y[i] =10;
							flag[i]=0;
							if(i%3==1)
							{
							//	exit(EXIT_SUCCESS);
									gameflag=1;
								//terminate the game i.e gameover
							}
						}
					}
					else if(brick_x[i] > (greenbox_x- collectingbox_xlength)  && brick_x[i] < (greenbox_x + collectingbox_xlength) )
					{
						if(abs(-7 - brick_y[i]) < (collectingbox_ylength + brick_ylength))
						{
							if(i%3==0)
							{
								points += 2;
			//					cout << points << "green" << endl;
							}
							brick_y[i] =10;
							flag[i]=0;
							if(i%3==1)
							{
								gameflag=1;
								//terminate the game i.e gameover
							}
						}
					}
				if(brick_y[i] > -7 )
					brick_y[i] -=speed[i];
				else{
					brick_y[i] =10;
					flag[i]=0;
				}
			}
int a ,b;
int temppoints;
temppoints =points;
if(temppoints<0)
{
	temppoints = abs(temppoints);
	negativeflag=1;
}
else
	negativeflag=0;
b=temppoints%10;
a=temppoints/10;
switch(a)
{
case 0:
	f21=1;
	f22=1;
	f23=1;
	f24=1;
	f25=1;
	f26=1;
	f27=0;
	break;
case 1:
	f21=0;
	f22=1;
	f23=1;
	f24=0;
	f25=0;
	f26=0;
	f27=0;
	break;
case 2:
	f21=1;
	f22=1;
	f23=0;
	f24=1;
	f25=1;
	f26=0;
	f27=1;
	break;
case 3:
	f21=1;
	f22=1;
	f23=1;
	f24=1;
	f25=0;
	f26=0;
	f27=1;
	break;
case 4:
	f21=0;
	f22=1;
	f23=1;
	f24=0;
	f25=0;
	f26=1;
	f27=1;
	break;
case 5:
	f21=1;
	f22=0;
	f23=1;
	f24=1;
	f25=0;
	f26=1;
	f27=1;
	break;
case 6:
	f21=1;
	f22=0;
	f23=1;
	f24=1;
	f25=1;
	f26=1;
	f27=1;
	break;
case 7:
	f21=1;
	f22=1;
	f23=1;
	f24=0;
	f25=0;
	f26=0;
	f27=0;
	break;
case 8:
	f21=1;
	f22=1;
	f23=1;
	f24=1;
	f25=1;
	f26=1;
	f27=1;
	break;
case 9:
	f21=1;
	f22=1;
	f23=1;
	f24=0;
	f25=0;
	f26=1;
	f27=1;
	break;
default:
	break;
}
switch(b)
{
case 0:
	f11=1;
	f12=1;
	f13=1;
	f14=1;
	f15=1;
	f16=1;
	f17=0;
	break;
case 1:
	f11=1;
	f12=1;
	f13=1;
	f14=1;
	f15=1;
	f16=1;
	f17=0;
	break;
case 2:
	f11=1;
	f12=1;
	f13=0;
	f14=1;
	f15=1;
	f16=0;
	f17=1;
	break;
case 3:
	f11=1;
	f12=1;
	f13=1;
	f14=1;
	f15=0;
	f16=0;
	f17=1;
	break;
case 4:
	f11=0;
	f12=1;
	f13=1;
	f14=0;
	f15=0;
	f16=1;
	f17=1;
	break;
case 5:
	f11=1;
	f12=0;
	f13=1;
	f14=1;
	f15=0;
	f16=1;
	f17=1;
	break;
case 6:
	f11=1;
	f12=0;
	f13=1;
	f14=1;
	f15=1;
	f16=1;
	f17=1;
	break;
case 7:
	f11=1;
	f12=1;
	f13=1;
	f14=0;
	f15=0;
	f16=0;
	f17=0;
	break;
case 8:
	f11=1;
	f12=1;
	f13=1;
	f14=1;
	f15=1;
	f16=1;
	f17=1;
	break;
case 9:
	f11=1;
	f12=1;
	f13=1;
	f14=0;
	f15=0;
	f16=1;
	f17=1;
	break;
default:
	break;
}

if(f24)
{
/*scorehorizontal1*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate1 = glm::translate (glm::vec3(6.2, 6.3, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorehorizontal);}
if(f27){
/*scorehorizontal2*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate2 = glm::translate (glm::vec3(6.3, 6.9, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorehorizontal);}
if(f21){
/*scorehorizontal3*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate3 = glm::translate (glm::vec3(6.2, 7.5, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate3);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorehorizontal);}
if(f25){
/*scorevertical1*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate11 = glm::translate (glm::vec3(6.1, 6.4, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate11);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(f26){
/*scorevertical2*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate12= glm::translate (glm::vec3(6.1, 7.0, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate12);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(f23){
/*scorevertical4*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate14 = glm::translate (glm::vec3(6.8, 6.4, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate14);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(f22){
/*scorevertical3*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate13= glm::translate (glm::vec3(6.8, 7.0, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate13);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(f14){
/*scorehorizontal4*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate4 = glm::translate (glm::vec3(7.2, 6.3, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate4);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorehorizontal);}
if(f17){
/*scorehorizontal5*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate5 = glm::translate (glm::vec3(7.2, 6.9, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate5);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorehorizontal);}
if(f11){
/*scorehorizontal6*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate6 = glm::translate (glm::vec3(7.2, 7.5, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate6);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorehorizontal);}
if(f15){
/*scorevertical21*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate21 = glm::translate (glm::vec3(7.1, 6.4, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate21);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(f16){
/*scorevertical22*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate22= glm::translate (glm::vec3(7.1, 7.0, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate22);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(f13){
/*scorevertical24*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate24 = glm::translate (glm::vec3(7.8, 6.4, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate24);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(f12){
/*scorevertical23*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate23= glm::translate (glm::vec3(7.8, 7.0, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate23);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(scorevertical);}
if(negativeflag){
	/*negative*/ Matrices.model = glm::mat4(1.0f);

	  glm::mat4 translate5 = glm::translate (glm::vec3(5.4, 6.9, 0));        // glTranslatef
	  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	  Matrices.model *= (translate5);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(scorehorizontal);
}
	  // draw3DObject draws the VAO given to it using current MVP matrix

  // Load identity to model matrix
/*  Matrices.model = glm::mat4(1.0f);
  Render your scene

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
*/
  // Increment angles
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateline = glm::translate (glm::vec3(0, -6.0, 0));        // glTranslatef
	glm::mat4 rotateline = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateline * rotateline);
	MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(line);

  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "BRICK BREAKER - 201502172", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();*/
createRedbox();
createGreenbox();
createlaser1();
createlaser2();
if(laserflag==0)
	createlaser();
createBrick1();
createBrick2();
createBrick3();

createline();
createmirror1();
createmirror2();
createscorehorizontal();
createscorevertical();

//laser_xlength ++;
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 800;
	int height = 800;
	laserflag=0;
//	speed[1]= 0.01, speed[2]= 0.013, speed[3]= 0.016 ,speed[4]=0.008, speed[5]= 0.015, speed[6]= 0.011, speed[7]= 0.012, speed[8]= ;
int i ;
for(i=0;i<10;i++)
{
	flag[i]=0;
	brick_y[i]=10;
}
speedlower = 0.03 ;
speedupper = 0.06 ;
    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
       if ((current_time - last_update_time) >= 0.1) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
