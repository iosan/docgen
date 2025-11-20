#include <gtest/gtest.h>
#include "text_viewer.h"
#include "text_section.h"
#include "section_manager.h"
#include <gtk/gtk.h>
#include <fstream>
#include <cstdio>

// Initialize GTK for testing
class GtkTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        int argc = 0;
        char** argv = nullptr;
        gtk_init(&argc, &argv);
    }
};

// TextViewer Tests
class TextViewerTest : public ::testing::Test {
protected:
    TextViewer viewer;
};

TEST_F(TextViewerTest, LoadValidFile) {
    // Create a temporary test file
    std::ofstream testFile("test_file.txt");
    testFile << "Hello, World!\nThis is a test file.";
    testFile.close();
    
    std::string content = viewer.loadFile("test_file.txt");
    EXPECT_EQ(content, "Hello, World!\nThis is a test file.");
    
    // Clean up
    std::remove("test_file.txt");
}

TEST_F(TextViewerTest, FileExists) {
    // Create a temporary test file
    std::ofstream testFile("test_exists.txt");
    testFile << "test";
    testFile.close();
    
    EXPECT_TRUE(viewer.fileExists("test_exists.txt"));
    EXPECT_FALSE(viewer.fileExists("non_existent_file.txt"));
    
    // Clean up
    std::remove("test_exists.txt");
}

TEST_F(TextViewerTest, LoadNonExistentFile) {
    EXPECT_THROW(viewer.loadFile("non_existent_file.txt"), std::runtime_error);
}

TEST_F(TextViewerTest, LoadEmptyFile) {
    // Create an empty test file
    std::ofstream testFile("empty_file.txt");
    testFile.close();
    
    std::string content = viewer.loadFile("empty_file.txt");
    EXPECT_EQ(content, "");
    
    // Clean up
    std::remove("empty_file.txt");
}

// TextSection Tests
class TextSectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // GTK needs to be initialized
    }
};

TEST_F(TextSectionTest, Construction) {
    TextSection section(1, "Test Section");
    
    EXPECT_EQ(section.getPosition(), 1);
    EXPECT_EQ(section.getHeader(), "Test Section");
    EXPECT_NE(section.getContainer(), nullptr);
    EXPECT_NE(section.getOrderButton(), nullptr);
    
    // Verify GTK widgets were created properly
    EXPECT_TRUE(GTK_IS_WIDGET(section.getContainer()));
    EXPECT_TRUE(GTK_IS_BUTTON(section.getOrderButton()));
}

TEST_F(TextSectionTest, SetAndGetHeader) {
    TextSection section(1, "Initial Header");
    
    EXPECT_EQ(section.getHeader(), "Initial Header");
    
    // setHeader updates display label and order button, not the entry
    section.setHeader("Updated Header");
    
    // Get label text from order button to verify update
    GtkWidget* order_button = section.getOrderButton();
    GtkWidget* label = gtk_bin_get_child(GTK_BIN(order_button));
    const char* label_text = gtk_label_get_text(GTK_LABEL(label));
    EXPECT_STREQ(label_text, "Updated Header");
}

TEST_F(TextSectionTest, SetContent) {
    TextSection section(1, "Test");
    
    std::string test_content = "Test content\nLine 2\nLine 3";
    EXPECT_NO_THROW(section.setContent(test_content));
    
    // Test with empty content
    EXPECT_NO_THROW(section.setContent(""));
    
    // Test with special characters
    EXPECT_NO_THROW(section.setContent("Special: <>&\"'\n\t"));
}

TEST_F(TextSectionTest, PositionUpdate) {
    TextSection section(1, "Test");
    
    EXPECT_EQ(section.getPosition(), 1);
    
    section.setPosition(5);
    EXPECT_EQ(section.getPosition(), 5);
    
    section.setPosition(0);
    EXPECT_EQ(section.getPosition(), 0);
    
    section.setPosition(100);
    EXPECT_EQ(section.getPosition(), 100);
}

TEST_F(TextSectionTest, VisibilityControl) {
    TextSection section(1, "Test");
    
    // Show the section
    EXPECT_NO_THROW(section.show());
    EXPECT_TRUE(gtk_widget_get_visible(section.getContainer()));
    
    // Hide the section
    EXPECT_NO_THROW(section.hide());
    EXPECT_FALSE(gtk_widget_get_visible(section.getContainer()));
    
    // Show again
    section.show();
    EXPECT_TRUE(gtk_widget_get_visible(section.getContainer()));
}

TEST_F(TextSectionTest, MultipleHeaderUpdates) {
    TextSection section(1, "Original");
    
    section.setHeader("Update 1");
    section.setHeader("Update 2");
    section.setHeader("Final Header");
    
    GtkWidget* order_button = section.getOrderButton();
    GtkWidget* label = gtk_bin_get_child(GTK_BIN(order_button));
    const char* label_text = gtk_label_get_text(GTK_LABEL(label));
    EXPECT_STREQ(label_text, "Final Header");
}

// SectionManager Tests
class SectionManagerTest : public ::testing::Test {
protected:
    GtkWidget* text_container;
    GtkWidget* order_box;
    SectionManager* manager;
    
    void SetUp() override {
        text_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        order_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        manager = new SectionManager(text_container, order_box);
    }
    
    void TearDown() override {
        delete manager;
        gtk_widget_destroy(text_container);
        gtk_widget_destroy(order_box);
    }
};

TEST_F(SectionManagerTest, InitialState) {
    EXPECT_EQ(manager->getSectionCount(), 0);
    EXPECT_FALSE(manager->hasContent());
}

TEST_F(SectionManagerTest, AddSection) {
    manager->addSection("Section 1", "Content 1");
    
    EXPECT_EQ(manager->getSectionCount(), 1);
    EXPECT_TRUE(manager->hasContent());
}

TEST_F(SectionManagerTest, AddMultipleSections) {
    manager->addSection("Section 1", "Content 1");
    manager->addSection("Section 2", "Content 2");
    manager->addSection("Section 3", "Content 3");
    
    EXPECT_EQ(manager->getSectionCount(), 3);
    EXPECT_TRUE(manager->hasContent());
}

TEST_F(SectionManagerTest, AddSectionWithoutContent) {
    manager->addSection("Empty Section");
    
    EXPECT_EQ(manager->getSectionCount(), 1);
    EXPECT_TRUE(manager->hasContent());
}

TEST_F(SectionManagerTest, ClearAll) {
    manager->addSection("Section 1", "Content 1");
    manager->addSection("Section 2", "Content 2");
    
    EXPECT_EQ(manager->getSectionCount(), 2);
    
    manager->clearAll();
    
    EXPECT_EQ(manager->getSectionCount(), 0);
    EXPECT_FALSE(manager->hasContent());
}

TEST_F(SectionManagerTest, ClearEmptyManager) {
    // Should not crash when clearing empty manager
    EXPECT_NO_THROW(manager->clearAll());
    EXPECT_EQ(manager->getSectionCount(), 0);
}

TEST_F(SectionManagerTest, MainSectionContent) {
    std::string test_content = "Main section test content\nWith multiple lines";
    
    EXPECT_NO_THROW(manager->setMainSectionContent(test_content));
    EXPECT_NO_THROW(manager->showMainSection());
    EXPECT_NO_THROW(manager->hideMainSection());
    
    // Set content multiple times
    manager->setMainSectionContent("Content 1");
    manager->setMainSectionContent("Content 2");
    manager->setMainSectionContent("");
}

TEST_F(SectionManagerTest, WidgetIntegration) {
    manager->addSection("Test Section", "Test Content");
    
    // Check that widgets were added to containers
    GList* text_children = gtk_container_get_children(GTK_CONTAINER(text_container));
    GList* order_children = gtk_container_get_children(GTK_CONTAINER(order_box));
    
    // Should have main section + added section
    EXPECT_GE(g_list_length(text_children), 1);
    EXPECT_GE(g_list_length(order_children), 1);
    
    // Verify widgets are valid GTK widgets
    for (GList* l = text_children; l != NULL; l = l->next) {
        EXPECT_TRUE(GTK_IS_WIDGET(l->data));
    }
    for (GList* l = order_children; l != NULL; l = l->next) {
        EXPECT_TRUE(GTK_IS_WIDGET(l->data));
    }
    
    g_list_free(text_children);
    g_list_free(order_children);
}

TEST_F(SectionManagerTest, ClearAllRemovesWidgets) {
    manager->addSection("Section 1", "Content 1");
    manager->addSection("Section 2", "Content 2");
    
    manager->clearAll();
    
    GList* text_children = gtk_container_get_children(GTK_CONTAINER(text_container));
    GList* order_children = gtk_container_get_children(GTK_CONTAINER(order_box));
    
    // Should only have main section left (which is hidden)
    int text_count = g_list_length(text_children);
    int order_count = g_list_length(order_children);
    
    g_list_free(text_children);
    g_list_free(order_children);
    
    // After clear, non-main sections should be removed
    EXPECT_LE(text_count, 1);  // Only main section
    EXPECT_LE(order_count, 1); // Only main order button
}

TEST_F(SectionManagerTest, AddManySection) {
    // Add multiple sections to test counter and management
    for (int i = 1; i <= 10; i++) {
        std::string header = "Section " + std::to_string(i);
        std::string content = "Content for section " + std::to_string(i);
        manager->addSection(header, content);
    }
    
    EXPECT_EQ(manager->getSectionCount(), 10);
    EXPECT_TRUE(manager->hasContent());
    
    manager->clearAll();
    EXPECT_EQ(manager->getSectionCount(), 0);
}

TEST_F(SectionManagerTest, AlternateClearAndAdd) {
    manager->addSection("Section 1", "Content 1");
    EXPECT_EQ(manager->getSectionCount(), 1);
    
    manager->clearAll();
    EXPECT_EQ(manager->getSectionCount(), 0);
    
    manager->addSection("Section 2", "Content 2");
    EXPECT_EQ(manager->getSectionCount(), 1);
    
    manager->addSection("Section 3", "Content 3");
    EXPECT_EQ(manager->getSectionCount(), 2);
    
    manager->clearAll();
    EXPECT_EQ(manager->getSectionCount(), 0);
}

TEST_F(SectionManagerTest, LongContent) {
    std::string long_content;
    for (int i = 0; i < 100; i++) {
        long_content += "This is line " + std::to_string(i) + " of the long content.\n";
    }
    
    EXPECT_NO_THROW(manager->addSection("Long Section", long_content));
    EXPECT_EQ(manager->getSectionCount(), 1);
}

TEST_F(SectionManagerTest, SpecialCharactersInContent) {
    std::string special_content = "Special chars: <>&\"'\n\t@#$%^&*()[]{}|\\";
    EXPECT_NO_THROW(manager->addSection("Special", special_content));
    EXPECT_EQ(manager->getSectionCount(), 1);
}

TEST_F(SectionManagerTest, SaveAndLoadEmptySet) {
    std::string filename = "test_empty_set.txt";
    
    // Save empty set
    EXPECT_TRUE(manager->saveToFile(filename));
    
    // Load it back
    EXPECT_TRUE(manager->loadFromFile(filename));
    EXPECT_EQ(manager->getSectionCount(), 0);
    
    // Clean up
    std::remove(filename.c_str());
}

TEST_F(SectionManagerTest, SaveAndLoadSingleSection) {
    std::string filename = "test_single_section.txt";
    
    // Add a section
    manager->addSection("Test Header", "Test Content\nLine 2\nLine 3");
    EXPECT_EQ(manager->getSectionCount(), 1);
    
    // Save it
    EXPECT_TRUE(manager->saveToFile(filename));
    
    // Clear and reload
    manager->clearAll();
    EXPECT_EQ(manager->getSectionCount(), 0);
    
    EXPECT_TRUE(manager->loadFromFile(filename));
    EXPECT_EQ(manager->getSectionCount(), 1);
    
    // Clean up
    std::remove(filename.c_str());
}

TEST_F(SectionManagerTest, SaveAndLoadMultipleSections) {
    std::string filename = "test_multiple_sections.txt";
    
    // Add multiple sections
    manager->addSection("Header 1", "Content 1");
    manager->addSection("Header 2", "Content 2\nWith multiple lines");
    manager->addSection("Header 3", "Content 3");
    EXPECT_EQ(manager->getSectionCount(), 3);
    
    // Save it
    EXPECT_TRUE(manager->saveToFile(filename));
    
    // Clear and reload
    manager->clearAll();
    EXPECT_EQ(manager->getSectionCount(), 0);
    
    EXPECT_TRUE(manager->loadFromFile(filename));
    EXPECT_EQ(manager->getSectionCount(), 3);
    
    // Clean up
    std::remove(filename.c_str());
}

TEST_F(SectionManagerTest, LoadNonExistentFile) {
    // Should return false for non-existent file
    EXPECT_FALSE(manager->loadFromFile("non_existent_file_123456.txt"));
}

TEST_F(SectionManagerTest, SaveToInvalidPath) {
    manager->addSection("Test", "Content");
    // Should return false for invalid path
    EXPECT_FALSE(manager->saveToFile("/invalid/path/that/does/not/exist/file.txt"));
}

// Main function with GTK environment setup
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GtkTestEnvironment);
    return RUN_ALL_TESTS();
}
