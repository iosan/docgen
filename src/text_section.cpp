#include "text_section.h"
#include <gtk/gtk.h>
#include <cstring>

TextSection::TextSection(int position, const std::string& default_header)
    : position_(position), container_(nullptr), header_entry_(nullptr),
      header_label_(nullptr), text_view_(nullptr), scrolled_window_(nullptr),
      order_button_(nullptr), order_label_(nullptr) {
    createUI(default_header);
}

TextSection::~TextSection() {
    // GTK widgets are destroyed when their parent container is destroyed
    // No manual cleanup needed
}

void TextSection::createUI(const std::string& default_header) {
    // Create main container with modern styling
    container_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkStyleContext* container_context = gtk_widget_get_style_context(container_);
    gtk_style_context_add_class(container_context, "section-container");
    gtk_widget_set_margin_start(container_, 4);
    gtk_widget_set_margin_end(container_, 4);
    gtk_widget_set_margin_top(container_, 4);
    gtk_widget_set_margin_bottom(container_, 4);

    // Create header entry with modern styling
    header_entry_ = gtk_entry_new();
    GtkStyleContext* entry_context = gtk_widget_get_style_context(header_entry_);
    gtk_style_context_add_class(entry_context, "header-entry");
    gtk_entry_set_text(GTK_ENTRY(header_entry_), default_header.c_str());
    gtk_entry_set_placeholder_text(GTK_ENTRY(header_entry_), "Enter section title...");
    gtk_box_pack_start(GTK_BOX(container_), header_entry_, FALSE, FALSE, 0);

    // Create header label with modern styling
    header_label_ = gtk_label_new(default_header.c_str());
    gtk_widget_set_halign(header_label_, GTK_ALIGN_START);
    gtk_widget_set_margin_top(header_label_, 8);
    gtk_widget_set_margin_bottom(header_label_, 8);
    gtk_widget_set_margin_start(header_label_, 4);
    
    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_SEMIBOLD));
    pango_attr_list_insert(attrs, pango_attr_scale_new(1.3));
    pango_attr_list_insert(attrs, pango_attr_foreground_new(0x2000, 0x2000, 0x2000));
    gtk_label_set_attributes(GTK_LABEL(header_label_), attrs);
    pango_attr_list_unref(attrs);
    
    gtk_box_pack_start(GTK_BOX(container_), header_label_, FALSE, FALSE, 0);

    // Create scrolled window for text view with shadow
    scrolled_window_ = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window_), GTK_SHADOW_IN);
    gtk_widget_set_size_request(scrolled_window_, -1, 100);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window_), 100);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scrolled_window_), 400);
    gtk_widget_set_margin_top(scrolled_window_, 4);
    gtk_widget_set_margin_bottom(scrolled_window_, 4);

    // Create text view with monospace font
    text_view_ = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view_), 8);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view_), 8);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view_), 8);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view_), 8);
    gtk_container_add(GTK_CONTAINER(scrolled_window_), text_view_);

    gtk_box_pack_start(GTK_BOX(container_), scrolled_window_, FALSE, TRUE, 0);

    // Create order button with modern styling
    order_button_ = gtk_button_new();
    GtkStyleContext* button_context = gtk_widget_get_style_context(order_button_);
    gtk_style_context_add_class(button_context, "order-button");
    order_label_ = gtk_label_new(default_header.c_str());
    gtk_container_add(GTK_CONTAINER(order_button_), order_label_);
    gtk_widget_set_can_focus(order_button_, FALSE);

    // Store position data
    gint* pos_data = g_new(gint, 1);
    *pos_data = position_;
    g_object_set_data_full(G_OBJECT(order_button_), "position", pos_data, g_free);
    
    // Store pointer to this object for callbacks
    g_object_set_data(G_OBJECT(header_entry_), "text_section", this);

    // Connect header change callback
    g_signal_connect(header_entry_, "changed", G_CALLBACK(onHeaderChanged), this);
}

void TextSection::onHeaderChanged(GtkEntry* entry, gpointer user_data) {
    TextSection* section = static_cast<TextSection*>(user_data);
    const char* text = gtk_entry_get_text(entry);
    
    if (text && strlen(text) > 0) {
        section->setHeader(text);
    }
}

std::string TextSection::getHeader() const {
    const char* text = gtk_entry_get_text(GTK_ENTRY(header_entry_));
    return text ? std::string(text) : "";
}

void TextSection::setHeader(const std::string& header) {
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
    if (pos_data) {
        *pos_data = position;
    }
}

void TextSection::show() {
    gtk_widget_show_all(container_);
    gtk_widget_show_all(order_button_);
}

void TextSection::hide() {
    gtk_widget_hide(container_);
    gtk_widget_hide(order_button_);
}
