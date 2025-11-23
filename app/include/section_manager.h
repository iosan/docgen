// =====================
// SectionManager.h
// =====================
// Manages all TextSection objects and document operations.
// Handles section ordering, saving/loading, and content generation.
// =====================

#ifndef SECTION_MANAGER_H
#define SECTION_MANAGER_H

#include <gtk/gtk.h>
#include <vector>
#include <memory>
#include <string>
#include <functional>

class TextSection;

class SectionManager {
public:
    // ----- Construction & Destruction -----
    SectionManager(GtkWidget* text_container, GtkWidget* order_box);
    ~SectionManager();

    // ----- Content Change Callbacks -----
    void setOnContentChangedCallback(std::function<void()> callback);
    void notifyContentChanged();

    // ----- Section Operations -----
    void addSection(const std::string& header, const std::string& content = std::string());
    void deleteSection(TextSection* section);
    void clearAll();

    // ----- Main Section Operations -----
    void showMainSection();
    void hideMainSection();
    void setMainSectionContent(const std::string& content);

    // ----- Getters -----
    int getSectionCount() const;
    bool hasContent() const;
    std::string getLoadedDocumentTitle() const;
    TextSection* getSectionAt(size_t index) const;

    // ----- Save/Load Operations -----
    bool saveToFile(const std::string& filepath) const;
    bool loadFromFile(const std::string& filepath);

    // ----- Section Data Access -----
    std::vector<std::pair<std::string, std::string>> getSectionsInOrder() const;

    // ----- Document Generation -----
    std::string generateAsciiDoc(const std::string& title = "") const;
    std::string generateMarkdown(const std::string& title = "") const;

private:
    GtkWidget* text_container_;
    GtkWidget* order_box_;
    
    std::vector<std::unique_ptr<TextSection>> sections_;
    int section_counter_;
    
    // Main section widgets
    GtkWidget* main_section_;
    GtkWidget* main_order_button_;
    GtkWidget* main_text_view_;
    
    // Drag state
    GtkWidget* dragged_widget_;
    std::string loaded_document_title_;
    gint dragged_source_index_;
    
    // Callback for content changes
    std::function<void()> on_content_changed_;
    
    void createMainSection();
    void setupDragAndDrop(GtkWidget* order_button, int position);
    
    // Drag and drop callbacks
    static void onDragBegin(GtkWidget* widget, GdkDragContext* context, gpointer user_data);
    static void onDragDataGet(GtkWidget* widget, GdkDragContext* context,
                              GtkSelectionData* selection_data, guint info,
                              guint time, gpointer user_data);
    static void onDragDataReceived(GtkWidget* widget, GdkDragContext* context,
                                   gint x, gint y, GtkSelectionData* selection_data,
                                   guint info, guint time, gpointer user_data);
    static gboolean onDragMotion(GtkWidget* widget, GdkDragContext* context,
                                 gint x, gint y, guint time, gpointer user_data);
    static void onDragLeave(GtkWidget* widget, GdkDragContext* context,
                            guint time, gpointer user_data);
    static void onDragEnd(GtkWidget* widget, GdkDragContext* context, gpointer user_data);
    static gboolean onOrderBoxDragMotion(GtkWidget* widget, GdkDragContext* context,
                                         gint x, gint y, guint time, gpointer user_data);
};

#endif // SECTION_MANAGER_H
