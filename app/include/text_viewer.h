// =====================
// TextViewer.h
// =====================
// Utility class for loading and checking text files.
// =====================

#ifndef TEXT_VIEWER_H
#define TEXT_VIEWER_H

#include <string>

class TextViewer {
public:
    // ----- File Operations -----
    std::string loadFile(const std::string& filepath);
    bool fileExists(const std::string& filepath);
};

#endif // TEXT_VIEWER_H
