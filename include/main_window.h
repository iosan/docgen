#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>
#include <memory>
#include "section_manager.h"
#include "text_viewer.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();

    void show();
    GtkWindow* getWindow() const { return GTK_WINDOW(window_); }
    std::string getDocumentTitle() const;

private:
    // Window and main container
    GtkWidget* window_;
    GtkWidget* main_vbox_;
    
    // Components
    std::unique_ptr<SectionManager> section_manager_;
    TextViewer text_viewer_;
    GtkWidget* document_title_entry_;

    // UI Creation methods
    void createMenuBar();
    void createUI();
    
    // Menu callbacks (static for GTK compatibility)
    static void onAddSection(GtkMenuItem* item, gpointer user_data);
    static void onSaveSet(GtkMenuItem* item, gpointer user_data);
    static void onOpenSet(GtkMenuItem* item, gpointer user_data);
    static void onCreateDoc(GtkMenuItem* item, gpointer user_data);
    static void onClearAll(GtkMenuItem* item, gpointer user_data);
    static void onQuit(GtkMenuItem* item, gpointer user_data);
    static void onAbout(GtkMenuItem* item, gpointer user_data);
    
    // Helper methods
    bool promptSaveIfNeeded();
    void updateTitle();
    
    // Track if content has been modified
    bool has_unsaved_changes_;
    std::string current_set_file_;
};

#endif // MAIN_WINDOW_H
