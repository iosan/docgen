#ifndef TEXT_SECTION_H
#define TEXT_SECTION_H

#include <gtk/gtk.h>
#include <string>

class SectionManager;

class TextSection {
public:
    TextSection(int position, const std::string& default_header);
    ~TextSection();

    // Getters
    GtkWidget* getContainer() const { return container_; }
    GtkWidget* getOrderButton() const { return order_button_; }
    int getPosition() const { return position_; }
    std::string getHeader() const;
    std::string getHeadline() const;
    int getHeadlineLevel() const;
    
    // Setters
    void setHeader(const std::string& header);
    void setContent(const std::string& content);
    void setPosition(int position);
    void setHeadline(const std::string& headline);
    void setHeadlineLevel(int level);
    void setManager(SectionManager* manager) { manager_ = manager; }
    
    // Visibility
    void show();
    void hide();

private:
    int position_;
    std::string header_text_;     // Stored header text
    SectionManager* manager_;     // Parent manager for deletion
    GtkWidget* container_;        // Main section container
    GtkWidget* delete_button_;    // Delete button with minus sign
    GtkWidget* header_label_;     // Header display label
    GtkWidget* text_view_;        // Text content view
    GtkWidget* scrolled_window_;  // Scrolled window for text view
    GtkWidget* order_button_;     // Button in order box
    GtkWidget* order_label_;      // Label in order button
    GtkWidget* order_level_label_; // Small level indicator (I/II/III) in order button
    GtkWidget* radio_group_box_;  // Box containing radio buttons
    GtkWidget* radio_i_;          // Radio button I (headline level 1)
    GtkWidget* radio_ii_;         // Radio button II (headline level 2)
    GtkWidget* radio_iii_;        // Radio button III (headline level 3)
    GtkWidget* headline_entry_;   // Headline text input field

    void createUI(const std::string& default_header);
    
    // Callbacks
    static void onRadioChanged(GtkToggleButton* button, gpointer user_data);
    static void onDeleteClicked(GtkButton* button, gpointer user_data);
    void updateLevelIndicator();
};

#endif // TEXT_SECTION_H
