#include "main_window.h"
#include <iostream>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(GtkApplication* app)
    : window_(nullptr), main_vbox_(nullptr), section_manager_(nullptr),
      document_title_entry_(nullptr), preview_web_view_(nullptr),
      has_unsaved_changes_(false), current_set_file_("") {
    
    window_ = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window_), "Text File Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window_), 800, 600);
    
    // Set minimum window size
    GdkGeometry hints;
    hints.min_width = 600;
    hints.min_height = 400;
    gtk_window_set_geometry_hints(GTK_WINDOW(window_), NULL, &hints, GDK_HINT_MIN_SIZE);

    main_vbox_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window_), main_vbox_);

    createMenuBar();
    createUI();
}

MainWindow::~MainWindow() {
    // GTK handles widget cleanup
}

void MainWindow::updateTitle() {
    std::string title = "Text File Viewer";
    if (!current_set_file_.empty()) {
        // Extract just the filename from the path
        size_t last_slash = current_set_file_.find_last_of("/\\");
        std::string filename = (last_slash != std::string::npos) 
            ? current_set_file_.substr(last_slash + 1) 
            : current_set_file_;
        title += " [" + filename + "]";
    }
    gtk_window_set_title(GTK_WINDOW(window_), title.c_str());
}

void MainWindow::createMenuBar() {
    GtkWidget* menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(main_vbox_), menu_bar, FALSE, FALSE, 0);

    // File menu
    GtkWidget* file_menu = gtk_menu_new();
    GtkWidget* file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);

    GtkWidget* save_set_item = gtk_menu_item_new_with_label("Save Set...");
    g_signal_connect(save_set_item, "activate", G_CALLBACK(onSaveSet), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_set_item);

    GtkWidget* open_set_item = gtk_menu_item_new_with_label("Open Set...");
    g_signal_connect(open_set_item, "activate", G_CALLBACK(onOpenSet), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_set_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());

    GtkWidget* quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(quit_item, "activate", G_CALLBACK(onQuit), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);

    // Edit menu
    GtkWidget* edit_menu = gtk_menu_new();
    GtkWidget* edit_item = gtk_menu_item_new_with_label("Edit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit_item);

    GtkWidget* add_item = gtk_menu_item_new_with_label("Add Text Section");
    g_signal_connect(add_item, "activate", G_CALLBACK(onAddSection), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), add_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());

    GtkWidget* create_doc_item = gtk_menu_item_new_with_label("Create AsciiDoc...");
    g_signal_connect(create_doc_item, "activate", G_CALLBACK(onCreateDoc), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), create_doc_item);

    GtkWidget* create_md_item = gtk_menu_item_new_with_label("Create Markdown...");
    g_signal_connect(create_md_item, "activate", G_CALLBACK(onCreateMarkdown), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), create_md_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), gtk_separator_menu_item_new());

    GtkWidget* clear_item = gtk_menu_item_new_with_label("Clear All");
    g_signal_connect(clear_item, "activate", G_CALLBACK(onClearAll), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), clear_item);

    // About menu
    GtkWidget* about_menu = gtk_menu_new();
    GtkWidget* about_menu_item = gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(about_menu_item), about_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), about_menu_item);

    GtkWidget* about_item = gtk_menu_item_new_with_label("About Text Viewer");
    g_signal_connect(about_item, "activate", G_CALLBACK(onAbout), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(about_menu), about_item);
    
    // Plus icon menu item (add text section)
    GtkWidget* add_icon_item = gtk_menu_item_new();
    GtkWidget* add_icon = gtk_image_new_from_icon_name("list-add", GTK_ICON_SIZE_MENU);
    gtk_container_add(GTK_CONTAINER(add_icon_item), add_icon);
    g_signal_connect(add_icon_item, "button-press-event", G_CALLBACK(+[](GtkWidget*, GdkEventButton*, gpointer data) -> gboolean {
        onAddSection(nullptr, data);
        return TRUE;
    }), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), add_icon_item);
    
    // Trash icon menu item (clear all)
    GtkWidget* clear_icon_item = gtk_menu_item_new();
    GtkWidget* clear_icon = gtk_image_new_from_icon_name("user-trash", GTK_ICON_SIZE_MENU);
    gtk_container_add(GTK_CONTAINER(clear_icon_item), clear_icon);
    g_signal_connect(clear_icon_item, "button-press-event", G_CALLBACK(+[](GtkWidget*, GdkEventButton*, gpointer data) -> gboolean {
        onClearAll(nullptr, data);
        return TRUE;
    }), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), clear_icon_item);
}

void MainWindow::createUI() {
    // Apply CSS for modern styling
    GtkCssProvider* css_provider = gtk_css_provider_new();
    const gchar* css_data = 
        "window { background-color: #f5f5f5; }"
        "menuitem { "
        "  box-shadow: none; "
        "  -gtk-icon-shadow: none; "
        "}"
        "menuitem:hover { "
        "  box-shadow: none; "
        "}"
        ".section-container { "
        "  background-color: white; "
        "  border-radius: 8px; "
        "  box-shadow: 0 2px 4px rgba(0,0,0,0.1); "
        "  margin: 6px; "
        "  padding: 8px; "
        "}"
        ".order-frame { "
        "  background-color: #ffffff; "
        "  border-radius: 6px; "
        "  border: 1px solid #e0e0e0; "
        "}"
        ".order-button { "
        "  border-radius: 4px; "
        "  padding: 6px 12px; "
        "  margin: 4px; "
        "  min-height: 24px; "
        "  background: linear-gradient(to bottom, #f8f8f8, #e8e8e8); "
        "  border: 1px solid #c0c0c0; "
        "  transition: margin 120ms ease-out, background 100ms ease-out; "
        "}"
        ".order-button:hover { "
        "  background: linear-gradient(to bottom, #e8e8e8, #d8d8d8); "
        "}"
        ".drag-placeholder { "
        "  background: linear-gradient(90deg, "
        "    transparent, "
        "    rgba(74, 144, 226, 0.3) 20%, "
        "    rgba(74, 144, 226, 0.5) 50%, "
        "    rgba(74, 144, 226, 0.3) 80%, "
        "    transparent); "
        "  border: 2px dashed #4a90e2; "
        "  border-radius: 4px; "
        "  margin: 4px; "
        "  padding: 6px 60px; "
        "}"
        ".header-entry { "
        "  border-radius: 4px; "
        "  padding: 8px; "
        "  margin-bottom: 8px; "
        "}"
        "entry { "
        "  font-size: 0.85em; "
        "  padding: 4px 6px; "
        "  min-height: 24px; "
        "}"
        "frame > label { "
        "  font-size: 0.85em; "
        "}"
        "frame entry { "
        "  font-size: 0.85em; "
        "}"
        "radiobutton { "
        "  font-size: 0.8em; "
        "  padding: 2px; "
        "  min-height: 18px; "
        "}"
        "radiobutton label { "
        "  font-size: 0.8em; "
        "}"
        "textview { "
        "  border-radius: 4px; "
        "  padding: 4px; "
        "  font-family: monospace; "
        "  font-size: 0.85em; "
        "}"
        "#preview-frame textview { "
        "  font-size: 0.3em; "
        "}";
    
    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                               GTK_STYLE_PROVIDER(css_provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider);
    
    // Order box for draggable section labels with modern styling
    GtkWidget* order_frame = gtk_frame_new(NULL);
    gtk_widget_set_name(order_frame, "order-frame");
    GtkStyleContext* frame_context = gtk_widget_get_style_context(order_frame);
    gtk_style_context_add_class(frame_context, "order-frame");
    
    // Add label with padding
    GtkWidget* order_label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* order_title = gtk_label_new("Section Order");
    PangoAttrList* title_attrs = pango_attr_list_new();
    pango_attr_list_insert(title_attrs, pango_attr_weight_new(PANGO_WEIGHT_SEMIBOLD));
    pango_attr_list_insert(title_attrs, pango_attr_scale_new(0.9));
    gtk_label_set_attributes(GTK_LABEL(order_title), title_attrs);
    pango_attr_list_unref(title_attrs);
    gtk_widget_set_halign(order_title, GTK_ALIGN_START);
    gtk_widget_set_margin_start(order_title, 12);
    gtk_widget_set_margin_end(order_title, 12);
    gtk_widget_set_margin_top(order_title, 8);
    gtk_widget_set_margin_bottom(order_title, 4);
    gtk_box_pack_start(GTK_BOX(order_label_box), order_title, FALSE, FALSE, 0);
    
    GtkWidget* order_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(order_scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_NEVER);
    gtk_widget_set_size_request(order_scrolled, -1, 56);
    gtk_widget_set_name(order_scrolled, "order-scrolled");
    
    // Add watermark background to order scrolled window using embedded GResource
    GtkCssProvider* watermark_provider = gtk_css_provider_new();
    
    // Use resource:// URI to reference embedded image
    const gchar* css_with_watermark = 
        "#order-scrolled { "
        "  background-image: url('resource:///org/docgen/watermark_order.png'); "
        "  background-repeat: repeat-x; "
        "  background-position: left center; "
        "  background-size: auto; "
        "  background-color: rgba(255, 255, 255, 0.9); "
        "}"
        "#order-scrolled * { "
        "  background-color: transparent; "
        "}"
        "#order-scrolled viewport { "
        "  background: linear-gradient(rgba(128, 128, 128, 0.15), rgba(128, 128, 128, 0.15)); "
        "}";
    
    gtk_css_provider_load_from_data(watermark_provider,
        css_with_watermark,
        -1, NULL);
    GtkStyleContext* order_scrolled_context = gtk_widget_get_style_context(order_scrolled);
    gtk_style_context_add_provider(order_scrolled_context, 
                                    GTK_STYLE_PROVIDER(watermark_provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(watermark_provider);
    
    GtkWidget* order_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(order_box, 12);
    gtk_widget_set_margin_end(order_box, 12);
    gtk_widget_set_margin_top(order_box, 8);
    gtk_widget_set_margin_bottom(order_box, 8);
    gtk_container_add(GTK_CONTAINER(order_scrolled), order_box);
    gtk_box_pack_start(GTK_BOX(order_label_box), order_scrolled, TRUE, TRUE, 0);
    
    // Document title input box at bottom
    GtkWidget* title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(title_box, 12);
    gtk_widget_set_margin_end(title_box, 12);
    gtk_widget_set_margin_top(title_box, 8);
    gtk_widget_set_margin_bottom(title_box, 8);
    
    GtkWidget* title_label = gtk_label_new("Main Document Title:");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(title_box), title_label, FALSE, FALSE, 0);
    
    document_title_entry_ = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(document_title_entry_), "Enter main document title...");
    gtk_widget_set_hexpand(document_title_entry_, TRUE);
    gtk_box_pack_start(GTK_BOX(title_box), document_title_entry_, TRUE, TRUE, 0);
    
    gtk_box_pack_end(GTK_BOX(order_label_box), title_box, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(order_frame), order_label_box);
    gtk_widget_set_margin_start(order_frame, 12);
    gtk_widget_set_margin_end(order_frame, 6);
    gtk_widget_set_margin_top(order_frame, 12);
    gtk_widget_set_margin_bottom(order_frame, 8);
    
    // Create horizontal box to hold order frame and preview frame side by side
    GtkWidget* order_preview_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(order_preview_hbox), order_frame, TRUE, TRUE, 0);
    
    // Preview frame - square aspect ratio
    GtkWidget* preview_frame = gtk_frame_new(NULL);
    gtk_widget_set_name(preview_frame, "preview-frame");
    GtkStyleContext* preview_frame_context = gtk_widget_get_style_context(preview_frame);
    gtk_style_context_add_class(preview_frame_context, "order-frame");
    
    GtkWidget* preview_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* preview_title = gtk_label_new("Preview");
    PangoAttrList* preview_title_attrs = pango_attr_list_new();
    pango_attr_list_insert(preview_title_attrs, pango_attr_weight_new(PANGO_WEIGHT_SEMIBOLD));
    pango_attr_list_insert(preview_title_attrs, pango_attr_scale_new(0.9));
    gtk_label_set_attributes(GTK_LABEL(preview_title), preview_title_attrs);
    pango_attr_list_unref(preview_title_attrs);
    gtk_widget_set_halign(preview_title, GTK_ALIGN_START);
    gtk_widget_set_margin_start(preview_title, 12);
    gtk_widget_set_margin_end(preview_title, 12);
    gtk_widget_set_margin_top(preview_title, 8);
    gtk_widget_set_margin_bottom(preview_title, 4);
    gtk_box_pack_start(GTK_BOX(preview_vbox), preview_title, FALSE, FALSE, 0);
    
    // Preview scrolled window - calculate square size based on available height
    GtkWidget* preview_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(preview_scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    // Set width equal to height (approx 140px for the content area + margins/title)
    gtk_widget_set_size_request(preview_scrolled, 140, 140);
    gtk_widget_set_margin_start(preview_scrolled, 12);
    gtk_widget_set_margin_end(preview_scrolled, 12);
    gtk_widget_set_margin_bottom(preview_scrolled, 8);
    
    // Preview web view for HTML rendering
    preview_web_view_ = WEBKIT_WEB_VIEW(webkit_web_view_new());
    webkit_web_view_set_editable(preview_web_view_, FALSE);
    gtk_container_add(GTK_CONTAINER(preview_scrolled), GTK_WIDGET(preview_web_view_));
    
    gtk_box_pack_start(GTK_BOX(preview_vbox), preview_scrolled, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(preview_frame), preview_vbox);
    gtk_widget_set_margin_start(preview_frame, 6);
    gtk_widget_set_margin_end(preview_frame, 12);
    gtk_widget_set_margin_top(preview_frame, 12);
    gtk_widget_set_margin_bottom(preview_frame, 8);
    gtk_widget_set_size_request(preview_frame, 300, -1);
    
    gtk_box_pack_start(GTK_BOX(order_preview_hbox), preview_frame, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox_), order_preview_hbox, FALSE, FALSE, 0);
    
    // Connect signal to update preview when document title changes
    g_signal_connect(document_title_entry_, "changed", G_CALLBACK(+[](GtkEntry*, gpointer data) {
        MainWindow* window = static_cast<MainWindow*>(data);
        window->updatePreview();
    }), this);

    // Frame for text sections
    GtkWidget* text_sections_frame = gtk_frame_new(NULL);
    GtkStyleContext* sections_frame_context = gtk_widget_get_style_context(text_sections_frame);
    gtk_style_context_add_class(sections_frame_context, "order-frame");
    
    // Vertical box to hold title label and scrolled window
    GtkWidget* sections_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Text Sections title label
    GtkWidget* sections_title = gtk_label_new("Text Sections");
    PangoAttrList* sections_title_attrs = pango_attr_list_new();
    pango_attr_list_insert(sections_title_attrs, pango_attr_weight_new(PANGO_WEIGHT_SEMIBOLD));
    pango_attr_list_insert(sections_title_attrs, pango_attr_scale_new(0.9));
    gtk_label_set_attributes(GTK_LABEL(sections_title), sections_title_attrs);
    pango_attr_list_unref(sections_title_attrs);
    gtk_widget_set_halign(sections_title, GTK_ALIGN_START);
    gtk_widget_set_margin_start(sections_title, 12);
    gtk_widget_set_margin_end(sections_title, 12);
    gtk_widget_set_margin_top(sections_title, 8);
    gtk_widget_set_margin_bottom(sections_title, 4);
    gtk_box_pack_start(GTK_BOX(sections_vbox), sections_title, FALSE, FALSE, 0);

    // Main scrolled window to contain all text views
    GtkWidget* main_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(main_scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_margin_start(main_scrolled, 12);
    gtk_widget_set_margin_end(main_scrolled, 12);
    gtk_widget_set_margin_bottom(main_scrolled, 8);
    gtk_box_pack_start(GTK_BOX(sections_vbox), main_scrolled, TRUE, TRUE, 0);
    
    // Container for all text views
    GtkWidget* text_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_add(GTK_CONTAINER(main_scrolled), text_container);
    
    gtk_container_add(GTK_CONTAINER(text_sections_frame), sections_vbox);
    gtk_widget_set_margin_start(text_sections_frame, 12);
    gtk_widget_set_margin_end(text_sections_frame, 12);
    gtk_widget_set_margin_bottom(text_sections_frame, 12);
    gtk_box_pack_start(GTK_BOX(main_vbox_), text_sections_frame, TRUE, TRUE, 0);

    // Create section manager
    section_manager_ = std::make_unique<SectionManager>(text_container, order_box);
    
    // Set callback to update preview when content changes
    section_manager_->setOnContentChangedCallback([this]() {
        updatePreview();
    });
}

void MainWindow::show() {
    gtk_widget_show_all(window_);
}

std::string MainWindow::getDocumentTitle() const {
    const char* text = gtk_entry_get_text(GTK_ENTRY(document_title_entry_));
    std::string title = text ? std::string(text) : "";
    return title.empty() ? "Default title" : title;
}

// Static callback implementations
void MainWindow::onAddSection(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Add Text File",
                                                     window->getWindow(),
                                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                                     "_Add", GTK_RESPONSE_ACCEPT,
                                                     NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char* basename = g_path_get_basename(filename);

        try {
            std::string content = window->text_viewer_.loadFile(filename);
            std::string header = basename;
            window->section_manager_->addSection(header, content);
            window->has_unsaved_changes_ = true;
            window->updatePreview();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            GtkWidget* error_dialog = gtk_message_dialog_new(window->getWindow(),
                                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_CLOSE,
                                                             "Error loading file: %s",
                                                             e.what());
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }

        g_free(basename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void MainWindow::onCreateDoc(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    
    // First, ensure the set is saved
    if (window->current_set_file_.empty() || window->has_unsaved_changes_) {
        // Prompt to save the set first
        GtkWidget* dialog = gtk_file_chooser_dialog_new("Save Section Set",
                                                         window->getWindow(),
                                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                                         "_Save", GTK_RESPONSE_ACCEPT,
                                                         NULL);
        
        if (!window->current_set_file_.empty()) {
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), window->current_set_file_.c_str());
        }
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gchar* filename = NULL;
        
        if (response == GTK_RESPONSE_ACCEPT) {
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        }
        
        // Destroy dialog immediately after getting response
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_ACCEPT && filename) {
            if (window->section_manager_->saveToFile(filename)) {
                window->current_set_file_ = filename;
                window->has_unsaved_changes_ = false;
                window->updateTitle();
            } else {
                g_free(filename);
                
                GtkWidget* error_dialog = gtk_message_dialog_new(window->getWindow(),
                                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                 GTK_MESSAGE_ERROR,
                                                                 GTK_BUTTONS_OK,
                                                                 "Failed to save section set.");
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
                return;
            }
            
            g_free(filename);
        } else {
            return; // User cancelled
        }
    }
    
    // Now create the AsciiDoc file
    GtkWidget* doc_dialog = gtk_file_chooser_dialog_new("Create AsciiDoc",
                                                         window->getWindow(),
                                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                                         "_Save", GTK_RESPONSE_ACCEPT,
                                                         NULL);
    
    // Add file filter for .adoc files
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "AsciiDoc files (*.adoc)");
    gtk_file_filter_add_pattern(filter, "*.adoc");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(doc_dialog), filter);
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(doc_dialog), TRUE);
    
    // Set default filename based on set filename, replacing .docgenset with .adoc
    if (!window->current_set_file_.empty()) {
        // Extract directory and filename
        size_t last_slash = window->current_set_file_.find_last_of("/\\");
        std::string directory;
        std::string filename;
        
        if (last_slash != std::string::npos) {
            directory = window->current_set_file_.substr(0, last_slash);
            filename = window->current_set_file_.substr(last_slash + 1);
        } else {
            filename = window->current_set_file_;
        }
        
        // Replace .docgenset extension with .adoc, or add .adoc if not present
        if (filename.length() >= 10 && 
            filename.substr(filename.length() - 10) == ".docgenset") {
            filename = filename.substr(0, filename.length() - 10) + ".adoc";
        } else if (filename.length() < 5 || 
                   filename.substr(filename.length() - 5) != ".adoc") {
            // Only append .adoc if it doesn't already have it
            filename += ".adoc";
        }
        
        // Set directory if we have one
        if (!directory.empty()) {
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(doc_dialog), directory.c_str());
        }
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(doc_dialog), filename.c_str());
    } else {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(doc_dialog), "document.adoc");
    }
    
    gint doc_response = gtk_dialog_run(GTK_DIALOG(doc_dialog));
    gchar* doc_filename = NULL;
    
    if (doc_response == GTK_RESPONSE_ACCEPT) {
        doc_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doc_dialog));
    }
    
    // Destroy dialog immediately after getting response
    gtk_widget_destroy(doc_dialog);
    
    if (doc_response == GTK_RESPONSE_ACCEPT && doc_filename) {
        // Get document title from input box
        std::string doc_title = window->getDocumentTitle();
        
        // Generate AsciiDoc content
        std::string asciidoc_content = window->section_manager_->generateAsciiDoc(doc_title);
        
        // Write to file
        std::ofstream doc_file(doc_filename);
        if (doc_file.is_open()) {
            doc_file << asciidoc_content;
            doc_file.close();
        } else {
            GtkWidget* error_dialog = gtk_message_dialog_new(window->getWindow(),
                                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             "Failed to create AsciiDoc file.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(doc_filename);
    }
}

void MainWindow::onCreateMarkdown(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    
    // First, ensure the set is saved
    if (window->current_set_file_.empty() || window->has_unsaved_changes_) {
        // Prompt to save the set first
        GtkWidget* dialog = gtk_file_chooser_dialog_new("Save Section Set",
                                                         window->getWindow(),
                                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                                         "_Save", GTK_RESPONSE_ACCEPT,
                                                         NULL);
        
        if (!window->current_set_file_.empty()) {
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), window->current_set_file_.c_str());
        }
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gchar* filename = NULL;
        
        if (response == GTK_RESPONSE_ACCEPT) {
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        }
        
        // Destroy dialog immediately after getting response
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_ACCEPT && filename) {
            if (window->section_manager_->saveToFile(filename)) {
                window->current_set_file_ = filename;
                window->has_unsaved_changes_ = false;
                window->updateTitle();
            } else {
                g_free(filename);
                
                GtkWidget* error_dialog = gtk_message_dialog_new(window->getWindow(),
                                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                                 GTK_MESSAGE_ERROR,
                                                                 GTK_BUTTONS_OK,
                                                                 "Failed to save section set.");
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
                return;
            }
            
            g_free(filename);
        } else {
            return; // User cancelled
        }
    }
    
    // Now create the Markdown file
    GtkWidget* doc_dialog = gtk_file_chooser_dialog_new("Create Markdown",
                                                         window->getWindow(),
                                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                                         "_Save", GTK_RESPONSE_ACCEPT,
                                                         NULL);
    
    // Add file filter for .md files
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Markdown files (*.md)");
    gtk_file_filter_add_pattern(filter, "*.md");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(doc_dialog), filter);
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(doc_dialog), TRUE);
    
    // Set default filename based on set filename, replacing .docgenset with .md
    if (!window->current_set_file_.empty()) {
        // Extract directory and filename
        size_t last_slash = window->current_set_file_.find_last_of("/\\");
        std::string directory;
        std::string filename;
        
        if (last_slash != std::string::npos) {
            directory = window->current_set_file_.substr(0, last_slash);
            filename = window->current_set_file_.substr(last_slash + 1);
        } else {
            filename = window->current_set_file_;
        }
        
        // Replace .docgenset extension with .md, or add .md if not present
        if (filename.length() >= 10 && 
            filename.substr(filename.length() - 10) == ".docgenset") {
            filename = filename.substr(0, filename.length() - 10) + ".md";
        } else if (filename.length() < 3 || 
                   filename.substr(filename.length() - 3) != ".md") {
            // Only append .md if it doesn't already have it
            filename += ".md";
        }
        
        // Set directory if we have one
        if (!directory.empty()) {
            gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(doc_dialog), directory.c_str());
        }
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(doc_dialog), filename.c_str());
    } else {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(doc_dialog), "document.md");
    }
    
    gint doc_response = gtk_dialog_run(GTK_DIALOG(doc_dialog));
    gchar* doc_filename = NULL;
    
    if (doc_response == GTK_RESPONSE_ACCEPT) {
        doc_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doc_dialog));
    }
    
    // Destroy dialog immediately after getting response
    gtk_widget_destroy(doc_dialog);
    
    if (doc_response == GTK_RESPONSE_ACCEPT && doc_filename) {
        // Get document title from input box
        std::string doc_title = window->getDocumentTitle();
        
        // Generate Markdown content
        std::string markdown_content = window->section_manager_->generateMarkdown(doc_title);
        
        // Write to file
        std::ofstream doc_file(doc_filename);
        if (doc_file.is_open()) {
            doc_file << markdown_content;
            doc_file.close();
        } else {
            GtkWidget* error_dialog = gtk_message_dialog_new(window->getWindow(),
                                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             "Failed to create Markdown file.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(doc_filename);
    }
}

void MainWindow::onClearAll(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    window->section_manager_->clearAll();
    // Clear the document title field
    gtk_entry_set_text(GTK_ENTRY(window->document_title_entry_), "");
    window->has_unsaved_changes_ = false;
    window->current_set_file_ = "";
    window->updatePreview();
}

void MainWindow::onQuit(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    GtkApplication* app = gtk_window_get_application(window->getWindow());
    g_application_quit(G_APPLICATION(app));
}

void MainWindow::onAbout(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    
    GtkWidget* about_dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Text File Viewer");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), "1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
                                   "A simple text file viewer with drag-and-drop section ordering");
    gtk_window_set_transient_for(GTK_WINDOW(about_dialog), window->getWindow());
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);
}

void MainWindow::onSaveSet(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Save Section Set",
                                                     window->getWindow(),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                                     "_Save", GTK_RESPONSE_ACCEPT,
                                                     NULL);
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    
    // Set default filename if we have a current file
    if (!window->current_set_file_.empty()) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), window->current_set_file_.c_str());
    } else {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "section_set.docgenset");
    }
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // First save sections to get content
        if (window->section_manager_->saveToFile(filename)) {
            // Prepend document title to file
            std::string doc_title = window->getDocumentTitle();
            if (doc_title != "Default title") {
                std::ifstream input(filename);
                std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
                input.close();
                
                std::ofstream output(filename);
                output << "[DOCUMENT_TITLE:" << doc_title << "]\n";
                output << content;
                output.close();
            }
            
            window->current_set_file_ = filename;
            window->has_unsaved_changes_ = false;
            window->updateTitle();
        } else {
            GtkWidget* error_dialog = gtk_message_dialog_new(window->getWindow(),
                                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             "Failed to save section set to file.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

void MainWindow::onOpenSet(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    
    // Prompt to save if there are unsaved changes
    if (!window->promptSaveIfNeeded()) {
        return; // User cancelled
    }
    
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open Section Set",
                                                     window->getWindow(),
                                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                                     "_Open", GTK_RESPONSE_ACCEPT,
                                                     NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        if (window->section_manager_->loadFromFile(filename)) {
            window->current_set_file_ = filename;
            window->has_unsaved_changes_ = false;
            window->updateTitle();
            // Load document title from the loaded file
            std::string doc_title = window->section_manager_->getLoadedDocumentTitle();
            if (!doc_title.empty()) {
                gtk_entry_set_text(GTK_ENTRY(window->document_title_entry_), doc_title.c_str());
            }
            window->updatePreview();
        } else {
            GtkWidget* error_dialog = gtk_message_dialog_new(window->getWindow(),
                                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             "Failed to load section set from file.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

bool MainWindow::promptSaveIfNeeded() {
    if (!has_unsaved_changes_) {
        return true; // No unsaved changes, continue
    }
    
    GtkWidget* dialog = gtk_message_dialog_new(getWindow(),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_NONE,
                                               "Save changes before opening a new set?");
    
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                          "_Don't Save", GTK_RESPONSE_NO,
                          "_Cancel", GTK_RESPONSE_CANCEL,
                          "_Save", GTK_RESPONSE_YES,
                          NULL);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // Save the file
        onSaveSet(nullptr, this);
        return true;
    } else if (response == GTK_RESPONSE_NO) {
        // Don't save, continue
        return true;
    } else {
        // Cancel operation
        return false;
    }
}

std::string MainWindow::convertAsciiDocToHTML(const std::string& markdown) {
    std::string html = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<style>
html, body { margin: 0; padding: 0; overflow-x: hidden; }
body { font-family: sans-serif; padding: 10px; line-height: 1.6; transform: scale(0.2); transform-origin: top left; width: 460%; max-width: 460%; word-wrap: break-word; overflow-wrap: break-word; box-sizing: border-box; }
h1 { font-size: 2em; margin-top: 0; }
h2 { font-size: 1.5em; margin-top: 1em; }
h3 { font-size: 1.25em; margin-top: 0.8em; }
h4 { font-size: 1.1em; margin-top: 0.6em; }
blockquote { border-left: 4px solid #ddd; padding-left: 15px; color: #666; margin: 1em 0; word-wrap: break-word; }
pre { background: #f5f5f5; border: 1px solid #ddd; padding: 10px; border-radius: 4px; overflow-x: auto; white-space: pre-wrap; word-wrap: break-word; max-width: 100%; }
code { background: #f0f0f0; padding: 2px 4px; border-radius: 3px; }
</style>
</head>
<body>
)";
    
    std::istringstream stream(markdown);
    std::string line;
    bool in_quote = false;
    bool in_code = false;
    bool in_paragraph = false;
    
    while (std::getline(stream, line)) {
        // Code block
        if (line.substr(0, 3) == "```") {
            if (!in_code) {
                if (in_paragraph) { html += "</p>\n"; in_paragraph = false; }
                html += "<pre><code>";
                in_code = true;
            } else {
                html += "</code></pre>\n";
                in_code = false;
            }
            continue;
        }
        
        if (in_code) {
            html += line + "\n";
            continue;
        }
        
        // Document title (# Title)
        if (line.substr(0, 2) == "# ") {
            if (in_paragraph) { html += "</p>\n"; in_paragraph = false; }
            html += "<h1>" + line.substr(2) + "</h1>\n";
        }
        // Level 1 heading (## Heading)
        else if (line.substr(0, 3) == "## ") {
            if (in_paragraph) { html += "</p>\n"; in_paragraph = false; }
            html += "<h2>" + line.substr(3) + "</h2>\n";
        }
        // Level 2 heading (### Heading)
        else if (line.substr(0, 4) == "### ") {
            if (in_paragraph) { html += "</p>\n"; in_paragraph = false; }
            html += "<h3>" + line.substr(4) + "</h3>\n";
        }
        // Level 3 heading (#### Heading)
        else if (line.substr(0, 5) == "#### ") {
            if (in_paragraph) { html += "</p>\n"; in_paragraph = false; }
            html += "<h4>" + line.substr(5) + "</h4>\n";
        }
        // Blockquote (> text)
        else if (line.substr(0, 2) == "> ") {
            if (!in_quote) {
                if (in_paragraph) { html += "</p>\n"; in_paragraph = false; }
                html += "<blockquote>";
                in_quote = true;
            }
            html += line.substr(2) + "<br>\n";
        }
        // Empty line
        else if (line.empty()) {
            if (in_quote) {
                html += "</blockquote>\n";
                in_quote = false;
            }
            if (in_paragraph) {
                html += "</p>\n";
                in_paragraph = false;
            }
        }
        // Normal text
        else {
            if (in_quote) {
                html += "</blockquote>\n";
                in_quote = false;
            }
            if (!in_paragraph) {
                html += "<p>";
                in_paragraph = true;
            }
            html += line + " ";
        }
    }
    
    if (in_paragraph) html += "</p>\n";
    if (in_quote) html += "</blockquote>\n";
    if (in_code) html += "</code></pre>\n";
    
    html += "</body></html>";
    return html;
}

void MainWindow::updatePreview() {
    if (!preview_web_view_ || !section_manager_) {
        return;
    }
    
    // Generate Markdown content
    std::string markdown_content = section_manager_->generateMarkdown(getDocumentTitle());
    
    // Convert to HTML for rendering
    std::string html_content = convertAsciiDocToHTML(markdown_content);
    
    // Load HTML content into WebView
    webkit_web_view_load_html(preview_web_view_, html_content.c_str(), nullptr);
}

GtkWindow* MainWindow::getWindow() const {
    return GTK_WINDOW(window_);
}
