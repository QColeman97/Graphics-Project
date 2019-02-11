/*
CPE/CSC 471 Quinn Coleman Quarter Proj from Framebuffer Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

#include <stdlib.h>
#include <time.h>

#define DAY 0
#define NIGHT 1

#define NO_KEY 0
#define W_KEY  1
#define A_KEY  2
#define S_KEY  3
#define D_KEY  4

#define NORM_FRONT -1
#define NORM_BACK 1

#define CYL_HEIGHT 4

#define ROOM_ROWS 4
#define ROOM_COLS 5
#define START_ROW 2
#define START_COL 2
/* Room states */
#define NO_ROOM 0
/* built room states */
#define L_DOOR 1
#define U_DOOR 2
#define R_DOOR 3
#define D_DOOR 4
#define LU_DOOR 5
#define UR_DOOR 6
#define RD_DOOR 7
#define DL_DOOR 8
#define LUR_DOOR 9
#define URD_DOOR 10
#define RDL_DOOR 11
#define DLU_DOOR 12

#define TRI_DOOR_SIZE (6*80*(CYL_HEIGHT/2) + 6*50*(CYL_HEIGHT/2) + 2*3*80)
#define TWO_DOOR_SIZE (6*80*(CYL_HEIGHT/2) + 6*60*(CYL_HEIGHT/2) + 2*3*80)
#define ONE_DOOR_SIZE (6*80*(CYL_HEIGHT/2) + 6*70*(CYL_HEIGHT/2) + 2*3*80)

// Modified for no roof
//#define TRI_DOOR_SIZE (6*80*(CYL_HEIGHT/2) + 6*50*(CYL_HEIGHT/2) + 3*80)
//#define TWO_DOOR_SIZE (6*80*(CYL_HEIGHT/2) + 6*60*(CYL_HEIGHT/2) + 3*80)
//#define ONE_DOOR_SIZE (6*80*(CYL_HEIGHT/2) + 6*70*(CYL_HEIGHT/2) + 3*80)

#define ROOM_BUFFER_SIZE (TRI_DOOR_SIZE + TWO_DOOR_SIZE + ONE_DOOR_SIZE)
#define CYL_BUFFER_SIZE (6*80*CYL_HEIGHT + 2*3*80)

// MIGHT BE BACKWARDS? Drew 1 door when wanted 3
#define TRI_DRAW_OFFSET 0
#define TWO_DRAW_OFFSET (TRI_DOOR_SIZE)
#define ONE_DRAW_OFFSET (TRI_DOOR_SIZE + TWO_DOOR_SIZE)

// TODO: Add LURD_DOOR b/c its needed OR make so never happens in init
// Feature to add: LR_DOOR?
int l_legal[] = {L_DOOR, LU_DOOR, DL_DOOR, LUR_DOOR, DLU_DOOR, RDL_DOOR};
int r_legal[] = {R_DOOR, UR_DOOR, RD_DOOR, URD_DOOR, RDL_DOOR, LUR_DOOR};
int u_legal[] = {U_DOOR, LU_DOOR, UR_DOOR, LUR_DOOR, URD_DOOR, DLU_DOOR};
int d_legal[] = {D_DOOR, RD_DOOR, DL_DOOR, URD_DOOR, RDL_DOOR, DLU_DOOR};

int lu_legal[] = {LU_DOOR, LUR_DOOR, DLU_DOOR};
int ur_legal[] = {UR_DOOR, LUR_DOOR, URD_DOOR};
int rd_legal[] = {RD_DOOR, RDL_DOOR, URD_DOOR};
int dl_legal[] = {DL_DOOR, DLU_DOOR, RDL_DOOR};

int lur_legal[] = {LUR_DOOR};
int urd_legal[] = {URD_DOOR};
int rdl_legal[] = {RDL_DOOR};
int dlu_legal[] = {DLU_DOOR};

int floor_properties[ROOM_ROWS][ROOM_COLS];
glm::vec3 room_coords[ROOM_ROWS][ROOM_COLS];

shared_ptr<Shape> car_shape, sphere_shape, plane_shape;
// Quinn's change
std::string resourceDir = "../../resources"; // Where the resources are loaded from

//glm::vec3 lightPos = glm::vec3(0, -0.12, 0.05);
glm::vec3 lightPos = glm::vec3(0, 0.12, -0.05);

int light_mode = DAY;

float camera_rad = 0.2;
float platform_rad = camera_rad * 3;
float room_rad = platform_rad * 3.6;

int latest_dir_key = NO_KEY, disabled_dir_key = NO_KEY;
bool collision = false;
bool lastFrameCollision = false;
bool othersPressed = false;

// Function definitions
void initRoomSides(vec3 room_vertex_buffer_data[], int index, int height, int rooms);
void initCylFace(vec3 room_vertex_buffer_data[], int index, float height);
void initFaceNormals(vec3 normal_buffer_data[], int index, int face);
void initSideNormals(vec3 room_normal_buffer_data[], int index, int height, int doors);

int random_int_in_range(int from, int to)
{
    return (rand() % to + from);
}

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		rot = glm::vec3(0, 0, 0);
		//pos = glm::vec3(0, 0, -10);
        pos = glm::vec3(0,0.12,-1.5);

	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
            //speed = 2*ftime;

            speed = 30*ftime;
		}
		else if (s == 1)
		{
			speed = -30*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -30*ftime;
		else if(d==1)
			yangle = 30*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> car_prog, room_prog, cyl_prog, light_prog, prog_framebuffer, plane_prog;
    std::shared_ptr<Program> car_depth_prog, room_depth_prog, cyl_depth_prog;

    //texture data
    GLuint FBOtex;
    // The framebuffer (data), which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    GLuint FrameBufferObj;//, depth_rb;
    
    // Contains vertex information for OpenGL
    GLuint RoomVertexArrayID, CylVertexArrayID, VertexArrayIDRect; //VertexBufferIDRect, VertexBufferTexRect;

    //GLuint DepthMatrixID;
    
	// Data necessary to give our box to OpenGL
	GLuint MeshPosID, MeshTexID, IndexBufferIDBox, RoomVertexBufferID, RoomVertexNormalIDBox, CylVertexBufferID, CylVertexNormalIDBox;
    
	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
            // HANDLING COLLISION
            // Only focus on going back and forward keys really
            // Rotating is harmless on collision (a and d)
            if (disabled_dir_key != W_KEY) {
                mycam.w = 1;
                latest_dir_key = W_KEY;
                if (disabled_dir_key != NO_KEY)
                    othersPressed = true;
            }
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
            if (disabled_dir_key != S_KEY) {
                mycam.s = 1;
                latest_dir_key = S_KEY;
                if (disabled_dir_key != NO_KEY)
                    othersPressed = true;
            }
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
            //collision = false;  //?
            mycam.a = 1;
            latest_dir_key = A_KEY;
            if (disabled_dir_key != NO_KEY)
                othersPressed = true;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
            //collision = false;  //?
            mycam.d = 1;
            latest_dir_key = D_KEY;
            if (disabled_dir_key != NO_KEY)
                othersPressed = true;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
        if (key == GLFW_KEY_L && action == GLFW_PRESS)
        {
            if (light_mode == DAY)
                light_mode = NIGHT;
            else
                light_mode = DAY;
        }
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

    /*void initCylFace(vec3 room_vertex_buffer_data[], int index) {
        // 3 DOOR ROOM
        // Front face
        glm::vec3 initial_vec3 = glm::vec3(50,50,50);
        glm::vec3 prev_vec3 = initial_vec3;
        float alpha = 0;
        float d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        float rad = 0.2;
        int z_flag = 1;
        //int index = 0;
        
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                room_vertex_buffer_data[i] = glm::vec3(0,0,0);
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,0);
                room_vertex_buffer_data[i] = vector;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 1;
                }
                room_vertex_buffer_data[i] = vector;
                prev_vec3 = vector;
            }
            else {  // Use previous value
                room_vertex_buffer_data[i] = prev_vec3;
                prev_vec3 = initial_vec3;
            }
            index++;
        }
    }*/
    
	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
        // Car
        // Initialize mesh.
        car_shape = std::make_shared<Shape>();
        std::string mat = resourceDir + "/";
        car_shape->loadMesh(resourceDir + "/Lamborghini_Aventador.obj", &mat, stbi_load);
        car_shape->resize();
        car_shape->init();

        // Headlight
        // Initialize mesh.
        sphere_shape = std::make_shared<Shape>();
        sphere_shape->loadMesh(resourceDir + "/sphere.obj");
        sphere_shape->resize();
        sphere_shape->init();
        
        plane_shape = std::make_shared<Shape>();
        plane_shape->loadMesh(resourceDir + "/plane.obj");
        plane_shape->resize();
        plane_shape->init();
        
		/********************************************************/
		
        // Rooms
        //generate the cube VAO
        glGenVertexArrays(1, &RoomVertexArrayID);
        glBindVertexArray(RoomVertexArrayID);
        
        // ROOM VERTICES
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &RoomVertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, RoomVertexBufferID);
        
        glm::vec3 room_vertex_buffer_data[ROOM_BUFFER_SIZE];
        int index = 0;
        // Consider making a 3-iter outer loop
        
        // 3 DOOR ROOM
        /*//initCylFace(room_vertex_buffer_data, index, 0);
        cout << "index: " << index << endl;
        initRoomSides(room_vertex_buffer_data, index, CYL_HEIGHT, 3);
        cout << "index: " << index << endl;
        initCylFace(room_vertex_buffer_data, index, CYL_HEIGHT);
        cout << "index: " << index << endl;

        // 2 DOOR ROOM
        //initCylFace(room_vertex_buffer_data, index, 0);
        cout << "index: " << index << endl;
        initRoomSides(room_vertex_buffer_data, index, CYL_HEIGHT, 2);
        cout << "index: " << index << endl;
        initCylFace(room_vertex_buffer_data, index, CYL_HEIGHT);
        cout << "index: " << index << endl;
        
        // 1 DOOR ROOM
        //initCylFace(room_vertex_buffer_data, index, 0);
        cout << "index: " << index << endl;
        initRoomSides(room_vertex_buffer_data, index, CYL_HEIGHT, 1);
        cout << "index: " << index << endl;
        initCylFace(room_vertex_buffer_data, index, CYL_HEIGHT);
        cout << "index: " << index << endl;*/
        
        // Front face
        glm::vec3 initial_vec3 = glm::vec3(50,50,50);
        glm::vec3 prev_vec3 = initial_vec3;
        float alpha = 0;
        float d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        float rad = 0.2;
        int z_flag = 1;
        //int index = 0;
        
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                room_vertex_buffer_data[i] = glm::vec3(0,0,0);
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,0);
                room_vertex_buffer_data[i] = vector;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 1;
                }
                room_vertex_buffer_data[i] = vector;
                prev_vec3 = vector;
            }
            else {  // Use previous value
                room_vertex_buffer_data[i] = prev_vec3;
                prev_vec3 = initial_vec3;
            }
            index++;
        }
        
        // Sides
        float row_height = 0;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++, row_height += 0.1) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                //skip iteration if door conditions
                if (((15 <= j && j < 25) || (35 <= j && j < 45) ||
                     (55 <= j && j < 65)) && (CYL_HEIGHT/2) <= i) {
                    continue;
                }
                for (int k = 0; k < 6; k++) {
                    
                    glm::vec3 vector;
                    switch(k) {
                        case 0:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height);
                            break;
                        case 1:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                        case 2:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 3:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height + 0.1);
                            break;
                        case 4:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 5:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                    }
                    room_vertex_buffer_data[index++] = vector;
                }
            }
        }
        
        // Back face
        initial_vec3 = glm::vec3(50,50,50);
        prev_vec3 = initial_vec3;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        rad = 0.2;
        z_flag = 1;
        
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                room_vertex_buffer_data[index++] = glm::vec3(0,0,CYL_HEIGHT*0.1f);
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,CYL_HEIGHT*0.1f);
                room_vertex_buffer_data[index++] = vector;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 1;
                }
                room_vertex_buffer_data[index++] = vector;
                prev_vec3 = vector;
            }
            else {  // Use previous value
                room_vertex_buffer_data[index++] = prev_vec3;
                prev_vec3 = initial_vec3;
            }
        }
        
        
        // 2 DOOR ROOM
        // Front face
        initial_vec3 = glm::vec3(50,50,50);
        prev_vec3 = initial_vec3;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        rad = 0.2;
        z_flag = 1;
        // don't overwrite
        //index = 0;
        
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                room_vertex_buffer_data[index++] = glm::vec3(0,0,0);
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,0);
                room_vertex_buffer_data[index++] = vector;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 1;
                }
                room_vertex_buffer_data[index++] = vector;
                prev_vec3 = vector;
            }
            else {  // Use previous value
                room_vertex_buffer_data[index++] = prev_vec3;
                prev_vec3 = initial_vec3;
            }
        }
        
        // Sides
        row_height = 0;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++, row_height += 0.1) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                //skip iteration if door conditions
                if (((15 <= j && j < 25) || (35 <= j && j < 45)) &&
                    (CYL_HEIGHT/2) <= i) {
                    continue;
                }
                for (int k = 0; k < 6; k++) {
                    
                    glm::vec3 vector;
                    switch(k) {
                        case 0:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height);
                            break;
                        case 1:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                        case 2:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 3:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height + 0.1);
                            break;
                        case 4:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 5:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                    }
                    room_vertex_buffer_data[index++] = vector;
                }
            }
        }
        
        // Back face
        initial_vec3 = glm::vec3(50,50,50);
        prev_vec3 = initial_vec3;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        rad = 0.2;
        z_flag = 1;
        
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                room_vertex_buffer_data[index++] = glm::vec3(0,0,CYL_HEIGHT*0.1f);
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,CYL_HEIGHT*0.1f);
                room_vertex_buffer_data[index++] = vector;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 1;
                }
                room_vertex_buffer_data[index++] = vector;
                prev_vec3 = vector;
            }
            else {  // Use previous value
                room_vertex_buffer_data[index++] = prev_vec3;
                prev_vec3 = initial_vec3;
            }
        }
        
        
        // 1 DOOR ROOM
        // Front face
        initial_vec3 = glm::vec3(50,50,50);
        prev_vec3 = initial_vec3;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        rad = 0.2;
        z_flag = 1;
        //index = 0;
        
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                room_vertex_buffer_data[index++] = glm::vec3(0,0,0);
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,0);
                room_vertex_buffer_data[index++] = vector;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 1;
                }
                room_vertex_buffer_data[index++] = vector;
                prev_vec3 = vector;
            }
            else {  // Use previous value
                room_vertex_buffer_data[index++] = prev_vec3;
                prev_vec3 = initial_vec3;
            }
            //index++;
        }
        
        // Sides
        row_height = 0;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++, row_height += 0.1) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                //skip iteration if door conditions
                if ((15 <= j && j < 25) && (CYL_HEIGHT/2) <= i) {
                    continue;
                }
                for (int k = 0; k < 6; k++) {
                    
                    glm::vec3 vector;
                    switch(k) {
                        case 0:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height);
                            break;
                        case 1:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                        case 2:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 3:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height + 0.1);
                            break;
                        case 4:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 5:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                    }
                    room_vertex_buffer_data[index++] = vector;
                }
            }
        }
        
        // Back face
        initial_vec3 = glm::vec3(50,50,50);
        prev_vec3 = initial_vec3;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        rad = 0.2;
        z_flag = 1;
        
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                room_vertex_buffer_data[index++] = glm::vec3(0,0,CYL_HEIGHT*0.1f);
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,CYL_HEIGHT*0.1f);
                room_vertex_buffer_data[index++] = vector;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 1;
                }
                room_vertex_buffer_data[index++] = vector;
                prev_vec3 = vector;
            }
            else {  // Use previous value
                room_vertex_buffer_data[index++] = prev_vec3;
                prev_vec3 = initial_vec3;
            }
        }
        
        //std::cout << "NUM VERTICES ROOM: " << index << "ROOM_BUFFER_SIZE: " << ROOM_BUFFER_SIZE << std::endl;
        
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*ROOM_BUFFER_SIZE, room_vertex_buffer_data, GL_DYNAMIC_DRAW);
        
        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        
        
        // ROOM NORMALS
        glGenBuffers(1, &RoomVertexNormalIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, RoomVertexNormalIDBox);
        glm::vec3 room_normal_buffer_data[ROOM_BUFFER_SIZE];
        
        // 3 DOOR ROOM
        // Front face
        index = 0;
        //initFaceNormals(room_normal_buffer_data, index, NORM_FRONT);
        /*initSideNormals(room_normal_buffer_data, index, CYL_HEIGHT, 3);
        initFaceNormals(room_normal_buffer_data, index, NORM_BACK);
        
        //initFaceNormals(room_normal_buffer_data, index, NORM_FRONT);
        initSideNormals(room_normal_buffer_data, index, CYL_HEIGHT, 2);
        initFaceNormals(room_normal_buffer_data, index, NORM_BACK);
        
        //initFaceNormals(room_normal_buffer_data, index, NORM_FRONT);
        initSideNormals(room_normal_buffer_data, index, CYL_HEIGHT, 1);
        initFaceNormals(room_normal_buffer_data, index, NORM_BACK);*/
        
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            room_normal_buffer_data[i] = glm::vec3(0,0,-1);
            index++;
        }
        
        // Sides
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                //skip iteration if door conditions
                if (((15 <= j && j < 25) || (35 <= j && j < 45) ||
                     (55 <= j && j < 65)) && (CYL_HEIGHT/2) <= i) {
                    continue;
                }
                
                glm::vec3 norm_vector;
                
                glm::vec3 xy_pos = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                glm::vec3 xy_pos_next = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, 0);
                
                
                glm::vec3 tan = xy_pos - xy_pos_next;
                //glm::vec3 tan = xy_pos + xy_pos_next;
                glm::vec3 parallel = glm::vec3(0,0,-1);
                
                norm_vector = glm::vec3((tan.y*parallel.z)-(tan.z*parallel.y), (tan.z*parallel.x)-(tan.x*parallel.z), (tan.x*parallel.y)-(tan.y*parallel.x));
                
                norm_vector = normalize(norm_vector);
                
                std::cout << "norm_vector: " << norm_vector.x  << "," << norm_vector.y << "," << norm_vector.z << std::endl;
                std::cout << "Cyl Height: " << i << std::endl;
                
                for (int k = 0; k < 6; k++) {
                    room_normal_buffer_data[index++] = norm_vector;
                }
            }
        }
        
        // Back face
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            room_normal_buffer_data[index++] = glm::vec3(0,0,1);
        }
        
        
        // 2 DOOR ROOM
        // Front face
        //index = 0;
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            room_normal_buffer_data[index++] = glm::vec3(0,0,-1);
            //index++;
        }
        
        // Sides
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                //skip iteration if door conditions
                if (((15 <= j && j < 25) || (35 <= j && j < 45)) && (CYL_HEIGHT/2) <= i) {
                    continue;
                }
                
                glm::vec3 norm_vector;
                
                glm::vec3 xy_pos = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                glm::vec3 xy_pos_next = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, 0);
                
                
                glm::vec3 tan = xy_pos - xy_pos_next;
                //glm::vec3 tan = xy_pos + xy_pos_next;
                glm::vec3 parallel = glm::vec3(0,0,-1);
                
                norm_vector = glm::vec3((tan.y*parallel.z)-(tan.z*parallel.y), (tan.z*parallel.x)-(tan.x*parallel.z), (tan.x*parallel.y)-(tan.y*parallel.x));
                
                norm_vector = normalize(norm_vector);
                
                std::cout << "norm_vector: " << norm_vector.x  << "," << norm_vector.y << "," << norm_vector.z << std::endl;
                std::cout << "Cyl Height: " << i << std::endl;
                
                for (int k = 0; k < 6; k++) {
                    room_normal_buffer_data[index++] = norm_vector;
                }
            }
        }
        
        // Back face
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            room_normal_buffer_data[index++] = glm::vec3(0,0,1);
        }
        
        
        // 1 DOOR ROOM
        // Front face
        //index = 0;
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            room_normal_buffer_data[index++] = glm::vec3(0,0,-1);
            //index++;
        }
        
        // Sides
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                //skip iteration if door conditions
                if ((15 <= j && j < 25) && (CYL_HEIGHT/2) <= i) {
                    continue;
                }
                
                glm::vec3 norm_vector;
                
                glm::vec3 xy_pos = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                glm::vec3 xy_pos_next = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, 0);
                
                
                glm::vec3 tan = xy_pos - xy_pos_next;
                //glm::vec3 tan = xy_pos + xy_pos_next;
                glm::vec3 parallel = glm::vec3(0,0,-1);
                
                norm_vector = glm::vec3((tan.y*parallel.z)-(tan.z*parallel.y), (tan.z*parallel.x)-(tan.x*parallel.z), (tan.x*parallel.y)-(tan.y*parallel.x));
                
                norm_vector = normalize(norm_vector);
                
                std::cout << "norm_vector: " << norm_vector.x  << "," << norm_vector.y << "," << norm_vector.z << std::endl;
                std::cout << "Cyl Height: " << i << std::endl;
                
                for (int k = 0; k < 6; k++) {
                    room_normal_buffer_data[index++] = norm_vector;
                }
            }
        }
        
        // Back face
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            room_normal_buffer_data[index++] = glm::vec3(0,0,1);
        }
        
        std::cout << "NUM NORMALS ROOM: " << index << "ROOM_BUFFER_SIZE: " << ROOM_BUFFER_SIZE << std::endl;
        
        //sizeof(glm::vec3)*(6*80*CYL_HEIGHT + 2*3*80) - edit for doors
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*ROOM_BUFFER_SIZE, room_normal_buffer_data, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        /***********************************************************/
        //generate the cylinder VAO
        glGenVertexArrays(1, &CylVertexArrayID);
        glBindVertexArray(CylVertexArrayID);
        
        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &CylVertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, CylVertexBufferID);
        
        glm::vec3 cylinder_vertex_buffer_data[CYL_BUFFER_SIZE];
        
        index = 0;
        /*initCylFace(cylinder_vertex_buffer_data, index, 0);
        initRoomSides(cylinder_vertex_buffer_data, index, CYL_HEIGHT, 0);
        initCylFace(cylinder_vertex_buffer_data, index, CYL_HEIGHT);*/

        // NEW - front face
        //GLfloat cylinder_vertex_buffer_data[3*3*80];
        //glm::vec3 sun_data[3*80];
        initial_vec3 = glm::vec3(50,50,50);
        prev_vec3 = initial_vec3;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        rad = 0.2;
        z_flag = 1;
        index = 0;
        
        //for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                cylinder_vertex_buffer_data[i] = glm::vec3(0,0,0);
                //std::cout << "Entered vertex 000" << std::endl;
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                vector = glm::vec3(0,rad,0);
                cylinder_vertex_buffer_data[i] = vector;
                //std::cout << "Entered vertex 010" << std::endl;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    //vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0.2);
                    z_flag = 0;
                }
                else {
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    z_flag = 1;
                }
                cylinder_vertex_buffer_data[i] = vector;
                prev_vec3 = vector;
                //std::cout << "Entered new vertex" << std::endl;
            }
            else {  // Use previous value
                cylinder_vertex_buffer_data[i] = prev_vec3;
                prev_vec3 = initial_vec3;
                //std::cout << "Entered prev vertex" << std::endl;
            }
            index++;
        }
        // NEW - front face
        
        
        
        row_height = 0;
        //glm::vec3 cylinder_vertex_buffer_data[6*80*CYL_HEIGHT];
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        //int index = 0;
        
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++, row_height += 0.1) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                for (int k = 0; k < 6; k++) {
                    
                    
                    glm::vec3 vector;
                    
                    switch(k) {
                        case 0:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height);
                            break;
                        case 1:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                        case 2:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 3:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height + 0.1);
                            break;
                        case 4:
                            vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                            break;
                        case 5:
                            vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                            break;
                    }
                    cylinder_vertex_buffer_data[index++] = vector;
                }
            }
        }
        
        // NEW - back face
        //GLfloat cylinder_vertex_buffer_data[3*3*80];
        //sun_data[3*80];
        initial_vec3 = glm::vec3(50,50,50);
        prev_vec3 = initial_vec3;
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
        rad = 0.2;
        z_flag = 1;
        
        //for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
        // Iterate for every triangle
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            glm::vec3 vector;
            if (i % 3 == 0) {
                //cylinder_vertex_buffer_data[i] = glm::vec3(0,0,0);
                cylinder_vertex_buffer_data[i + index] = glm::vec3(0,0,CYL_HEIGHT*0.1f);
                //std::cout << "Entered vertex 000" << std::endl;
            }
            else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
                //vector = glm::vec3(0,rad,0);
                vector = glm::vec3(0,rad,CYL_HEIGHT*0.1f);
                cylinder_vertex_buffer_data[i + index] = vector;
                //std::cout << "Entered vertex 010" << std::endl;
            }
            else if (prev_vec3 == initial_vec3) {
                if (z_flag) {
                    //vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0.2);
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 0;
                }
                else {
                    //vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                    vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, CYL_HEIGHT*0.1f);
                    z_flag = 1;
                }
                cylinder_vertex_buffer_data[i + index] = vector;
                prev_vec3 = vector;
                //std::cout << "Entered new vertex" << std::endl;
            }
            else {  // Use previous value
                cylinder_vertex_buffer_data[i + index] = prev_vec3;
                prev_vec3 = initial_vec3;
                //std::cout << "Entered prev vertex" << std::endl;
            }
        }
        // NEW - back face
        
        std::cout << "NUM VERTICES CYL: " << (index + (3*80)) << "CYL_BUFFER_SIZE: " << CYL_BUFFER_SIZE << std::endl;
        
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*CYL_BUFFER_SIZE, cylinder_vertex_buffer_data, GL_DYNAMIC_DRAW);
        
        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        
        glGenBuffers(1, &CylVertexNormalIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, CylVertexNormalIDBox);
        
        glm::vec3 cylinder_normal_buffer_data[CYL_BUFFER_SIZE];
        
        // Front face
        index = 0;
        /*initFaceNormals(cylinder_normal_buffer_data, index, NORM_FRONT);
        initSideNormals(cylinder_normal_buffer_data, index, CYL_HEIGHT, 0);
        initFaceNormals(cylinder_normal_buffer_data, index, NORM_BACK);*/

        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            cylinder_normal_buffer_data[i] = glm::vec3(0,0,-1);
            index++;
        }
        
        // Sides
        alpha = 0;
        d_alpha = (2.0 * 3.1415926) / 80.0;
        rad = 0.2;
        // Iterate for every triangle
        for (int i = 0; i < CYL_HEIGHT; i++) {
            
            for (int j = 0; j < 80; j++, alpha += d_alpha) {
                
                glm::vec3 norm_vector;
                
                glm::vec3 xy_pos = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
                glm::vec3 xy_pos_next = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, 0);
                
                
                glm::vec3 tan = xy_pos - xy_pos_next;
                //glm::vec3 tan = xy_pos + xy_pos_next;
                glm::vec3 parallel = glm::vec3(0,0,-1);
                
                norm_vector = glm::vec3((tan.y*parallel.z)-(tan.z*parallel.y), (tan.z*parallel.x)-(tan.x*parallel.z), (tan.x*parallel.y)-(tan.y*parallel.x));
                
                norm_vector = normalize(norm_vector);
                
                std::cout << "norm_vector: " << norm_vector.x  << "," << norm_vector.y << "," << norm_vector.z << std::endl;
                std::cout << "Cyl Height: " << i << std::endl;
                
                for (int k = 0; k < 6; k++) {
                    cylinder_normal_buffer_data[index++] = norm_vector;
                }
            }
        }
        
        // Back face
        for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
            cylinder_normal_buffer_data[i + index] = glm::vec3(0,0,1);
        }
        
        std::cout << "NUM NORMALS CYL: " << (index + (3*80)) << "CYL_BUFFER_SIZE: " << CYL_BUFFER_SIZE << std::endl;
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*CYL_BUFFER_SIZE, cylinder_normal_buffer_data, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        
        
        // WHERE THE SHADOWMAPPING INITGEOM BEGINS
        
        int width, height, channels;
        char filepath[1000];
        
        //glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // The framebuffer
        glGenFramebuffers(1, &FrameBufferObj);
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObj);
        
		//Frame Buffer Object
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		//RGBA8 2D texture, 24 bit depth texture, 256x256
		
        // Depth texture
        glGenTextures(1, &FBOtex);
		glBindTexture(GL_TEXTURE_2D, FBOtex);
		/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
        
		//NULL means reserve texture memory, but texels are undefined
		//**** Tell OpenGL to reserve level 0
        
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        // SHADOW_MAPPING DIFF
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        
        // SHADOW_MAPPING DIFFERENCES
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        
        
		//You must reserve memory for other mipmaps levels as well either by making a series of calls to
		//glTexImage2D or use glGenerateMipmapEXT(GL_TEXTURE_2D).
		//Here, we'll use :
		//glGenerateMipmap(GL_TEXTURE_2D);
		//-------------------------
		//glGenFramebuffers(1, &FrameBufferObj);
		//glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObj);
		//Attach 2D texture to this FBO
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOtex, 0);
        // SHADOW_MAPPING DIFF
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FBOtex, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        
		//-------------------------
		/*glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);*/
		//-------------------------
		//Attach depth buffer to FBO
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
		//-------------------------
		//Does the GPU support current FBO configuration?
		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
			{
			case GL_FRAMEBUFFER_COMPLETE:
				cout << "status framebuffer: good";
				break;
			default:
				cout << "status framebuffer: bad!!!!!!!!!!!!!!!!!!!!!!!!!";
			}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //GLuint Tex1Location = glGetUniformLocation(prog_framebuffer->pid, "shadow_map_tex");
        GLuint Tex1Location = glGetUniformLocation(plane_prog->pid, "shadow_map_tex");
        //glUseProgram(prog_framebuffer->pid);
        glUseProgram(plane_prog->pid);
        glUniform1i(Tex1Location, 15);

        glBindVertexArray(0);
	}

    void initRooms()
    {
        // Initialized randomly generated room structure
        for (int i = 0; i < ROOM_ROWS; i++)
            for (int j = 0; j < ROOM_COLS; j++)
                floor_properties[i][j] = NO_ROOM;
        
        // Create randomly generated room structure
        floor_properties[START_ROW][START_COL] = LUR_DOOR;
        // LEFT DOOR ROOM SITUATION (ROOM TO THE RIGHT)
        int first_index = random_int_in_range(0,5);
        floor_properties[START_ROW][START_COL+1] = l_legal[first_index];
        // create additional rooms based on result:
        switch (floor_properties[START_ROW][START_COL+1]) {
            case LU_DOOR:
                floor_properties[START_ROW-1][START_COL+1] = D_DOOR;
                break;
            case DL_DOOR:
                floor_properties[START_ROW+1][START_COL+1] = U_DOOR;
                break;
            case LUR_DOOR:
                floor_properties[START_ROW-1][START_COL+1] = D_DOOR;
                floor_properties[START_ROW][START_COL+2] = L_DOOR;
                break;
            case DLU_DOOR:
                floor_properties[START_ROW+1][START_COL+1] = U_DOOR;
                floor_properties[START_ROW-1][START_COL+1] = D_DOOR;
                break;
            case RDL_DOOR:
                floor_properties[START_ROW][START_COL+2] = L_DOOR;
                floor_properties[START_ROW+1][START_COL+1] = U_DOOR;
                break;
            default:
                break;
        }
        
        // DOWN DOOR ROOM SITUATION
        first_index = random_int_in_range(0,5);
        
        if (floor_properties[START_ROW-1][START_COL+1])
            floor_properties[START_ROW-1][START_COL] = rd_legal[first_index];
        else
            floor_properties[START_ROW-1][START_COL] = d_legal[first_index];
        
        // create additional rooms based on result:
        //{RD_DOOR, DL_DOOR, URD_DOOR, RDL_DOOR, DLU_DOOR}
        switch (floor_properties[START_ROW-1][START_COL]) {
            case DL_DOOR:
                floor_properties[START_ROW-1][START_COL-1] = R_DOOR;
                break;
            case RD_DOOR:
                if (floor_properties[START_ROW-1][START_COL+1])
                    floor_properties[START_ROW-1][START_COL+1] = DL_DOOR;
                else
                    floor_properties[START_ROW-1][START_COL+1] = L_DOOR;
                break;
            case URD_DOOR:
                if (floor_properties[START_ROW-1][START_COL+1])
                    floor_properties[START_ROW-1][START_COL+1] = DL_DOOR;
                else
                    floor_properties[START_ROW-1][START_COL+1] = L_DOOR;
                floor_properties[START_ROW-2][START_COL] = D_DOOR;
                break;
            case RDL_DOOR:
                if (floor_properties[START_ROW-1][START_COL+1])
                    floor_properties[START_ROW-1][START_COL+1] = DL_DOOR;
                else
                    floor_properties[START_ROW-1][START_COL+1] = L_DOOR;
                floor_properties[START_ROW-1][START_COL-1] = R_DOOR;
                break;
            case DLU_DOOR:
                floor_properties[START_ROW-1][START_COL-1] = R_DOOR;
                floor_properties[START_ROW-2][START_COL] = D_DOOR;
                break;
            default:
                break;
        }
        
        
        // RIGHT DOOR ROOM SITUATION (ROOM TO THE LEFT)
        first_index = random_int_in_range(0,5);
        
        if (floor_properties[START_ROW-1][START_COL-1])
            floor_properties[START_ROW][START_COL-1] = ur_legal[first_index];
        else
            floor_properties[START_ROW][START_COL-1] = r_legal[first_index];
        
        // create additional rooms based on result:
        //{UR_DOOR, RD_DOOR, URD_DOOR, RDL_DOOR, LUR_DOOR}
        switch (floor_properties[START_ROW][START_COL-1]) {
            case UR_DOOR:
                if (floor_properties[START_ROW-1][START_COL-1])
                    floor_properties[START_ROW-1][START_COL-1] = RD_DOOR;
                else
                    floor_properties[START_ROW-1][START_COL-1] = D_DOOR;
                break;
            case RD_DOOR:
                floor_properties[START_ROW+1][START_COL-1] = U_DOOR;
                break;
            case URD_DOOR:
                if (floor_properties[START_ROW-1][START_COL-1])
                    floor_properties[START_ROW-1][START_COL-1] = RD_DOOR;
                else
                    floor_properties[START_ROW-1][START_COL-1] = D_DOOR;
                floor_properties[START_ROW+1][START_COL-1] = U_DOOR;
                break;
            case RDL_DOOR:
                floor_properties[START_ROW+1][START_COL-1] = U_DOOR;
                floor_properties[START_ROW][START_COL-2] = R_DOOR;
                break;
            case LUR_DOOR:
                if (floor_properties[START_ROW-1][START_COL-1])
                    floor_properties[START_ROW-1][START_COL-1] = RD_DOOR;
                else
                    floor_properties[START_ROW-1][START_COL-1] = D_DOOR;
                floor_properties[START_ROW][START_COL-2] = R_DOOR;
                break;
            default:
                break;
        }
        
        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                std::cout << floor_properties[i][j] << ",";
            }
            std::cout << std::endl;
        }
        
        // Make the room coordinates for collisions
        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                room_coords[i][j] = glm::vec3((float)(j-START_ROW) * 3.99f,0,(float)(i-START_COL) * 3.99f);
            }
        }
    }
    
	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		/*prog_framebuffer = make_shared<Program>();
		prog_framebuffer->setVerbose(true);
		prog_framebuffer->setShaderNames(resourceDirectory + "/vertFB.glsl", resourceDirectory + "/fragFB.glsl");
		if (!prog_framebuffer->init())
			{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
			}
		prog_framebuffer->init();
        prog_framebuffer->addUniform("DepthMVP");
        //cyl_prog->addUniform("DepthBiasMVP");
        //prog_framebuffer->addUniform("lfactor");
		prog_framebuffer->addAttribute("vertPos");
		//prog_framebuffer->addAttribute("vertTex");*/

        plane_prog = make_shared<Program>();
        plane_prog->setVerbose(true);
        plane_prog->setShaderNames(resourceDirectory + "/plane_shader_vertex.glsl", resourceDirectory + "/plane_shader_fragment.glsl");
        if (!plane_prog->init())
        {
            std::cerr << "One or more plane shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        plane_prog->addUniform("P");
        plane_prog->addUniform("V");
        plane_prog->addUniform("M");
        plane_prog->addAttribute("vertPos");
        plane_prog->addAttribute("vertNor");
        plane_prog->addAttribute("vertTex");
        
        
        // Initialize the GLSL program for the car.
        car_prog = make_shared<Program>();
        car_prog->setVerbose(true);
        car_prog->setShaderNames(resourceDirectory + "/car_shader_vertex.glsl", resourceDirectory + "/car_shader_fragment.glsl");
        if (!car_prog->init())
        {
            std::cerr << "One or more car shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        car_prog->addUniform("P");
        car_prog->addUniform("V");
        car_prog->addUniform("M");
        car_prog->addUniform("DepthMVP");
        car_prog->addUniform("campos");
        car_prog->addUniform("lfactor");
        car_prog->addUniform("lpos");
        car_prog->addAttribute("vertPos");
        car_prog->addAttribute("vertNor");
        car_prog->addAttribute("vertTex");
        
        car_depth_prog = make_shared<Program>();
        car_depth_prog->setVerbose(true);
        car_depth_prog->setShaderNames(resourceDirectory + "/vertFB.glsl", resourceDirectory + "/fragFB.glsl");
        if (!car_depth_prog->init())
        {
            std::cerr << "One or more car shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        car_depth_prog->addUniform("DepthMVP");
        car_depth_prog->addUniform("M");
        car_depth_prog->addAttribute("vertPos");
        
        // Initialize the GLSL program for the car.
        light_prog = std::make_shared<Program>();
        light_prog->setVerbose(true);
        light_prog->setShaderNames(resourceDirectory + "/headlight_shader_vertex.glsl", resourceDirectory + "/headlight_shader_fragment.glsl");
        if (!light_prog->init()) {
            std::cerr << "light prog error" << std::endl;
            exit(1);
        }
        light_prog->addUniform("P");
        light_prog->addUniform("V");
        light_prog->addUniform("M");
        light_prog->addUniform("campos");
        light_prog->addUniform("lfactor");
        light_prog->addAttribute("vertPos");
        light_prog->addAttribute("vertNor");
        
        // Initialize the GLSL program for the room.
        room_prog = std::make_shared<Program>();
        room_prog->setVerbose(true);
        room_prog->setShaderNames(resourceDirectory + "/room_shader_vertex.glsl", resourceDirectory + "/room_shader_fragment.glsl");
        if (!room_prog->init()) {
            std::cerr << "room prog error" << std::endl;
            exit(1);
        }
        room_prog->addUniform("P");
        room_prog->addUniform("V");
        room_prog->addUniform("M");
        room_prog->addUniform("DepthMVP");
        room_prog->addUniform("campos");
        room_prog->addUniform("lfactor");
        room_prog->addUniform("lpos");
        room_prog->addAttribute("vertPos");
        room_prog->addAttribute("vertNor");
        room_prog->addAttribute("vertTex");

        room_depth_prog = std::make_shared<Program>();
        room_depth_prog->setVerbose(true);
        room_depth_prog->setShaderNames(resourceDirectory + "/vertFB.glsl", resourceDirectory + "/fragFB.glsl");
        if (!room_depth_prog->init()) {
            std::cerr << "room prog error" << std::endl;
            exit(1);
        }
        room_depth_prog->addUniform("DepthMVP");
        room_depth_prog->addUniform("M");
        room_depth_prog->addAttribute("vertPos");
        
        // Initialize the GLSL program for cyl.
        cyl_prog = std::make_shared<Program>();
        cyl_prog->setVerbose(true);
        cyl_prog->setShaderNames(resourceDirectory + "/cyl_shader_vertex.glsl", resourceDirectory + "/cyl_shader_fragment.glsl");
        if (!cyl_prog->init()) {
            std::cerr << "cyl prog error" << std::endl;
            exit(1);
        }
        cyl_prog->addUniform("P");
        cyl_prog->addUniform("V");
        cyl_prog->addUniform("M");
        cyl_prog->addUniform("DepthMVP");
        cyl_prog->addUniform("campos");
        cyl_prog->addUniform("lfactor");
        cyl_prog->addUniform("lpos");
        cyl_prog->addAttribute("vertPos");
        cyl_prog->addAttribute("vertNor");
        cyl_prog->addAttribute("vertTex");

        cyl_depth_prog = std::make_shared<Program>();
        cyl_depth_prog->setVerbose(true);
        cyl_depth_prog->setShaderNames(resourceDirectory + "/vertFB.glsl", resourceDirectory + "/fragFB.glsl");
        if (!cyl_depth_prog->init()) {
            std::cerr << "cyl prog error" << std::endl;
            exit(1);
        }
        cyl_depth_prog->addUniform("DepthMVP");
        cyl_depth_prog->addUniform("M");
        cyl_depth_prog->addAttribute("vertPos");
        
        initRooms();
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	//------------------------------------------------------------------------------
	void render_to_framebuffer()
    //void render()
	{
        // render into frame buffer texture
        
        glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObj);


		double frametime = get_last_elapsed_time();

		// Clear framebuffer.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
        //V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		//P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
        
        // CREATE MATRICES FOR SHADOW_MAPPING
        // Compute the MVP matrix from the light's point of view
        //glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
        glm::mat4 depthProjectionMatrix = glm::ortho<float>(-12,12,-5,5,-10,10);
        // previous eye param:
        //glm::vec3 lightInvDir = glm::vec3(0.5f,2,2);
        //glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
        
        
        glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
        //glm::mat4 depthViewMatrix = glm::lookAt(mycam.pos, glm::vec3(0,0,0), glm::vec3(0,1,0));
        // eye: location of camera
        // center: where camera is looking ???
        // up
        
        //glm::mat4 depthModelMatrix = glm::translate(glm::mat4(1), glm::vec3(0, -100, 0)); // Can be anything
        glm::mat4 depthModelMatrix = glm::mat4(1); // Can be anything
        glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
        

   
        
        
        float ninety_degrees = 3.14159/2;
        static float spin_angle = 0;
        spin_angle += 0.01;
        
        
        // Matrix setup for cylinders - include later
        glm::mat4 origin_cylT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -(CYL_HEIGHT/16.0f)));
        glm::mat4 cylOriginR = glm::rotate(glm::mat4(1.0f), ninety_degrees, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 cylS = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 0.2f, 3.0f));
        glm::mat4 CylM = cylS * cylOriginR * origin_cylT;
        
        
        // SHADOW_MAPPING
        /*glm::mat4 biasMatrix(
                             0.5, 0.0, 0.0, 0.0,
                             0.0, 0.5, 0.0, 0.0,
                             0.0, 0.0, 0.5, 0.0,
                             0.5, 0.5, 0.5, 1.0
                             );
        glm::mat4 depthBiasMVP = biasMatrix*depthMVP;*/
        
        
        // Draw cylinders using GLSL.
        cyl_depth_prog->bind();
        glBindVertexArray(CylVertexArrayID);
        
        // FOR SHADOW_MAPPING
        glUniformMatrix4fv(cyl_depth_prog->getUniform("DepthMVP"), 1, GL_FALSE, &depthMVP[0][0]);
        
        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (floor_properties[i][j] != NO_ROOM) {
                    // Draw cyl platform
                    glm::mat4 cylR;
                    if (floor_properties[i][j] == L_DOOR || floor_properties[i][j] == UR_DOOR || floor_properties[i][j] == LUR_DOOR || floor_properties[i][j] == LU_DOOR) {
                        cylR = glm::rotate(glm::mat4(1.0f), spin_angle, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (floor_properties[i][j] == U_DOOR || floor_properties[i][j] == R_DOOR || floor_properties[i][j] == DLU_DOOR || floor_properties[i][j] == DL_DOOR) {
                        cylR = glm::rotate(glm::mat4(1.0f), spin_angle + ninety_degrees, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else {
                        cylR = glm::rotate(glm::mat4(1.0f), spin_angle + (2*ninety_degrees), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    glm::mat4 cylT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                    
                    glm::mat4 CylAM = cylT * cylR * CylM;
                    M = CylAM;
                    glUniformMatrix4fv(cyl_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    
                    //actually draw from vertex 0, 6*80*CYL_HEIGHT  + 2*3*80 vertices
                    glDrawArrays(GL_TRIANGLES, 0, CYL_BUFFER_SIZE);
                }
            }
        }
        cyl_depth_prog->unbind();
        
        glm::mat4 roomOrigT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -7.0f));
        glm::mat4 roomS = glm::scale(glm::mat4(1.0f), glm::vec3(3.6f, 3.6f, 20.0f));
        glm::mat4 roomR90 = glm::rotate(glm::mat4(1.0f), ninety_degrees, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 roomR180 = glm::rotate(glm::mat4(1.0f), ninety_degrees*2, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 roomR270 = glm::rotate(glm::mat4(1.0f), ninety_degrees*3, glm::vec3(0.0f, 0.0f, 1.0f));
        
        // Draw rooms using GLSL.
        room_depth_prog->bind();
        CHECKED_GL_CALL(glBindVertexArray(RoomVertexArrayID));
        
        // FOR SHADOW_MAPPING
        CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("DepthMVP"), 1, GL_FALSE, &depthMVP[0][0]));
        
        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (floor_properties[i][j] != NO_ROOM) {
                    glm::mat4 roomT, RoomM;
                    switch (floor_properties[i][j]) {
                            // TODO IMPLEMENT ROATATIONS APPRIROPRATELY
                        case L_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            // OLD - what I thought would work
                            RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            //RoomM = roomT * roomR90 * CylM * roomOrigT * roomS;  <- example
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case U_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR90 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case R_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR180 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case D_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR270 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case LU_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR270 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            //RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case UR_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR180 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case RD_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR90 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            //RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case DL_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            // OLD - what I thought would work
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case LUR_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR180 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            //RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE));
                            break;
                        case URD_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR90 * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            
                            //RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE));
                            break;
                        case RDL_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            // OLD - what I thought would work
                            //RoomM = roomT * CylM * roomOrigT * roomS;
                            // TODO FOR ALL
                            // NEW - seems to should to work
                            RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE));
                            break;
                        case DLU_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomS;
                            //RoomM = roomT * roomR270 * CylM * roomOrigT * roomS;
                            //RoomM = roomR270 * roomT * CylM * roomOrigT * roomS;
                            // OLD - what I thought would work
                            RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            //RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            
                            M = RoomM;
                            CHECKED_GL_CALL(glUniformMatrix4fv(room_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                            
                            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE));
                            //glDrawArrays(GL_TRIANGLES, 0, 6*80*(CYL_HEIGHT/2) + 6*50*(CYL_HEIGHT/2) + 2*3*80);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        room_depth_prog->unbind();
        
        CHECKED_GL_CALL(glBindVertexArray(0));
     
        
        //glm::mat4 carS = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
        glm::mat4 carS = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f, 0.7f));
        //glm::mat4 origin_carT = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, -0.5f));
        glm::mat4 CarM = carS;
        
        car_depth_prog->bind();
        
        // FOR SHADOW_MAPPING
        //glUniformMatrix4fv(car_depth_prog->getUniform("DepthMVP"), 1, GL_FALSE, &depthMVP[0][0]);
        CHECKED_GL_CALL(glUniformMatrix4fv(car_depth_prog->getUniform("DepthMVP"), 1, GL_FALSE, &depthMVP[0][0]));
        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (floor_properties[i][j]) {
                    // Draw car
                    glm::mat4 carR;
                    if (floor_properties[i][j] == L_DOOR || floor_properties[i][j] == UR_DOOR || floor_properties[i][j] == LUR_DOOR || floor_properties[i][j] == LU_DOOR) {
                        carR = glm::rotate(glm::mat4(1.0f), spin_angle, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (floor_properties[i][j] == U_DOOR || floor_properties[i][j] == R_DOOR || floor_properties[i][j] == DLU_DOOR || floor_properties[i][j] == DL_DOOR) {
                        carR = glm::rotate(glm::mat4(1.0f), spin_angle + ninety_degrees, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else {
                        carR = glm::rotate(glm::mat4(1.0f), spin_angle + (2*ninety_degrees), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                
                
                    glm::mat4 carT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW)*3.99f, -0.29f, (float)(i-START_COL)*3.99f));

                    glm::mat4 CarAM = carT * carR *CarM;
                
                    M = CarAM;
                    //glUniformMatrix4fv(car_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    CHECKED_GL_CALL(glUniformMatrix4fv(car_depth_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]));
                    // For using materials in shape
                    car_shape->draw(car_depth_prog, false);
                    //car_shape->draw(car_prog, true);
                }
            }
        }
        car_depth_prog->unbind();
        
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glBindTexture(GL_TEXTURE_2D, FBOtex);
		//glGenerateMipmap(GL_TEXTURE_2D);

	}
    
    /*void render_shadow_map()
    {
        double frametime = get_last_elapsed_time();

        // Get current frame buffer size.
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        float aspect = width/(float)height;
        glViewport(0, 0, width, height);
        
        // Clear framebuffer.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 V, M, P; //View, Model and Perspective matrix
        V = mycam.process(frametime);
        M = glm::mat4(1);
        // Apply orthographic projection....
        P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
        
        glm::mat4 transPlane = glm::translate(glm::mat4(1), glm::vec3(0.0f, -0.5f, 0.0f));
        
        plane_prog->bind();
        
        M = transPlane;
        
        glUniformMatrix4fv(plane_prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(plane_prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(plane_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_2D, FBOtex);
        plane_shape->draw(plane_prog, false);
        plane_prog->unbind();
    }*/
    
    void render()
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_2D, FBOtex);
        
        double frametime = get_last_elapsed_time();
        
        // Get current frame buffer size.
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        float aspect = width/(float)height;
        glViewport(0, 0, width, height);
        
        // Clear framebuffer.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        // Create the matrix stacks - please leave these alone for now
        
        glm::mat4 V, M, P; //View, Model and Perspective matrix
        V = mycam.process(frametime);
        M = glm::mat4(1);
        // Apply orthographic projection....
        P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
        
        // Perspective matrix of light source
        glm::mat4 depthProjectionMatrix = glm::ortho<float>(-12,12,-5,5,-10,10);
        
        glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
      
        glm::mat4 depthModelMatrix = glm::mat4(1); // Can be anything
        glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
        
        // Quinn's addition
        // Can be inside 2 rooms @ once max
        int present_rows[] = {-1, -1};
        int present_cols[] = {-1, -1};
        int pres_index = 0;
        //int present_row, present_col;
        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (abs(room_coords[i][j].x - mycam.pos.x) < room_rad &&
                    abs(room_coords[i][j].z - mycam.pos.z) < room_rad) {
                    
                    present_rows[pres_index] = i;
                    present_cols[pres_index] = j;
                    pres_index++;
                }
            }
        }
        
        
        collision = false;
        
        // DETECT COLLISION
        float dist_from_room_center = length(mycam.pos - room_coords[present_rows[0]][present_cols[0]]);
        // Wall collision
        if ((dist_from_room_center + camera_rad) >= room_rad && present_rows[1] == -1)
            collision = true;
        // Platform collision
        if (dist_from_room_center <= (camera_rad + platform_rad))
            collision = true;
        
        std::cout << "COLLISION: " << collision << std::endl;
        std::cout << "last collision: " << lastFrameCollision << std::endl;
        std::cout << "disabled key: " << disabled_dir_key << std::endl;
        std::cout << "last pressed key: " << latest_dir_key << std::endl;
        std::cout << "others pressed: " << othersPressed << std::endl;
        
        // If we have a disabled key, if any other key pressed chance at no collision
        
        // For instantaneous collision reaction
        if (collision) {
            if (lastFrameCollision && othersPressed) {
                disabled_dir_key = NO_KEY;
                othersPressed = false;
            } else if (lastFrameCollision == false || othersPressed == false) {
                if (latest_dir_key == W_KEY) {
                    mycam.w = 0;
                    disabled_dir_key = W_KEY;
                }
                else if (latest_dir_key == S_KEY) {
                    mycam.s = 0;
                    disabled_dir_key = S_KEY;
                }
            }
        } else {
            disabled_dir_key = NO_KEY;
            othersPressed = false;
        }
        lastFrameCollision = collision;
        
        float light_factor;
        if (light_mode == DAY)
            light_factor = 1.0;
        else
            light_factor = 0.5;
        
        float ninety_degrees = 3.14159/2;
        static float spin_angle = 0;
        spin_angle += 0.01;
        
        
        // Matrix setup for cylinders - include later
        glm::mat4 origin_cylT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -(CYL_HEIGHT/16.0f)));
        glm::mat4 cylOriginR = glm::rotate(glm::mat4(1.0f), ninety_degrees, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 cylS = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 0.2f, 3.0f));
        glm::mat4 CylM = cylS * cylOriginR * origin_cylT;
        
        
        // Draw cylinders using GLSL.
        cyl_prog->bind();
        glBindVertexArray(CylVertexArrayID);
        
        glUniformMatrix4fv(cyl_prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(cyl_prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        // FOR SHADOW_MAPPING
        glUniformMatrix4fv(cyl_prog->getUniform("DepthMVP"), 1, GL_FALSE, &depthMVP[0][0]);
        
        glUniform3fv(cyl_prog->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform1f(cyl_prog->getUniform("lfactor"), light_factor);
        glUniform3fv(cyl_prog->getUniform("lpos"), 1, &lightPos[0]);

        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (floor_properties[i][j] != NO_ROOM) {
                    // Draw cyl platform
                    glm::mat4 cylR;
                    if (floor_properties[i][j] == L_DOOR || floor_properties[i][j] == UR_DOOR || floor_properties[i][j] == LUR_DOOR || floor_properties[i][j] == LU_DOOR) {
                        cylR = glm::rotate(glm::mat4(1.0f), spin_angle, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (floor_properties[i][j] == U_DOOR || floor_properties[i][j] == R_DOOR || floor_properties[i][j] == DLU_DOOR || floor_properties[i][j] == DL_DOOR) {
                        cylR = glm::rotate(glm::mat4(1.0f), spin_angle + ninety_degrees, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else {
                        cylR = glm::rotate(glm::mat4(1.0f), spin_angle + (2*ninety_degrees), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    
                    glm::mat4 cylT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                    
                    glm::mat4 CylAM = cylT * cylR * CylM;
                    M = CylAM;
                    glUniformMatrix4fv(cyl_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    
                    //actually draw from vertex 0, 6*80*CYL_HEIGHT  + 2*3*80 vertices
                    glDrawArrays(GL_TRIANGLES, 0, CYL_BUFFER_SIZE);
                }
            }
        }
        cyl_prog->unbind();
        
        glm::mat4 roomOrigT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -7.0f));
        glm::mat4 roomS = glm::scale(glm::mat4(1.0f), glm::vec3(3.6f, 3.6f, 20.0f));
        glm::mat4 roomR90 = glm::rotate(glm::mat4(1.0f), ninety_degrees, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 roomR180 = glm::rotate(glm::mat4(1.0f), ninety_degrees*2, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 roomR270 = glm::rotate(glm::mat4(1.0f), ninety_degrees*3, glm::vec3(0.0f, 0.0f, 1.0f));
        
        // Draw rooms using GLSL.
        room_prog->bind();
        glBindVertexArray(RoomVertexArrayID);
        
        glUniformMatrix4fv(room_prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(room_prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        // FOR SHADOW_MAPPING
        glUniformMatrix4fv(room_prog->getUniform("DepthMVP"), 1, GL_FALSE, &depthMVP[0][0]);
        glUniform3fv(room_prog->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform1f(room_prog->getUniform("lfactor"), light_factor);
        glUniform3fv(room_prog->getUniform("lpos"), 1, &lightPos[0]);

        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (floor_properties[i][j] != NO_ROOM) {
                    glm::mat4 roomT, RoomM;
                    switch (floor_properties[i][j]) {
                        case L_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE);
                            break;
                        case U_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE);
                            break;
                        case R_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE);
                            break;
                        case D_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, ONE_DRAW_OFFSET, ONE_DOOR_SIZE);
                            break;
                        case LU_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE);
                            break;
                        case UR_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE);
                            break;
                        case RD_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                 
                            RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE);
                            break;
                        case DL_DOOR:
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                   
                            RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TWO_DRAW_OFFSET, TWO_DOOR_SIZE);
                            break;
                        case LUR_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case URD_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomR90 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case RDL_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomR180 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        case DLU_DOOR:
                            // Draw cyl room
                            roomT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                            
                            RoomM = roomT * CylM * roomOrigT * roomS;
            
                            RoomM = roomT * CylM * roomOrigT * roomR270 * roomS;
                            
                            M = RoomM;
                            glUniformMatrix4fv(room_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                            
                            glDrawArrays(GL_TRIANGLES, TRI_DRAW_OFFSET, TRI_DOOR_SIZE);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        room_prog->unbind();
        
        glBindVertexArray(0);
        
        
        glm::mat4 carS = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f, 0.7f));
        glm::mat4 CarM = carS;
        
        car_prog->bind();
        
        glUniformMatrix4fv(car_prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(car_prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        // FOR SHADOW_MAPPING
        glUniformMatrix4fv(car_prog->getUniform("DepthMVP"), 1, GL_FALSE, &depthMVP[0][0]);
        glUniform3fv(car_prog->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform1f(car_prog->getUniform("lfactor"), light_factor);
        glUniform3fv(car_prog->getUniform("lpos"), 1, &lightPos[0]);

        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (floor_properties[i][j]) {
                    // Draw car
                    glm::mat4 carR;
                    if (floor_properties[i][j] == L_DOOR || floor_properties[i][j] == UR_DOOR || floor_properties[i][j] == LUR_DOOR || floor_properties[i][j] == LU_DOOR) {
                        carR = glm::rotate(glm::mat4(1.0f), spin_angle, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (floor_properties[i][j] == U_DOOR || floor_properties[i][j] == R_DOOR || floor_properties[i][j] == DLU_DOOR || floor_properties[i][j] == DL_DOOR) {
                        carR = glm::rotate(glm::mat4(1.0f), spin_angle + ninety_degrees, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else {
                        carR = glm::rotate(glm::mat4(1.0f), spin_angle + (2*ninety_degrees), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    
                    glm::mat4 carT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW)*3.99f, -0.29f, (float)(i-START_COL)*3.99f));
                    
                    glm::mat4 CarAM = carT * carR *CarM;
                    
                    M = CarAM;
                    glUniformMatrix4fv(car_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    
                    // For using materials in shape
                    car_shape->draw(car_prog, false);
                    //car_shape->draw(car_prog, true);
                }
            }
        }
        car_prog->unbind();
        
        float light_angle = 3.14149/1.5;
        
        glm::mat4 lightS = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.02f));
        // Angled light placement
        glm::mat4 firstLightT = glm::translate(glm::mat4(1.0f), glm::vec3(0.275f, 0.29f, 0.825f));
        glm::mat4 secondLightT = glm::translate(glm::mat4(1.0f), glm::vec3(-0.275f, 0.29f, 0.825f));
        glm::mat4 lightR = glm::rotate(glm::mat4(1.0f), light_angle, glm::vec3(1.0f, 0.0f, 0.0f));
        
        // Draw headlights using GLSL.
        light_prog->bind();
        
        glUniformMatrix4fv(light_prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(light_prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniform3fv(light_prog->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform1f(light_prog->getUniform("lfactor"), light_factor);
        
        for (int i = 0; i < ROOM_ROWS; i++) {
            for (int j = 0; j < ROOM_COLS; j++) {
                if (floor_properties[i][j] != NO_ROOM) {
                    // Draw headlights
                    glm::mat4 light_R;
                    if (floor_properties[i][j] == L_DOOR || floor_properties[i][j] == UR_DOOR || floor_properties[i][j] == LUR_DOOR || floor_properties[i][j] == LU_DOOR) {
                        light_R = glm::rotate(glm::mat4(1.0f), spin_angle, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (floor_properties[i][j] == U_DOOR || floor_properties[i][j] == R_DOOR || floor_properties[i][j] == DLU_DOOR || floor_properties[i][j] == DL_DOOR) {
                        light_R = glm::rotate(glm::mat4(1.0f), spin_angle + ninety_degrees, glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else {
                        light_R = glm::rotate(glm::mat4(1.0f), spin_angle + (2*ninety_degrees), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    
                    glm::mat4 lightT = glm::translate(glm::mat4(1.0f), glm::vec3((float)(j-START_ROW) * 3.99f, -0.5f, (float)(i-START_COL) * 3.99f));
                    
                    
                    // M = T * R * S = standard order
                    // First headlight
                    glm::mat4 LightAM = lightT * light_R * CarM * firstLightT * lightR * lightS;
                    
                    M = LightAM;
                    glUniformMatrix4fv(light_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    sphere_shape->draw(light_prog, false);
                    
                    // Second headlight
                    LightAM = lightT * light_R *CarM * secondLightT * lightR * lightS;
                    
                    M = LightAM;
                    glUniformMatrix4fv(light_prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
                    sphere_shape->draw(light_prog, false);
                }
            }
        }
        light_prog->unbind();
        
    }

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
    srand(time(0));
    
	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
        application->render_to_framebuffer();
        application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}



/* HELPER GEOMETRY FUNCTIONS */
void initRoomSides(vec3 room_vertex_buffer_data[], int index, int height, int rooms) {
    // Sides
    float row_height = 0;
    float alpha = 0;
    float d_alpha = (2.0 * 3.1415926) / 80.0;
    float rad = 0.2;
    
    // Iterate for every triangle
    for (int i = 0; i < height; i++, row_height += 0.1) {
        
        for (int j = 0; j < 80; j++, alpha += d_alpha) {
            
            //skip iteration if door conditions
            if ((rooms == 3) && ((15 <= j && j < 25) || (35 <= j && j < 45) ||
                                 (55 <= j && j < 65)) && (height/2) <= i) {
                continue;
            }
            else if ((rooms == 2) && ((15 <= j && j < 25) || (35 <= j && j < 45)) && (height/2) <= i) {
                continue;
            }
            else if ((rooms == 1) && (15 <= j && j < 25) && (height/2) <= i) {
                continue;
            }
            for (int k = 0; k < 6; k++) {
                
                glm::vec3 vector;
                switch(k) {
                    case 0:
                        vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height);
                        break;
                    case 1:
                        vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                        break;
                    case 2:
                        vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                        break;
                    case 3:
                        vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height + 0.1);
                        break;
                    case 4:
                        vector = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, row_height);
                        break;
                    case 5:
                        vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, row_height + 0.1);
                        break;
                }
                room_vertex_buffer_data[index++] = vector;
            }
        }
    }
}

void initCylFace(vec3 room_vertex_buffer_data[], int index, float height) {
    // Back face
    vec3 initial_vec3 = glm::vec3(50,50,50);
    vec3 prev_vec3 = initial_vec3;
    float alpha = 0;
    float d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
    float rad = 0.2;
    int z_flag = 1;
    
    // Iterate for every triangle
    for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
        glm::vec3 vector;
        if (i % 3 == 0) {
            room_vertex_buffer_data[index++] = glm::vec3(0,0,height*0.1f);
        }
        else if (i == 1 || i == ((3*80) - 1)) { // At second vertex
            vector = glm::vec3(0,rad,height*0.1f);
            room_vertex_buffer_data[index++] = vector;
        }
        else if (prev_vec3 == initial_vec3) {
            if (z_flag) {
                vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, height*0.1f);
                z_flag = 0;
            }
            else {
                vector = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, height*0.1f);
                z_flag = 1;
            }
            room_vertex_buffer_data[index++] = vector;
            prev_vec3 = vector;
        }
        else {  // Use previous value
            room_vertex_buffer_data[index++] = prev_vec3;
            prev_vec3 = initial_vec3;
        }
    }
}

void initFaceNormals(vec3 normal_buffer_data[], int index, int face) {
    float alpha = 0;
    float d_alpha = (2.0 * 3.1415926) / 80.0 / 3.0;
    
    for (int i = 0; i < (3*80); i++, alpha += d_alpha) {
        normal_buffer_data[i] = glm::vec3(0,0,face);
        index++;
    }
}

void initSideNormals(vec3 room_normal_buffer_data[], int index, int height, int doors) {
    // Sides
    float alpha = 0;
    float d_alpha = (2.0 * 3.1415926) / 80.0;
    float rad = 0.2;
    // Iterate for every triangle
    for (int i = 0; i < height; i++) {
        
        for (int j = 0; j < 80; j++, alpha += d_alpha) {
            
            //skip iteration if door conditions
            if ((doors == 3) && ((15 <= j && j < 25) || (35 <= j && j < 45) ||
                                 (55 <= j && j < 65)) && (height/2) <= i) {
                continue;
            }
            else if ((doors == 2) && ((15 <= j && j < 25) || (35 <= j && j < 45)) &&
                     (height/2) <= i) {
                continue;
            }
            else if ((doors == 1) && (15 <= j && j < 25) && (height/2) <= i) {
                continue;
            }
            
            glm::vec3 norm_vector;
            
            glm::vec3 xy_pos = glm::vec3(-sin(alpha)*rad, cos(alpha)*rad, 0);
            glm::vec3 xy_pos_next = glm::vec3(-sin(alpha + d_alpha)*rad, cos(alpha + d_alpha)*rad, 0);
            
            
            glm::vec3 tan = xy_pos - xy_pos_next;
            //glm::vec3 tan = xy_pos + xy_pos_next;
            glm::vec3 parallel = glm::vec3(0,0,-1);
            
            norm_vector = glm::vec3((tan.y*parallel.z)-(tan.z*parallel.y), (tan.z*parallel.x)-(tan.x*parallel.z), (tan.x*parallel.y)-(tan.y*parallel.x));
            
            norm_vector = normalize(norm_vector);
            
            std::cout << "norm_vector: " << norm_vector.x  << "," << norm_vector.y << "," << norm_vector.z << std::endl;
            std::cout << "Cyl Height: " << i << std::endl;
            
            for (int k = 0; k < 6; k++) {
                room_normal_buffer_data[index++] = norm_vector;
            }
        }
    }
}
