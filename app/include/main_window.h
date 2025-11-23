// =====================
// MainWindow.h
// =====================
// Main application window and UI controller.
// Handles menu, preview, and document title management.
// =====================

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <memory>
#include "section_manager.h"
#include "text_viewer.h"

class MainWindow {
public:
    // ----- Construction & Destruction -----
    MainWindow(GtkApplication* app);
    ~MainWindow();

    // ----- UI Control -----
    void show();
    GtkWindow* getWindow() const;
    std::string getDocumentTitle() const;

private:
    // ----- Window & Containers -----
    GtkWidget* window_;
    GtkWidget* main_vbox_;

    // ----- Components -----
    std::unique_ptr<SectionManager> section_manager_;
    TextViewer text_viewer_;
    GtkWidget* document_title_entry_;
    WebKitWebView* preview_web_view_;

    // ----- UI Creation Methods -----
    void createMenuBar();
    void createUI();
    void updatePreview();
    std::string convertAsciiDocToHTML(const std::string& asciidoc);

    // ----- Menu Callbacks (static for GTK compatibility) -----
    static void onAddSection(GtkMenuItem* item, gpointer user_data);
    static void onSaveSet(GtkMenuItem* item, gpointer user_data);
    static void onOpenSet(GtkMenuItem* item, gpointer user_data);
    static void onCreateDoc(GtkMenuItem* item, gpointer user_data);
    static void onCreateMarkdown(GtkMenuItem* item, gpointer user_data);
    static void onClearAll(GtkMenuItem* item, gpointer user_data);
    static void onQuit(GtkMenuItem* item, gpointer user_data);
    static void onAbout(GtkMenuItem* item, gpointer user_data);

    // ----- Helper Methods -----
    bool promptSaveIfNeeded();
    void updateTitle();

    // ----- State Tracking -----
    bool has_unsaved_changes_;
    std::string current_set_file_;
};

#endif // MAIN_WINDOW_H
