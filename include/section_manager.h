#ifndef SECTION_MANAGER_H
#define SECTION_MANAGER_H

#include <gtk/gtk.h>
#include <vector>
#include <memory>
#include <string>

class TextSection;

class SectionManager {
public:
    SectionManager(GtkWidget* text_container, GtkWidget* order_box);
    ~SectionManager();

    // Section operations
    void addSection(const std::string& header, const std::string& content = std::string());
    void clearAll();
    
    // Main section operations
    void showMainSection();
    void hideMainSection();
    void setMainSectionContent(const std::string& content);
    
    // Getters
    int getSectionCount() const { return sections_.size(); }
    bool hasContent() const { return !sections_.empty(); }
    
    // Save/Load operations
    bool saveToFile(const std::string& filepath) const;
    bool loadFromFile(const std::string& filepath);
    
    // Get section data for current order
    std::vector<std::pair<std::string, std::string>> getSectionsInOrder() const;
    
    // Generate AsciiDoc content
    std::string generateAsciiDoc(const std::string& title = "") const;

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
    gint dragged_source_index_;
    
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
