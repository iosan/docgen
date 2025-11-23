#include "text_viewer.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

std::string TextViewer::loadFile(const std::string& filepath) {
    if (!fileExists(filepath)) {
        throw std::runtime_error("File does not exist: " + filepath);
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool TextViewer::fileExists(const std::string& filepath) {
    struct stat buffer;
    return (stat(filepath.c_str(), &buffer) == 0);
}
