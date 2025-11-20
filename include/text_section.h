#ifndef TEXT_SECTION_H
#define TEXT_SECTION_H

#include <gtk/gtk.h>
#include <string>

class TextSection {
public:
    TextSection(int position, const std::string& default_header);
    ~TextSection();

    // Getters
    GtkWidget* getContainer() const { return container_; }
    GtkWidget* getOrderButton() const { return order_button_; }
    int getPosition() const { return position_; }
    std::string getHeader() const;
    
    // Setters
    void setHeader(const std::string& header);
    void setContent(const std::string& content);
    void setPosition(int position);
    
    // Visibility
    void show();
    void hide();

private:
    int position_;
    GtkWidget* container_;        // Main section container
    GtkWidget* header_entry_;     // Header input field
    GtkWidget* header_label_;     // Header display label
    GtkWidget* text_view_;        // Text content view
    GtkWidget* scrolled_window_;  // Scrolled window for text view
    GtkWidget* order_button_;     // Button in order box
    GtkWidget* order_label_;      // Label in order button

    void createUI(const std::string& default_header);
    
    // Callback for header changes
    static void onHeaderChanged(GtkEntry* entry, gpointer user_data);
};

#endif // TEXT_SECTION_H
