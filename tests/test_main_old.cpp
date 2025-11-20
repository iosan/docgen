#include <gtest/gtest.h>
#include "text_viewer.h"
#include <fstream>
#include <cstdio>

// Test file loading
TEST(TextViewerTest, LoadValidFile) {
    TextViewer viewer;
    
    // Create a temporary test file
    std::ofstream testFile("test_file.txt");
    testFile << "Hello, World!\nThis is a test file.";
    testFile.close();
    
    std::string content = viewer.loadFile("test_file.txt");
    EXPECT_EQ(content, "Hello, World!\nThis is a test file.");
    
    // Clean up
    std::remove("test_file.txt");
}

// Test file exists check
TEST(TextViewerTest, FileExists) {
    TextViewer viewer;
    
    // Create a temporary test file
    std::ofstream testFile("test_exists.txt");
    testFile << "test";
    testFile.close();
    
    EXPECT_TRUE(viewer.fileExists("test_exists.txt"));
    EXPECT_FALSE(viewer.fileExists("non_existent_file.txt"));
    
    // Clean up
    std::remove("test_exists.txt");
}

// Test loading non-existent file
TEST(TextViewerTest, LoadNonExistentFile) {
    TextViewer viewer;
    EXPECT_THROW(viewer.loadFile("non_existent_file.txt"), std::runtime_error);
}
