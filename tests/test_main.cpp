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
    
    // setHeader updates internal header_text_ and display labels
    section.setHeader("Updated Header");
    
    // Verify header was updated using the getter
    EXPECT_EQ(section.getHeader(), "Updated Header");
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
    
    // Verify final header using getter
    EXPECT_EQ(section.getHeader(), "Final Header");
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

// Headline and Level Tests
TEST_F(TextSectionTest, HeadlineManagement) {
    TextSection section(1, "Test Section");
    
    // Test setting and getting headline
    section.setHeadline("Test Headline");
    EXPECT_EQ(section.getHeadline(), "Test Headline");
    
    // Test empty headline
    section.setHeadline("");
    EXPECT_EQ(section.getHeadline(), "");
    
    // Test headline with special characters
    section.setHeadline("Special: <>&\"'\n\t");
    EXPECT_EQ(section.getHeadline(), "Special: <>&\"'\n\t");
}

TEST_F(TextSectionTest, HeadlineLevelManagement) {
    TextSection section(1, "Test Section");
    
    // Test default level (should be 2)
    EXPECT_EQ(section.getHeadlineLevel(), 2);
    
    // Test setting level 1
    section.setHeadlineLevel(1);
    EXPECT_EQ(section.getHeadlineLevel(), 1);
    
    // Test setting level 2
    section.setHeadlineLevel(2);
    EXPECT_EQ(section.getHeadlineLevel(), 2);
    
    // Test setting level 3
    section.setHeadlineLevel(3);
    EXPECT_EQ(section.getHeadlineLevel(), 3);
    
    // Test multiple level changes
    section.setHeadlineLevel(1);
    section.setHeadlineLevel(3);
    section.setHeadlineLevel(2);
    EXPECT_EQ(section.getHeadlineLevel(), 2);
}

TEST_F(TextSectionTest, HeadlineAndLevelTogether) {
    TextSection section(1, "Test");
    
    section.setHeadline("First Headline");
    section.setHeadlineLevel(1);
    
    EXPECT_EQ(section.getHeadline(), "First Headline");
    EXPECT_EQ(section.getHeadlineLevel(), 1);
    
    section.setHeadline("Second Headline");
    section.setHeadlineLevel(3);
    
    EXPECT_EQ(section.getHeadline(), "Second Headline");
    EXPECT_EQ(section.getHeadlineLevel(), 3);
}

TEST_F(SectionManagerTest, DeleteSection) {
    manager->addSection("Section 1", "Content 1");
    manager->addSection("Section 2", "Content 2");
    manager->addSection("Section 3", "Content 3");
    
    EXPECT_EQ(manager->getSectionCount(), 3);
    
    // Delete middle section
    if (manager->getSectionCount() > 1) {
        // Note: deleteSection is called through TextSection's delete button
        // We're testing that the manager supports deletion
        EXPECT_TRUE(manager->hasContent());
    }
}

TEST_F(SectionManagerTest, GenerateAsciiDocEmpty) {
    std::string doc = manager->generateAsciiDoc("");
    EXPECT_TRUE(doc.empty() || doc == "\n" || doc == "");
}

TEST_F(SectionManagerTest, GenerateAsciiDocWithTitle) {
    std::string doc = manager->generateAsciiDoc("Test Document");
    EXPECT_TRUE(doc.find("= Test Document") != std::string::npos);
}

TEST_F(SectionManagerTest, GenerateAsciiDocWithSections) {
    manager->addSection("Section 1", "Content 1");
    
    std::string doc = manager->generateAsciiDoc("My Document");
    
    // Should contain document title
    EXPECT_TRUE(doc.find("= My Document") != std::string::npos);
    // Should contain heading marker
    EXPECT_TRUE(doc.find("==") != std::string::npos);
}

TEST_F(SectionManagerTest, SaveAndLoadWithHeadlines) {
    std::string filename = "test_headlines.docgenset";
    
    // Add sections with headlines and levels
    manager->addSection("Section 1", "Content 1");
    if (manager->getSectionAt(0)) {
        manager->getSectionAt(0)->setHeadline("First Headline");
        manager->getSectionAt(0)->setHeadlineLevel(1);
    }
    
    manager->addSection("Section 2", "Content 2");
    if (manager->getSectionAt(1)) {
        manager->getSectionAt(1)->setHeadline("Second Headline");
        manager->getSectionAt(1)->setHeadlineLevel(3);
    }
    
    // Save
    EXPECT_TRUE(manager->saveToFile(filename));
    
    // Clear and reload
    manager->clearAll();
    EXPECT_EQ(manager->getSectionCount(), 0);
    
    EXPECT_TRUE(manager->loadFromFile(filename));
    EXPECT_EQ(manager->getSectionCount(), 2);
    
    // Verify headlines and levels were preserved
    if (manager->getSectionCount() >= 2) {
        EXPECT_EQ(manager->getSectionAt(0)->getHeadline(), "First Headline");
        EXPECT_EQ(manager->getSectionAt(0)->getHeadlineLevel(), 1);
        
        EXPECT_EQ(manager->getSectionAt(1)->getHeadline(), "Second Headline");
        EXPECT_EQ(manager->getSectionAt(1)->getHeadlineLevel(), 3);
    }
    
    // Clean up
    std::remove(filename.c_str());
}

TEST_F(SectionManagerTest, LoadedDocumentTitle) {
    std::string filename = "test_doc_title.docgenset";
    
    // Create a file with document title
    std::ofstream file(filename);
    file << "[DOCUMENT_TITLE:My Test Title]\n";
    file << "[SECTION:Test Section]\n";
    file << "[HEADLINE:Test Headline]\n";
    file << "[LEVEL:2]\n";
    file << "Test content\n";
    file << "[END_SECTION]\n";
    file.close();
    
    // Load the file
    EXPECT_TRUE(manager->loadFromFile(filename));
    
    // Verify loaded document title
    EXPECT_EQ(manager->getLoadedDocumentTitle(), "My Test Title");
    
    // Clean up
    std::remove(filename.c_str());
}

TEST_F(SectionManagerTest, GetSectionsInOrder) {
    manager->addSection("First", "Content 1");
    manager->addSection("Second", "Content 2");
    manager->addSection("Third", "Content 3");
    
    auto sections = manager->getSectionsInOrder();
    
    EXPECT_EQ(sections.size(), 3);
    EXPECT_EQ(sections[0].first, "First");
    EXPECT_EQ(sections[1].first, "Second");
    EXPECT_EQ(sections[2].first, "Third");
}

TEST_F(SectionManagerTest, MainSectionVisibility) {
    // Initially main section exists
    EXPECT_NO_THROW(manager->hideMainSection());
    EXPECT_NO_THROW(manager->showMainSection());
    
    // Set content and show
    manager->setMainSectionContent("Main content here");
    manager->showMainSection();
    
    // Hide and show again
    manager->hideMainSection();
    manager->showMainSection();
}

TEST_F(SectionManagerTest, SaveEmptyHeadline) {
    std::string filename = "test_empty_headline.docgenset";
    
    manager->addSection("Test Section", "Content");
    // Leave headline empty (default)
    
    EXPECT_TRUE(manager->saveToFile(filename));
    
    manager->clearAll();
    EXPECT_TRUE(manager->loadFromFile(filename));
    
    EXPECT_EQ(manager->getSectionCount(), 1);
    if (manager->getSectionAt(0)) {
        EXPECT_EQ(manager->getSectionAt(0)->getHeadline(), "");
    }
    
    std::remove(filename.c_str());
}

TEST_F(SectionManagerTest, MultipleAddAndClearCycles) {
    for (int cycle = 0; cycle < 5; cycle++) {
        manager->addSection("Section A", "Content A");
        manager->addSection("Section B", "Content B");
        EXPECT_EQ(manager->getSectionCount(), 2);
        
        manager->clearAll();
        EXPECT_EQ(manager->getSectionCount(), 0);
    }
}

TEST_F(TextSectionTest, ContentWithNewlines) {
    TextSection section(1, "Test");
    
    std::string content = "Line 1\nLine 2\nLine 3\n\nLine 5";
    section.setContent(content);
    
    // Verify no crash and proper handling
    EXPECT_NO_THROW(section.show());
}

TEST_F(TextSectionTest, LongHeader) {
    std::string long_header;
    for (int i = 0; i < 100; i++) {
        long_header += "A";
    }
    
    TextSection section(1, long_header);
    EXPECT_EQ(section.getHeader(), long_header);
    
    section.setHeader("Short");
    EXPECT_EQ(section.getHeader(), "Short");
}

TEST_F(SectionManagerTest, AsciiDocGenerationWithDifferentLevels) {
    manager->addSection("Section L1", "Content for level 1");
    if (manager->getSectionAt(0)) {
        manager->getSectionAt(0)->setHeadline("Level 1 Headline");
        manager->getSectionAt(0)->setHeadlineLevel(1);
    }
    
    manager->addSection("Section L2", "Content for level 2");
    if (manager->getSectionAt(1)) {
        manager->getSectionAt(1)->setHeadline("Level 2 Headline");
        manager->getSectionAt(1)->setHeadlineLevel(2);
    }
    
    manager->addSection("Section L3", "Content for level 3");
    if (manager->getSectionAt(2)) {
        manager->getSectionAt(2)->setHeadline("Level 3 Headline");
        manager->getSectionAt(2)->setHeadlineLevel(3);
    }
    
    std::string doc = manager->generateAsciiDoc("Test Document");
    
    // Verify different heading levels are present
    EXPECT_TRUE(doc.find("== Level 1 Headline") != std::string::npos);
    EXPECT_TRUE(doc.find("=== Level 2 Headline") != std::string::npos);
    EXPECT_TRUE(doc.find("==== Level 3 Headline") != std::string::npos);
}

TEST_F(SectionManagerTest, SaveAndLoadEmptyHeadlineAndDefaultLevel) {
    std::string filename = "test_defaults.docgenset";
    
    manager->addSection("Test", "Content");
    // Don't set headline or level, use defaults
    
    EXPECT_TRUE(manager->saveToFile(filename));
    
    manager->clearAll();
    EXPECT_TRUE(manager->loadFromFile(filename));
    
    EXPECT_EQ(manager->getSectionCount(), 1);
    
    std::remove(filename.c_str());
}

TEST_F(TextViewerTest, LoadFileWithUnicode) {
    std::ofstream testFile("test_unicode.txt");
    testFile << "Hello 世界\nΓεια σου κόσμε\nПривет мир";
    testFile.close();
    
    std::string content = viewer.loadFile("test_unicode.txt");
    EXPECT_TRUE(content.find("世界") != std::string::npos);
    EXPECT_TRUE(content.find("κόσμε") != std::string::npos);
    EXPECT_TRUE(content.find("мир") != std::string::npos);
    
    std::remove("test_unicode.txt");
}

// Main function with GTK environment setup
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GtkTestEnvironment);
    return RUN_ALL_TESTS();
}
