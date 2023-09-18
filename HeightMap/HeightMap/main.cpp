#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader_m.h"
#include "camera.h"
#include "basicCubeMesh.h"

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void DrawCube(GLfloat centerPosX, GLfloat centerPosY, GLfloat centerPosZ, GLfloat edgelength);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int useWireframe = 0;
int displayGrayscale = 0;

// camera - give pretty starting point
Camera camera(glm::vec3(67.0f, 627.5f, 169.9f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    -128.1f, -42.4f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: Terrain CPU", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;

    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    //// Create a camera
     //Camera camera;

     //// Set the initial position and look at the center of the chessboard
     //camera.setPosition(0); // Start at the first position
     //camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f)); // Look at the center

     //// Set the key callback
     //glfwSetKeyCallback(window, keyCallback);
    // build and compile our shader program
    // ------------------------------------
    Shader heightMapShader("resources/shaders/height.shader.vert","resources/shaders/height.shader.frag" );
    Shader cubeShader("resources/shaders/basic.shader.vert", "resources/shaders/basic.shader.frag");
    // load and create a texture
    // -------------------------
    // load image, create texture and generate mipmaps
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("resources/heightmaps/myHeightmap.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << "Loaded heightmap of size " << 50 << " x " << 50 << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<float> vertices;
    float yScale = 64.0f / 256.0f, yShift = 16.0f;
    int rez = 1;
    unsigned bytePerPixel = nrChannels;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
            unsigned char y = pixelOffset[0];

            // vertex
            vertices.push_back(-height / 2.0f + height * i / (float)height);   // vx
            vertices.push_back((int)y * yScale - yShift);   // vy
            vertices.push_back(-width / 2.0f + width * j / (float)width);   // vz
        }
    }
    std::cout << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
    stbi_image_free(data);

    std::vector<unsigned> indices;
    for (unsigned i = 0; i < height - 1; i += rez)
    {
        for (unsigned j = 0; j < width; j += rez)
        {
            for (unsigned k = 0; k < 2; k++)
            {
                indices.push_back(j + width * (i + k * rez));
            }
        }
    }
    std::cout << "Loaded " << indices.size() << " indices" << std::endl;

    const int numStrips = (height - 1) / rez;
    const int numTrisPerStrip = (width / rez) * 2 - 2;
    std::cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << std::endl;
    std::cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << std::endl;

    // first, configure the cube's VAO (and terrainVBO + terrainIBO)
    unsigned int terrainVAO, terrainVBO, terrainIBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrainIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

    // render loop
    // -----------

    std::vector<basicCubeVertex> cubevertices = {
       {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},
       {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f)},

       {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
       {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},

       {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
       {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},

       {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
       {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},

       {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
       {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},

       {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
       {glm::vec3(0.5f,  0.5f, -0.5f), glm::vec2(1.0f, 1.0f)},
       {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1.0f, 0.0f)},
       {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0.0f, 0.0f)},
       {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec2(0.0f, 1.0f)},
    };
    basicCubeMesh myCube(cubevertices);
    
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //        std::cout << deltaTime << "ms (" << 1.0f / deltaTime << " FPS)" << std::endl;

                // input
                // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        heightMapShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        heightMapShader.setMat4("projection", projection);
        heightMapShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        heightMapShader.setMat4("model", model);

        

        // render the cube
        glBindVertexArray(terrainVAO);
        //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for (unsigned strip = 0; strip < numStrips; strip++)
        {
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                numTrisPerStrip + 2,   // number of indices to render
                GL_UNSIGNED_INT,     // index data type
                (void*)(sizeof(unsigned) * (numTrisPerStrip + 2) * strip)); // offset to starting index
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        unsigned int random;
        bool white = false;
      
        int xadjust = 0;
            int yadjust = 0;
        for (size_t i = 0; i < 10; i++)
        {

            for (size_t j = 0; j < 10; j++)
            {
                if (j < 8)
                {
                    random = rand() % 10;
                    if (white == true)
                    {
                        glm::mat4 cubemodel = glm::mat4(1.0f);
                            //model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                            cubemodel = glm::rotate(cubemodel, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
                        
                            glm::mat4 cubeview = glm::mat4(1.0f);
                            // note that we're translating the scene in the reverse direction of where we want to move
                            cubeview = glm::translate(cubeview, glm::vec3(0.0f, 0.0f, -3.0f));
                        
                            glm::mat4 cubeprojection = glm::mat4(1.0f);
                            cubeprojection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
                        
                            unsigned int viewLoc = glGetUniformLocation(cubeShader.ID, "view");
                        
                            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &cubeview[0][0]);
                        
                            cubeShader.setMat4("projection", cubeprojection);
                            cubeShader.setMat4("model", cubemodel);
                        glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
                           glm::mat4 trans = glm::mat4(1.0f);

                            trans = glm::translate(trans, glm::vec3(xadjust,yadjust,random));
                            vec = trans * vec;
                        
                            std::cout << vec.x << vec.y << vec.z << std::endl;
                        
                            unsigned int transformLoc = glGetUniformLocation(cubeShader.ID, "transform");
                        
                           glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
                        
                           cubeShader.setMat4("transform", trans);
                                         
                        myCube.Draw(cubeShader,0);
                        white = false;
                    }
                    else
                    {
                        glm::mat4 cubemodel = glm::mat4(1.0f);
                        //model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                        cubemodel = glm::rotate(cubemodel, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

                        glm::mat4 cubeview = glm::mat4(1.0f);
                        // note that we're translating the scene in the reverse direction of where we want to move
                        cubeview = glm::translate(cubeview, glm::vec3(0.0f, 0.0f, -3.0f));

                        glm::mat4 cubeprojection = glm::mat4(1.0f);
                        cubeprojection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

                        unsigned int viewLoc = glGetUniformLocation(cubeShader.ID, "view");

                        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &cubeview[0][0]);

                        cubeShader.setMat4("projection", cubeprojection);
                        cubeShader.setMat4("model", cubemodel);
                        glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
                        glm::mat4 trans = glm::mat4(1.0f);

                        trans = glm::translate(trans, glm::vec3(xadjust, yadjust, random));
                        vec = trans * vec;

                        std::cout << vec.x << vec.y << vec.z << std::endl;

                        unsigned int transformLoc = glGetUniformLocation(cubeShader.ID, "transform");

                        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

                        cubeShader.setMat4("transform", trans);

                        myCube.Draw(cubeShader, 0);
                        white = false;
                    }
                    
                }
                else
                {
                    myCube.Draw(cubeShader, 3);
                }
                yadjust++;
            }
            xadjust++;
        }
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);
    glDeleteBuffers(1, &terrainIBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever a key event occurs, this callback is called
// ---------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_SPACE:
            useWireframe = 1 - useWireframe;
            break;
        case GLFW_KEY_G:
            displayGrayscale = 1 - displayGrayscale;
            break;
        default:
            break;
        }
    }
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

//void DrawCube(Shader cubeShader) 
//{
//    //CUBE
//    glm::mat4 cubemodel = glm::mat4(1.0f);
//    //model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//    cubemodel = glm::rotate(cubemodel, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
//
//    glm::mat4 cubeview = glm::mat4(1.0f);
//    // note that we're translating the scene in the reverse direction of where we want to move
//    cubeview = glm::translate(cubeview, glm::vec3(0.0f, 0.0f, -3.0f));
//
//    glm::mat4 cubeprojection = glm::mat4(1.0f);
//    cubeprojection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
//
//    unsigned int viewLoc = glGetUniformLocation(cubeShader.ID, "view");
//
//    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &cubeview[0][0]);
//
//    cubeShader.setMat4("projection", cubeprojection);
//    cubeShader.setMat4("model", cubemodel);
//    
//}
//
////void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)  ////Responsible for the movement between cameras
////  {
////    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
////        if (key == GLFW_KEY_RIGHT) {
////            camera.cyclePosition(1); // Move to the next position
////        }
////        else if (key == GLFW_KEY_LEFT) {
////            camera.cyclePosition(-1); // Move to the previous position
////        }
////    }
////}
//
//void MoveCube(Shader cubeShader,int x, int y, int z)
//{
//    glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
//    glm::mat4 trans = glm::mat4(1.0f);
//
//
//    trans = glm::translate(trans, glm::vec3(0.0f+x, 0.0f+y, 0.0f+z));
//    vec = trans * vec;
//
//    std::cout << vec.x << vec.y << vec.z << std::endl;
//
//    unsigned int transformLoc = glGetUniformLocation(cubeShader.ID, "transform");
//
//    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
//
//    cubeShader.setMat4("transform", trans);
//}