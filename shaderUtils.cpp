#include "shaderUtils.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Function implementation
std::string loadShaderSource(const char* filePath) {
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        exit(EXIT_FAILURE);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
