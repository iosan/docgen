// =====================
// TextSection.cpp
// =====================
// Implements TextSection UI and logic
// =====================

#include "text_section.h"
#include "section_manager.h"
#include <gtk/gtk.h>
#include <cstring>

// ----- Construction & Destruction -----
TextSection::TextSection(int position, const std::string& default_header)
    : position_(position), header_text_(default_header), manager_(nullptr), container_(nullptr),
      delete_button_(nullptr), header_label_(nullptr), text_view_(nullptr), scrolled_window_(nullptr),
      order_button_(nullptr), order_label_(nullptr), order_level_label_(nullptr),
      radio_group_box_(nullptr), radio_i_(nullptr), radio_ii_(nullptr), radio_iii_(nullptr),
      headline_entry_(nullptr), type_text_(nullptr), type_quote_(nullptr), type_box_(nullptr) {
    createUI(default_header);
}

TextSection::~TextSection() {
    // GTK widgets are destroyed when their parent container is destroyed
}

// ----- UI Setup -----
// Creates all GTK widgets and layouts for the section
void TextSection::createUI(const std::string& default_header) {
    // Create main container with modern styling
    container_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkStyleContext* container_context = gtk_widget_get_style_context(container_);
    gtk_style_context_add_class(container_context, "section-container");
    gtk_widget_set_margin_start(container_, 4);
    gtk_widget_set_margin_end(container_, 4);
    gtk_widget_set_margin_top(container_, 4);
    gtk_widget_set_margin_bottom(container_, 4);

    // Create header label with modern styling
    header_label_ = gtk_label_new(default_header.c_str());
    gtk_widget_set_halign(header_label_, GTK_ALIGN_START);
    gtk_widget_set_margin_top(header_label_, 8);
    gtk_widget_set_margin_bottom(header_label_, 8);
    gtk_widget_set_margin_start(header_label_, 4);
    
    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_SEMIBOLD));
    pango_attr_list_insert(attrs, pango_attr_scale_new(1.0));
    pango_attr_list_insert(attrs, pango_attr_foreground_new(0x2000, 0x2000, 0x2000));
    gtk_label_set_attributes(GTK_LABEL(header_label_), attrs);
    pango_attr_list_unref(attrs);
    
    // Create horizontal box for header label, delete button, and radio buttons
    GtkWidget* header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(header_box), header_label_, FALSE, FALSE, 0);
    
    // Create delete button to the right of filename
    delete_button_ = gtk_button_new_with_label("−");  // Minus sign
    gtk_widget_set_size_request(delete_button_, 24, 24);
    gtk_button_set_relief(GTK_BUTTON(delete_button_), GTK_RELIEF_NONE);
    
    // Style the delete button with red color
    GtkStyleContext* delete_context = gtk_widget_get_style_context(delete_button_);
    GtkCssProvider* css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "button { color: red; font-size: 16px; font-weight: bold; min-width: 20px; min-height: 20px; padding: 0; }"
        "button:hover { background-color: rgba(255, 0, 0, 0.1); }"
        ".level-i { color: #006400; }"   /* Dark green for level I */
        ".level-ii { color: #008000; }"  /* Green for level II */
        ".level-iii { color: #90EE90; }", /* Light green for level III */
        -1, NULL);
    gtk_style_context_add_provider(delete_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css_provider);
    
    g_signal_connect(delete_button_, "clicked", G_CALLBACK(onDeleteClicked), this);
    gtk_box_pack_start(GTK_BOX(header_box), delete_button_, FALSE, FALSE, 2);
    
    // Create frame with title "Headline" for radio buttons and input
    GtkWidget* radio_frame = gtk_frame_new("Headline");
    gtk_widget_set_margin_end(radio_frame, 5);
    
    // Create horizontal box inside the frame (input field left of radio buttons)
    GtkWidget* frame_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_start(frame_hbox, 5);
    gtk_widget_set_margin_end(frame_hbox, 5);
    gtk_widget_set_margin_top(frame_hbox, 3);
    gtk_widget_set_margin_bottom(frame_hbox, 3);
    
    // Create headline input field
    headline_entry_ = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(headline_entry_), "Section headline...");
    gtk_widget_set_size_request(headline_entry_, 150, -1);
    gtk_box_pack_start(GTK_BOX(frame_hbox), headline_entry_, FALSE, FALSE, 0);

    // Connect headline entry change signal to refresh preview
    g_signal_connect(headline_entry_, "changed", G_CALLBACK(onHeadlineChanged), this);
    
    // Create horizontal box for radio buttons
    radio_group_box_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    
    // Create radio buttons I, II, III (headline levels 1, 2, 3)
    radio_i_ = gtk_radio_button_new_with_label(NULL, "I");
    radio_ii_ = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_i_), "II");
    radio_iii_ = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_i_), "III");
    
    // Apply green tones to radio button labels
    GtkWidget* label_i = gtk_bin_get_child(GTK_BIN(radio_i_));
    GtkWidget* label_ii = gtk_bin_get_child(GTK_BIN(radio_ii_));
    GtkWidget* label_iii = gtk_bin_get_child(GTK_BIN(radio_iii_));
    
    GtkCssProvider* radio_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(radio_css,
        ".level-i-label { color: #006400; }"     /* Dark green */
        ".level-ii-label { color: #228B22; }"    /* Forest green */
        ".level-iii-label { color: #32CD32; }",  /* Lime green */
        -1, NULL);
    
    GtkStyleContext* ctx_i = gtk_widget_get_style_context(label_i);
    GtkStyleContext* ctx_ii = gtk_widget_get_style_context(label_ii);
    GtkStyleContext* ctx_iii = gtk_widget_get_style_context(label_iii);
    
    gtk_style_context_add_provider(ctx_i, GTK_STYLE_PROVIDER(radio_css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(ctx_ii, GTK_STYLE_PROVIDER(radio_css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(ctx_iii, GTK_STYLE_PROVIDER(radio_css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    gtk_style_context_add_class(ctx_i, "level-i-label");
    gtk_style_context_add_class(ctx_ii, "level-ii-label");
    gtk_style_context_add_class(ctx_iii, "level-iii-label");
    
    g_object_unref(radio_css);
    
    // Set tooltips to explain headline levels
    gtk_widget_set_tooltip_text(radio_i_, "Headline Level 1 (=)");
    gtk_widget_set_tooltip_text(radio_ii_, "Headline Level 2 (==)");
    gtk_widget_set_tooltip_text(radio_iii_, "Headline Level 3 (===)");
    
    // Make radio buttons smaller
    gtk_widget_set_size_request(radio_i_, -1, 18);
    gtk_widget_set_size_request(radio_ii_, -1, 18);
    gtk_widget_set_size_request(radio_iii_, -1, 18);
    
    // Select level 1 by default (=)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_i_), TRUE);
    
    // Connect radio button change signals
    g_signal_connect(radio_i_, "toggled", G_CALLBACK(onRadioChanged), this);
    g_signal_connect(radio_ii_, "toggled", G_CALLBACK(onRadioChanged), this);
    g_signal_connect(radio_iii_, "toggled", G_CALLBACK(onRadioChanged), this);
    
    // Pack radio buttons horizontally
    gtk_box_pack_start(GTK_BOX(radio_group_box_), radio_i_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_group_box_), radio_ii_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_group_box_), radio_iii_, FALSE, FALSE, 0);
    
    // Pack radio buttons to frame hbox
    gtk_box_pack_start(GTK_BOX(frame_hbox), radio_group_box_, FALSE, FALSE, 0);
    
    // Add frame hbox to frame
    gtk_container_add(GTK_CONTAINER(radio_frame), frame_hbox);
    
    // Pack frame to header box
    gtk_box_pack_end(GTK_BOX(header_box), radio_frame, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(container_), header_box, FALSE, FALSE, 0);

    // Create scrolled window for text view with shadow
    scrolled_window_ = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window_), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled_window_, -1, 70);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window_), 70);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scrolled_window_), 300);
    gtk_widget_set_margin_top(scrolled_window_, 2);
    gtk_widget_set_margin_bottom(scrolled_window_, 2);

    // Create text view with monospace font
    text_view_ = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view_), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view_), 4);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view_), 4);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view_), 4);
    gtk_container_add(GTK_CONTAINER(scrolled_window_), text_view_);

    gtk_box_pack_start(GTK_BOX(container_), scrolled_window_, FALSE, TRUE, 0);

    // Create order button as GtkButton for test compatibility
    order_button_ = gtk_button_new();
    GtkStyleContext* button_context = gtk_widget_get_style_context(order_button_);
    gtk_style_context_add_class(button_context, "order-button");
    
    // Create horizontal box for label and level indicator
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(button_box, 4);
    gtk_widget_set_margin_end(button_box, 4);
    gtk_widget_set_margin_top(button_box, 2);
    gtk_widget_set_margin_bottom(button_box, 2);
    
    order_label_ = gtk_label_new(default_header.c_str());
    gtk_box_pack_start(GTK_BOX(button_box), order_label_, TRUE, TRUE, 0);
    
    // Create tiny radio buttons for section type (text/quote/box) - vertical layout
    GtkWidget* type_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_end(type_box, 2);
    gtk_widget_set_valign(type_box, GTK_ALIGN_CENTER);
    
    
    type_text_ = gtk_radio_button_new_with_label(NULL, "text");
    type_quote_ = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(type_text_), "quote");
    type_box_ = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(type_text_), "box");
    
    // Make radio button circles much smaller with CSS
    GtkCssProvider* type_css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(type_css,
        "radiobutton { "
        "  min-height: 0; "
        "  min-width: 0; "
        "  padding: 0; "
        "  margin: 0; "
        "}"
        "radiobutton radio { "
        "  min-height: 8px; "
        "  min-width: 8px; "
        "  padding: 0; "
        "  margin: 0 2px 0 0; "
        "}"
        "radiobutton label { "
        "  padding: 0; "
        "  margin: 0; "
        "  font-size: 8px; "
        "}",
        -1, NULL);
    
    GtkStyleContext* ctx_text = gtk_widget_get_style_context(type_text_);
    GtkStyleContext* ctx_quote = gtk_widget_get_style_context(type_quote_);
    GtkStyleContext* ctx_box = gtk_widget_get_style_context(type_box_);
    
    gtk_style_context_add_provider(ctx_text, GTK_STYLE_PROVIDER(type_css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(ctx_quote, GTK_STYLE_PROVIDER(type_css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(ctx_box, GTK_STYLE_PROVIDER(type_css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(type_css);
    
    // Set text as default
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(type_text_), TRUE);
    
    // Connect type radio button change signals
    g_signal_connect(type_text_, "toggled", G_CALLBACK(onTypeRadioChanged), this);
    g_signal_connect(type_quote_, "toggled", G_CALLBACK(onTypeRadioChanged), this);
    g_signal_connect(type_box_, "toggled", G_CALLBACK(onTypeRadioChanged), this);
    
    gtk_box_pack_start(GTK_BOX(type_box), type_text_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(type_box), type_quote_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(type_box), type_box_, FALSE, FALSE, 0);
    
    gtk_box_pack_end(GTK_BOX(button_box), type_box, FALSE, FALSE, 0);
    
    // Create small level indicator label (I/II/III)
    order_level_label_ = gtk_label_new(NULL);
    gtk_label_set_use_markup(GTK_LABEL(order_level_label_), TRUE);
    gtk_widget_set_halign(order_level_label_, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(order_level_label_, GTK_ALIGN_CENTER);
    gtk_box_pack_end(GTK_BOX(button_box), order_level_label_, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(order_button_), button_box);
    gtk_widget_set_can_focus(order_button_, FALSE);
    
    // Make the event box clickable for dragging but don't interfere with radio buttons
    gtk_widget_set_events(order_button_, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

    // Store position data
    gint* pos_data = g_new(gint, 1);
    *pos_data = position_;
    g_object_set_data_full(G_OBJECT(order_button_), "position", pos_data, g_free);
    
    // Initialize the level indicator with default level (II)
    updateLevelIndicator();
}

// ----- Data Getters -----
GtkWidget* TextSection::getContainer() const { return container_; }
GtkWidget* TextSection::getOrderButton() const { return order_button_; }
int TextSection::getPosition() const { return position_; }
std::string TextSection::getHeader() const { return header_text_; }
std::string TextSection::getHeadline() const {
    const char* text = gtk_entry_get_text(GTK_ENTRY(headline_entry_));
    return text ? std::string(text) : "";
}
int TextSection::getHeadlineLevel() const {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_i_))) return 1;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_ii_))) return 2;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_iii_))) return 3;
    return 2;
}
std::string TextSection::getSectionType() const {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(type_text_))) return "text";
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(type_quote_))) return "quote";
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(type_box_))) return "box";
    return "text";
}

// ----- Data Setters -----
void TextSection::setHeader(const std::string& header) {
    header_text_ = header;
    gtk_label_set_text(GTK_LABEL(header_label_), header.c_str());
    gtk_label_set_text(GTK_LABEL(order_label_), header.c_str());
}
void TextSection::setContent(const std::string& content) {
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_));
    gtk_text_buffer_set_text(buffer, content.c_str(), -1);
}
void TextSection::setPosition(int position) {
    position_ = position;
    gint* pos_data = (gint*)g_object_get_data(G_OBJECT(order_button_), "position");
    if (pos_data) *pos_data = position;
}
void TextSection::setHeadline(const std::string& headline) {
    gtk_entry_set_text(GTK_ENTRY(headline_entry_), headline.c_str());
}
void TextSection::setHeadlineLevel(int level) {
    if (level == 1) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_i_), TRUE);
    else if (level == 2) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_ii_), TRUE);
    else if (level == 3) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_iii_), TRUE);
}
void TextSection::setSectionType(const std::string& type) {
    if (type == "text") gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(type_text_), TRUE);
    else if (type == "quote") gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(type_quote_), TRUE);
    else if (type == "box") gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(type_box_), TRUE);
}

// ----- UI Visibility -----
void TextSection::show() {
    gtk_widget_show_all(container_);
    gtk_widget_show_all(order_button_);
}
void TextSection::hide() {
    gtk_widget_hide(container_);
    gtk_widget_hide(order_button_);
}

// ----- GTK Signal Callbacks -----
void TextSection::onRadioChanged(GtkToggleButton* button, gpointer user_data) {
    (void)button;
    TextSection* section = static_cast<TextSection*>(user_data);
    section->updateLevelIndicator();
    if (section && section->manager_) section->manager_->notifyContentChanged();
}
void TextSection::onTypeRadioChanged(GtkToggleButton* button, gpointer user_data) {
    (void)button;
    TextSection* section = static_cast<TextSection*>(user_data);
    if (section && section->manager_) section->manager_->notifyContentChanged();
}
void TextSection::onDeleteClicked(GtkButton* button, gpointer user_data) {
    (void)button;
    TextSection* section = static_cast<TextSection*>(user_data);
    if (section && section->manager_) section->manager_->deleteSection(section);
}
void TextSection::onHeadlineChanged(GtkEditable* /*editable*/, gpointer user_data) {
    TextSection* section = static_cast<TextSection*>(user_data);
    if (section && section->manager_) section->manager_->notifyContentChanged();
}

// ----- UI Helpers -----
void TextSection::updateLevelIndicator() {
    int level = getHeadlineLevel();
    const char* markup = NULL;
    if (level == 1) markup = "<span size='small' weight='bold' foreground='#006400'>I</span>";
    else if (level == 2) markup = "<span size='small' weight='bold' foreground='#228B22'>II</span>";
    else if (level == 3) markup = "<span size='small' weight='bold' foreground='#32CD32'>III</span>";
    if (markup) gtk_label_set_markup(GTK_LABEL(order_level_label_), markup);
}
