#include "shader.h"


std::string readFile(const std::string& filepath) {
    // Open the file
    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    // Get the size of the file
    std::streamsize fileSize = file.tellg();
    if (fileSize < 0) {
        throw std::runtime_error("Failed to get the file size!");
    }

    // Go back to the beginning of the file
    file.seekg(0, std::ios::beg);

    // Create a string with the size of the file
    std::string buffer(fileSize, '\0');

    // Read the content of the file into the string
    if (!file.read(&buffer[0], fileSize)) {
        throw std::runtime_error("Failed to read the file content!");
    }

    file.close();
    return buffer;
}

GLuint Shader::createShader(const std::string& shader_filepath, GLenum shader_type) {
    int success;
    char infoLog[512];
    std::string shader_source = readFile(shader_filepath);
    const GLchar* source = shader_source.c_str();
    unsigned int shader;

    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        // assuming fragment if not vertex is very big assumption; done because laziness
        std::cout << "ERROR::SHADER::" << (shader_type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                  << "::COMPILATION_FAILED\n" << infoLog << '\n';
    }

    return shader;
}

// creates shader program and links with vertex and fragment shaders
// NOTE: also calls glDeleteShader on shaders
GLuint Shader::createShaderProgram(GLuint vertex_shader, GLuint fragment_shader) {
    int success;
    char infoLog[512];
    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << '\n';
    }

    // delete shader objects once linked with program object
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

Shader::Shader(const char* v_path, const char* f_path) {
    GLuint vertex_shader = createShader(v_path, GL_VERTEX_SHADER);
    GLuint fragment_shader = createShader(f_path, GL_FRAGMENT_SHADER);
    this->id = createShaderProgram(vertex_shader, fragment_shader);
    in_use = false;
}

void Shader::use() {
    glUseProgram(this->id);
    in_use = true;
}

void Shader::addUniform(std::string name) {
    if (in_use) {
        glGetUniformLocation(this->id, name.c_str());
    }
    else {
        throw std::runtime_error("Tried to addUniform before the shader was in use.");
    }
}

void Shader::addUniforms(std::vector<std::string> names) {
    if (in_use) {
        for (int i = 0; i < names.size(); i++) {
            glGetUniformLocation(this->id, names[i].c_str());
        }
    }
    else {
        throw std::runtime_error("Tried to addUniform before the shader was in use.");
    }
}

void Shader::remove() {
    glDeleteProgram(this->id);
    in_use = false;
}