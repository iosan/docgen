// =====================
// TextSection.h
// =====================
// Represents a document section with headline, type, and content.
// Handles GTK UI creation and user interaction.
// =====================

#ifndef TEXT_SECTION_H
#define TEXT_SECTION_H

#include <gtk/gtk.h>
#include <string>

class SectionManager;

class TextSection {
public:
    // ----- Construction & Destruction -----
    TextSection(int position, const std::string& default_header);
    ~TextSection();

    // ----- Data Getters -----
    GtkWidget* getContainer() const; // Main section container
    GtkWidget* getOrderButton() const; // Order button widget
    int getPosition() const; // Section position
    std::string getHeader() const; // Section header
    std::string getHeadline() const; // Headline text
    int getHeadlineLevel() const; // Headline level (I/II/III)
    std::string getSectionType() const; // Section type (text/quote/box)

    // ----- Data Setters -----
    void setHeader(const std::string& header); // Set section header
    void setContent(const std::string& content); // Set section content
    void setPosition(int position); // Set section position
    void setHeadline(const std::string& headline); // Set headline text
    void setHeadlineLevel(int level); // Set headline level
    void setSectionType(const std::string& type); // Set section type
    void setManager(SectionManager* manager) { manager_ = manager; } // Set parent manager

    // ----- UI Visibility -----
    void show(); // Show section UI
    void hide(); // Hide section UI

private:
    // ----- Data Members -----
    int position_;
    std::string header_text_;
    SectionManager* manager_;

    // ----- GTK Widgets -----
    GtkWidget* container_ = nullptr;
    GtkWidget* delete_button_ = nullptr;
    GtkWidget* header_label_ = nullptr;
    GtkWidget* text_view_ = nullptr;
    GtkWidget* scrolled_window_ = nullptr;
    GtkWidget* order_button_ = nullptr;
    GtkWidget* order_label_ = nullptr;
    GtkWidget* order_level_label_ = nullptr;
    GtkWidget* radio_group_box_ = nullptr;
    GtkWidget* radio_i_ = nullptr;
    GtkWidget* radio_ii_ = nullptr;
    GtkWidget* radio_iii_ = nullptr;
    GtkWidget* headline_entry_ = nullptr;
    GtkWidget* type_text_ = nullptr;
    GtkWidget* type_quote_ = nullptr;
    GtkWidget* type_box_ = nullptr;

    // ----- UI Setup -----
    void createUI(const std::string& default_header);

    // ----- GTK Signal Callbacks -----
    static void onRadioChanged(GtkToggleButton* button, gpointer user_data);
    static void onTypeRadioChanged(GtkToggleButton* button, gpointer user_data);
    static void onDeleteClicked(GtkButton* button, gpointer user_data);
    static void onHeadlineChanged(GtkEditable* editable, gpointer user_data);

    // ----- UI Helpers -----
    void updateLevelIndicator(); // Update I/II/III indicator
};

#endif // TEXT_SECTION_H
