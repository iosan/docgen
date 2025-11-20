#include "section_manager.h"
#include "text_section.h"
#include <algorithm>
#include <fstream>
#include <string>

static GtkTargetEntry target_list[] = {
    { (gchar*)"GTK_LIST_BOX_ROW", GTK_TARGET_SAME_APP, 0 }
};

SectionManager::SectionManager(GtkWidget* text_container, GtkWidget* order_box)
    : text_container_(text_container), order_box_(order_box),
      section_counter_(0), main_section_(nullptr),
      main_order_button_(nullptr), main_text_view_(nullptr),
      dragged_widget_(nullptr), loaded_document_title_(""), dragged_source_index_(-1) {
    createMainSection();
    
    // Make order box a drop target for gaps between elements
    gtk_drag_dest_set(order_box_, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP), 
                      target_list, 1, GDK_ACTION_MOVE);
    g_signal_connect(order_box_, "drag-motion", G_CALLBACK(onOrderBoxDragMotion), this);
    g_signal_connect(order_box_, "drag-data-received", G_CALLBACK(onDragDataReceived), this);
    g_signal_connect(order_box_, "drag-leave", G_CALLBACK(onDragLeave), this);
}

SectionManager::~SectionManager() {
    sections_.clear();
}

void SectionManager::createMainSection() {
    // Create main section container
    main_section_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(main_section_, 10);
    gtk_widget_set_margin_end(main_section_, 10);
    gtk_widget_set_margin_top(main_section_, 5);
    gtk_widget_set_margin_bottom(main_section_, 5);

    // Main header entry
    GtkWidget* main_header_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(main_header_entry), "Main Section");
    gtk_entry_set_placeholder_text(GTK_ENTRY(main_header_entry), "Enter header...");
    gtk_box_pack_start(GTK_BOX(main_section_), main_header_entry, FALSE, FALSE, 0);

    // Main header display
    GtkWidget* main_header_display = gtk_label_new("Main Section");
    gtk_widget_set_halign(main_header_display, GTK_ALIGN_START);
    gtk_widget_set_margin_top(main_header_display, 5);
    gtk_widget_set_margin_bottom(main_header_display, 5);
    
    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(attrs, pango_attr_scale_new(1.2));
    gtk_label_set_attributes(GTK_LABEL(main_header_display), attrs);
    pango_attr_list_unref(attrs);
    
    gtk_box_pack_start(GTK_BOX(main_section_), main_header_display, FALSE, FALSE, 0);

    // Main scrolled window
    GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, -1, 100);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window), 100);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scrolled_window), 400);

    // Main text view
    main_text_view_ = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(main_text_view_), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(main_text_view_), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), main_text_view_);

    gtk_box_pack_start(GTK_BOX(main_section_), scrolled_window, FALSE, TRUE, 0);

    // Add separator
    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_section_), separator, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(text_container_), main_section_, FALSE, TRUE, 5);

    // Create main order button
    main_order_button_ = gtk_button_new();
    GtkWidget* order_label_main = gtk_label_new("Main Section");
    gtk_container_add(GTK_CONTAINER(main_order_button_), order_label_main);
    gtk_widget_set_can_focus(main_order_button_, FALSE);

    gint* main_position = g_new(gint, 1);
    *main_position = 0;
    g_object_set_data_full(G_OBJECT(main_order_button_), "position", main_position, g_free);
    
    // Store order label and display for header updates
    struct HeaderData {
        GtkLabel* header_display;
        GtkLabel* order_label;
    };
    HeaderData* header_data = g_new(HeaderData, 1);
    header_data->header_display = GTK_LABEL(main_header_display);
    header_data->order_label = GTK_LABEL(order_label_main);
    
    g_signal_connect(main_header_entry, "changed", G_CALLBACK(+[](GtkEntry* entry, gpointer data) {
        HeaderData* hd = (HeaderData*)data;
        const char* text = gtk_entry_get_text(entry);
        if (text && strlen(text) > 0) {
            gtk_label_set_text(hd->header_display, text);
            gtk_label_set_text(hd->order_label, text);
        }
    }), header_data);
    
    g_object_set_data_full(G_OBJECT(main_header_entry), "header_data", header_data, g_free);

    setupDragAndDrop(main_order_button_, 0);
    
    gtk_box_pack_start(GTK_BOX(order_box_), main_order_button_, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(order_box_), "main_order_item", main_order_button_);
    g_object_set_data(G_OBJECT(order_box_), "section_manager", this);

    // Hide initially and prevent show_all from showing them
    gtk_widget_set_no_show_all(main_section_, TRUE);
    gtk_widget_set_no_show_all(main_order_button_, TRUE);
    gtk_widget_hide(main_section_);
    gtk_widget_hide(main_order_button_);
}

void SectionManager::setupDragAndDrop(GtkWidget* order_button, int position) {
    (void)position;  // Currently unused but kept for future extensibility
    
    gtk_drag_source_set(order_button, GDK_BUTTON1_MASK, target_list, 1, GDK_ACTION_MOVE);
    gtk_drag_dest_set(order_button, 
                      (GtkDestDefaults)(GTK_DEST_DEFAULT_ALL), 
                      target_list, 1, GDK_ACTION_MOVE);
    
    g_signal_connect(order_button, "drag-begin", G_CALLBACK(onDragBegin), this);
    g_signal_connect(order_button, "drag-end", G_CALLBACK(onDragEnd), this);
    g_signal_connect(order_button, "drag-data-get", G_CALLBACK(onDragDataGet), this);
    g_signal_connect(order_button, "drag-data-received", G_CALLBACK(onDragDataReceived), this);
    g_signal_connect(order_button, "drag-motion", G_CALLBACK(onDragMotion), this);
    g_signal_connect(order_button, "drag-leave", G_CALLBACK(onDragLeave), this);
}

void SectionManager::addSection(const std::string& header, const std::string& content) {
    section_counter_++;
    
    auto section = std::make_unique<TextSection>(section_counter_, header);
    section->setManager(this);
    
    if (!content.empty()) {
        section->setContent(content);
    }
    
    // Add to containers
    gtk_box_pack_start(GTK_BOX(text_container_), section->getContainer(), FALSE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(order_box_), section->getOrderButton(), FALSE, FALSE, 0);
    
    // Setup drag and drop
    setupDragAndDrop(section->getOrderButton(), section_counter_);
    
    section->show();
    sections_.push_back(std::move(section));
}

void SectionManager::deleteSection(TextSection* section) {
    if (!section) return;
    
    // Find the section in our vector
    auto it = std::find_if(sections_.begin(), sections_.end(),
                          [section](const std::unique_ptr<TextSection>& ptr) {
                              return ptr.get() == section;
                          });
    
    if (it != sections_.end()) {
        // Remove widgets from containers
        GtkWidget* container = (*it)->getContainer();
        GtkWidget* order_button = (*it)->getOrderButton();
        
        if (container && gtk_widget_get_parent(container)) {
            gtk_container_remove(GTK_CONTAINER(text_container_), container);
        }
        if (order_button && gtk_widget_get_parent(order_button)) {
            gtk_container_remove(GTK_CONTAINER(order_box_), order_button);
        }
        
        // Remove from vector (this will destroy the unique_ptr and the object)
        sections_.erase(it);
    }
}

void SectionManager::clearAll() {
    // Clear main section
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(main_text_view_));
    gtk_text_buffer_set_text(buffer, "", -1);
    
    hideMainSection();
    
    // Remove all section widgets from containers before destroying objects
    for (auto& section : sections_) {
        GtkWidget* container = section->getContainer();
        GtkWidget* order_button = section->getOrderButton();
        
        if (container && gtk_widget_get_parent(container)) {
            gtk_container_remove(GTK_CONTAINER(text_container_), container);
        }
        if (order_button && gtk_widget_get_parent(order_button)) {
            gtk_container_remove(GTK_CONTAINER(order_box_), order_button);
        }
    }
    
    // Remove all sections
    sections_.clear();
    
    // Reset counter
    section_counter_ = 0;
}

void SectionManager::showMainSection() {
    gtk_widget_show_all(main_section_);
    gtk_widget_show_all(main_order_button_);
}

void SectionManager::hideMainSection() {
    gtk_widget_hide(main_section_);
    gtk_widget_hide(main_order_button_);
}



std::vector<std::pair<std::string, std::string>> SectionManager::getSectionsInOrder() const {
    std::vector<std::pair<std::string, std::string>> result;
    
    // Get sections in current display order
    GList* text_children = gtk_container_get_children(GTK_CONTAINER(text_container_));
    
    for (GList* l = text_children; l != NULL; l = l->next) {
        GtkWidget* section_widget = GTK_WIDGET(l->data);
        
        // Skip main section
        if (section_widget == main_section_) {
            continue;
        }
        
        // Find corresponding TextSection object
        for (const auto& section : sections_) {
            if (section->getContainer() == section_widget) {
                result.push_back({section->getHeader(), ""});
                break;
            }
        }
    }
    
    g_list_free(text_children);
    return result;
}

std::string SectionManager::generateAsciiDoc(const std::string& title) const {
    std::string result;
    
    // Add document title if provided
    if (!title.empty()) {
        result = "= " + title + "\n\n";
    }
    
    // Get sections in current order
    GList* text_children = gtk_container_get_children(GTK_CONTAINER(text_container_));
    
    for (GList* l = text_children; l != NULL; l = l->next) {
        GtkWidget* section_widget = GTK_WIDGET(l->data);
        
        // Skip main section
        if (section_widget == main_section_) {
            continue;
        }
        
        // Find corresponding TextSection object
        for (const auto& section : sections_) {
            if (section->getContainer() == section_widget) {
                std::string header = section->getHeader();
                std::string headline = section->getHeadline();
                int level = section->getHeadlineLevel();
                
                // Use headline if provided, otherwise use default
                std::string section_title = headline.empty() ? "section headline" : headline;
                
                // Generate AsciiDoc heading based on level
                std::string heading_marker;
                switch (level) {
                    case 1:
                        heading_marker = "== ";  // Level 1 heading
                        break;
                    case 2:
                        heading_marker = "=== ";  // Level 2 heading
                        break;
                    case 3:
                        heading_marker = "==== ";  // Level 3 heading
                        break;
                    default:
                        heading_marker = "=== ";  // Default to level 2
                }
                
                // Navigate widget tree to find text view
                GList* section_children = gtk_container_get_children(GTK_CONTAINER(section->getContainer()));
                GtkWidget* scrolled = nullptr;
                for (GList* sc = section_children; sc != NULL; sc = sc->next) {
                    if (GTK_IS_SCROLLED_WINDOW(sc->data)) {
                        scrolled = GTK_WIDGET(sc->data);
                        break;
                    }
                }
                g_list_free(section_children);
                
                if (scrolled) {
                    GtkWidget* text_view_widget = gtk_bin_get_child(GTK_BIN(scrolled));
                    if (GTK_IS_TEXT_VIEW(text_view_widget)) {
                        GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_widget));
                        GtkTextIter start, end;
                        gtk_text_buffer_get_bounds(buffer, &start, &end);
                        gchar* content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
                        
                        // Write AsciiDoc section with custom headline and level
                        result += heading_marker + section_title + "\n\n";
                        result += std::string(content) + "\n\n";
                        
                        g_free(content);
                    }
                }
                break;
            }
        }
    }
    
    g_list_free(text_children);
    return result;
}

bool SectionManager::saveToFile(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    // Save document title (stored separately by main window, so we don't save it here)
    // The main window will need to handle this
    
    // Get sections in current order
    GList* text_children = gtk_container_get_children(GTK_CONTAINER(text_container_));
    
    for (GList* l = text_children; l != NULL; l = l->next) {
        GtkWidget* section_widget = GTK_WIDGET(l->data);
        
        // Skip main section
        if (section_widget == main_section_) {
            continue;
        }
        
        // Find corresponding TextSection object
        for (const auto& section : sections_) {
            if (section->getContainer() == section_widget) {
                std::string header = section->getHeader();
                
                // Navigate widget tree to find text view
                GList* section_children = gtk_container_get_children(GTK_CONTAINER(section->getContainer()));
                GtkWidget* scrolled = nullptr;
                for (GList* sc = section_children; sc != NULL; sc = sc->next) {
                    if (GTK_IS_SCROLLED_WINDOW(sc->data)) {
                        scrolled = GTK_WIDGET(sc->data);
                        break;
                    }
                }
                g_list_free(section_children);
                
                if (scrolled) {
                    GtkWidget* text_view_widget = gtk_bin_get_child(GTK_BIN(scrolled));
                    if (GTK_IS_TEXT_VIEW(text_view_widget)) {
                        GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_widget));
                        GtkTextIter start, end;
                        gtk_text_buffer_get_bounds(buffer, &start, &end);
                        gchar* content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
                        
                        std::string headline = section->getHeadline();
                        int level = section->getHeadlineLevel();
                        
                        // Write section header
                        file << "[SECTION:" << header << "]\n";
                        file << "[HEADLINE:" << headline << "]\n";
                        file << "[LEVEL:" << level << "]\n";
                        // Write content
                        file << content << "\n";
                        file << "[END_SECTION]\n\n";
                        
                        g_free(content);
                    }
                }
                break;
            }
        }
    }
    
    g_list_free(text_children);
    file.close();
    return true;
}

bool SectionManager::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    // Clear existing sections
    clearAll();
    
    std::string line;
    std::string current_header;
    std::string current_headline;
    int current_level = 2;
    std::string current_content;
    std::string document_title;
    bool in_section = false;
    
    while (std::getline(file, line)) {
        if (line.find("[DOCUMENT_TITLE:") == 0) {
            // Extract document title (will be handled by main window)
            size_t start = line.find(":") + 1;
            size_t end = line.find("]");
            document_title = line.substr(start, end - start);
        } else if (line.find("[SECTION:") == 0) {
            // Extract header
            size_t start = line.find(":") + 1;
            size_t end = line.find("]");
            current_header = line.substr(start, end - start);
            current_content.clear();
            current_headline.clear();
            current_level = 2;
            in_section = true;
        } else if (line.find("[HEADLINE:") == 0) {
            // Extract headline
            size_t start = line.find(":") + 1;
            size_t end = line.find("]");
            current_headline = line.substr(start, end - start);
        } else if (line.find("[LEVEL:") == 0) {
            // Extract level
            size_t start = line.find(":") + 1;
            size_t end = line.find("]");
            current_level = std::stoi(line.substr(start, end - start));
        } else if (line == "[END_SECTION]") {
            if (in_section && !current_header.empty()) {
                // Remove trailing newline if present
                if (!current_content.empty() && current_content.back() == '\n') {
                    current_content.pop_back();
                }
                addSection(current_header, current_content);
                // Set headline and level for the last added section
                if (!sections_.empty()) {
                    sections_.back()->setHeadline(current_headline);
                    sections_.back()->setHeadlineLevel(current_level);
                }
            }
            in_section = false;
            current_header.clear();
            current_headline.clear();
            current_level = 2;
            current_content.clear();
        } else if (in_section) {
            current_content += line + "\n";
        }
    }
    
    file.close();
    loaded_document_title_ = document_title;
    return true;
}

void SectionManager::setMainSectionContent(const std::string& content) {
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(main_text_view_));
    gtk_text_buffer_set_text(buffer, content.c_str(), -1);
    showMainSection();
}

// Static callback implementations
void SectionManager::onDragBegin(GtkWidget* widget, GdkDragContext* context, gpointer user_data) {
    (void)context;
    SectionManager* manager = (SectionManager*)user_data;
    
    // Store the dragged widget and find its index
    manager->dragged_widget_ = widget;
    
    GList* children = gtk_container_get_children(GTK_CONTAINER(manager->order_box_));
    gint index = 0;
    for (GList* l = children; l != NULL; l = l->next) {
        if (GTK_WIDGET(l->data) == widget) {
            manager->dragged_source_index_ = index;
            break;
        }
        index++;
    }
    g_list_free(children);
    
    // Make dragged element semi-transparent
    gtk_widget_set_opacity(widget, 0.3);
}

void SectionManager::onDragDataGet(GtkWidget* widget, GdkDragContext* context,
                                   GtkSelectionData* selection_data, guint info,
                                   guint time, gpointer user_data) {
    (void)context;
    (void)info;
    (void)time;
    (void)user_data;
    
    gint* position = (gint*)g_object_get_data(G_OBJECT(widget), "position");
    if (position) {
        gtk_selection_data_set(selection_data, gtk_selection_data_get_target(selection_data),
                              32, (const guchar*)position, sizeof(gint));
    }
}

void SectionManager::onDragDataReceived(GtkWidget* widget, GdkDragContext* context,
                                        gint x, gint y, GtkSelectionData* selection_data,
                                        guint info, guint time, gpointer user_data) {
    (void)widget;
    (void)x;
    (void)y;
    (void)selection_data;
    (void)info;
    (void)user_data;
    
    gtk_drag_finish(context, TRUE, FALSE, time);
}

gboolean SectionManager::onDragMotion(GtkWidget* widget, GdkDragContext* context,
                                      gint x, gint y, guint time, gpointer user_data) {
    (void)y;
    
    SectionManager* manager = (SectionManager*)user_data;
    
    // Don't process on the dragged widget itself
    if (widget == manager->dragged_widget_) {
        gdk_drag_status(context, GDK_ACTION_MOVE, time);
        return TRUE;
    }
    
    // Get hovered widget position
    GtkAllocation hover_alloc;
    gtk_widget_get_allocation(widget, &hover_alloc);
    
    // Determine if center of drag is over left or right half
    gboolean insert_after = (x >= hover_alloc.width / 2);
    
    // Find the position of the hovered widget
    GList* children = gtk_container_get_children(GTK_CONTAINER(manager->order_box_));
    gint target_pos = -1;
    gint current_pos = 0;
    
    for (GList* l = children; l != NULL; l = l->next) {
        GtkWidget* child = GTK_WIDGET(l->data);
        if (child == widget) {
            target_pos = current_pos;
            break;
        }
        current_pos++;
    }
    g_list_free(children);
    
    if (target_pos >= 0) {
        // Calculate final position
        gint new_pos = insert_after ? target_pos + 1 : target_pos;
        
        // Adjust if we're moving from before to after the target
        if (manager->dragged_source_index_ >= 0 && manager->dragged_source_index_ < new_pos) {
            new_pos--;
        }
        
        // Reorder the dragged widget
        gtk_box_reorder_child(GTK_BOX(manager->order_box_), manager->dragged_widget_, new_pos);
        manager->dragged_source_index_ = new_pos;
    }
    
    gdk_drag_status(context, GDK_ACTION_MOVE, time);
    return TRUE;
}

void SectionManager::onDragLeave(GtkWidget* widget, GdkDragContext* context,
                                 guint time, gpointer user_data) {
    (void)widget;
    (void)context;
    (void)time;
    (void)user_data;
}

void SectionManager::onDragEnd(GtkWidget* widget, GdkDragContext* context, gpointer user_data) {
    (void)context;
    
    SectionManager* manager = (SectionManager*)user_data;
    
    // Restore widget opacity
    gtk_widget_set_opacity(widget, 1.0);
    
    // Sync text sections to match order box arrangement
    GList* order_children = gtk_container_get_children(GTK_CONTAINER(manager->order_box_));
    GList* text_children = gtk_container_get_children(GTK_CONTAINER(manager->text_container_));
    
    gint position = 0;
    for (GList* order_l = order_children; order_l != NULL; order_l = order_l->next) {
        GtkWidget* order_button = GTK_WIDGET(order_l->data);
        
        // Find matching text section
        for (GList* text_l = text_children; text_l != NULL; text_l = text_l->next) {
            GtkWidget* text_section = GTK_WIDGET(text_l->data);
            
            // Check if this text section's order button matches
            for (size_t i = 0; i < manager->sections_.size(); ++i) {
                if (manager->sections_[i]->getOrderButton() == order_button &&
                    manager->sections_[i]->getContainer() == text_section) {
                    gtk_box_reorder_child(GTK_BOX(manager->text_container_), text_section, position);
                    break;
                }
            }
        }
        position++;
    }
    
    g_list_free(order_children);
    g_list_free(text_children);
    
    manager->dragged_widget_ = nullptr;
    manager->dragged_source_index_ = -1;
}

gboolean SectionManager::onOrderBoxDragMotion(GtkWidget* widget, GdkDragContext* context,
                                              gint x, gint y, guint time, gpointer user_data) {
    (void)widget;
    (void)x;
    
    SectionManager* manager = (SectionManager*)user_data;
    
    if (!manager->dragged_widget_) {
        gdk_drag_status(context, GDK_ACTION_MOVE, time);
        return TRUE;
    }
    
    // Get all children to find position based on y coordinate
    GList* children = gtk_container_get_children(GTK_CONTAINER(manager->order_box_));
    gint target_pos = 0;
    gboolean found = FALSE;
    
    for (GList* l = children; l != NULL; l = l->next) {
        GtkWidget* child = GTK_WIDGET(l->data);
        
        // Skip the dragged widget itself
        if (child == manager->dragged_widget_) {
            continue;
        }
        
        GtkAllocation alloc;
        gtk_widget_get_allocation(child, &alloc);
        
        // If y is in upper half, insert before this element
        if (y < alloc.y + alloc.height / 2) {
            found = TRUE;
            break;
        }
        
        target_pos++;
    }
    
    gint total_count = g_list_length(children);
    g_list_free(children);
    
    // If not found and we have items, position at end
    if (!found && total_count > 1) {
        target_pos = total_count - 1;
    }
    
    // Reorder the dragged widget
    gtk_box_reorder_child(GTK_BOX(manager->order_box_), manager->dragged_widget_, target_pos);
    manager->dragged_source_index_ = target_pos;
    
    gdk_drag_status(context, GDK_ACTION_MOVE, time);
    return TRUE;
}
