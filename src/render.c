#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "cglm/cglm.h"

GLFWwindow *window;

GLuint load_shader(const char *filePath, GLenum shaderType) {
    // Load shader source code
    FILE *file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open shader file: %s\n", filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the source code plus the null terminator
    char *source = (char *)malloc(length + 1);
    if (!source) {
        fprintf(stderr, "Failed to allocate memory for shader source\n");
        fclose(file);
        return 0;
    }

    // Read the shader source code
    size_t bytesRead = fread(source, 1, length, file);
    fclose(file);

    // Ensure the string is null-terminated
    source[bytesRead] = '\0';

    // Create and compile shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation error: %s\n", infoLog);
    }

    free(source);
    return shader;
}

GLuint create_shaderprogram() {
    GLuint vertexShader = load_shader("shader/vertex_shader.glsl", GL_VERTEX_SHADER);
    GLuint fragmentShader = load_shader("shader/fragment_shader.glsl", GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    //Delete shaders as they're no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint shader_open_atlas()
{
    // Load image from file
    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/atlas.png", &width, &height, &nrChannels, 0);
    
    if (!data) {
        printf("Faszom\n");
        return -1;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Determine format based on channels
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    // Upload image data to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D); // Generate mipmaps if needed

    // Free the image memory after uploading
    stbi_image_free(data);
    return texture;
}

int initialize_window()
{
    if(!glfwInit())
    {
        printf("Failded to initialize glfw!\n");
        return 0;
    }

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My window", NULL, NULL);

    if(!window)
    {
        return 0;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if(glewInit() != GLEW_OK)
    {
        printf("Failded to initialize glew!\n");
        return 0;
    }

    return 1;
}

void configure_render()
{
    glClearColor(0.25f, 0.55f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glfwSwapInterval(1);
}

GLuint viewLoc;
GLuint projLoc;
GLuint selectionPos;
GLuint funPos;

void initialize_shaders()
{
    GLuint shaderporgram = create_shaderprogram();
    glUseProgram(shaderporgram);

    viewLoc = glGetUniformLocation(shaderporgram, "view");
    projLoc = glGetUniformLocation(shaderporgram, "projection");
    selectionPos = glGetUniformLocation(shaderporgram, "selection");
    GLuint atlasLoc = glGetUniformLocation(shaderporgram, "atlasTexture");
    funPos  = glGetUniformLocation(shaderporgram, "fun");

    mat4 projection;
    glm_perspective(glm_rad(90.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f, projection);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat *)projection);

    GLuint atlas = shader_open_atlas();
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, atlas);
    glUniform1i(atlasLoc, 0);
}

void render_update_selection(int x, int y, int z)
{
    glUniform3i(selectionPos, x, y, z);
    glUniform1f(funPos, glfwGetTime());
}

void update_view(vec3 position, vec3 forward)
{
    vec3 target;
    glm_vec3_add(position, forward, target);

    mat4 view;
    glm_lookat(position, target, (vec3){0, 1, 0}, view);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat *)view);
}


GLuint vao;
GLuint vbo;

void init_cube()
{
    GLfloat vertecies[] =
    {
        0, 0, 1,  1, 1, 1,  0, 1, 1,  0, 0, 1,  1, 0, 1,  1, 1, 1, //front face
        1, 0, 0,  0, 1, 0,  1, 1, 0,  1, 0, 0,  0, 0, 0,  0, 1, 0, //back face
        1, 0, 1,  1, 1, 0,  1, 1, 1,  1, 0, 1,  1, 0, 0,  1, 1, 0, //right face
        0, 0, 0,  0, 1, 1,  0, 1, 0,  0, 0, 0,  0, 0, 1,  0, 1, 1, //left face
        0, 1, 1,  1, 1, 0,  0, 1, 0,  0, 1, 1,  1, 1, 1,  1, 1, 0, //up face
        1, 0, 0,  1, 0, 1,  0, 0, 1,  1, 0, 0,  0, 0, 1,  0, 0, 0
    };

    GLfloat vertExtra[144];
    int vertIndex = 0;

    for (size_t i = 0; i < 108; i+=3)
    {
        vertExtra[vertIndex++] = vertecies[i];
        vertExtra[vertIndex++] = vertecies[i + 1];
        vertExtra[vertIndex++] = vertecies[i + 2];
        vertExtra[vertIndex++] = (GLfloat)i / 108;
        printf("%f\n", vertExtra[vertIndex - 1]);
    }
    

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertExtra), vertExtra, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render_cube()
{
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void render_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
    glBindVertexArray(0);
}