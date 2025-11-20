#include "main_window.h"
#include <iostream>
#include <fstream>

MainWindow::MainWindow(GtkApplication* app)
    : window_(nullptr), main_vbox_(nullptr), section_manager_(nullptr),
      has_unsaved_changes_(false), current_set_file_("") {
    
    window_ = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window_), "Text File Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window_), 800, 600);

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

    GtkWidget* create_doc_item = gtk_menu_item_new_with_label("Create Doc...");
    g_signal_connect(create_doc_item, "activate", G_CALLBACK(onCreateDoc), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), create_doc_item);

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
}

void MainWindow::createUI() {
    // Apply CSS for modern styling
    GtkCssProvider* css_provider = gtk_css_provider_new();
    const gchar* css_data = 
        "window { background-color: #f5f5f5; }"
        ".section-container { "
        "  background-color: white; "
        "  border-radius: 8px; "
        "  box-shadow: 0 2px 4px rgba(0,0,0,0.1); "
        "  margin: 8px; "
        "  padding: 12px; "
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
        "textview { "
        "  border-radius: 4px; "
        "  padding: 8px; "
        "  font-family: monospace; "
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
    
    GtkWidget* order_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(order_box, 12);
    gtk_widget_set_margin_end(order_box, 12);
    gtk_widget_set_margin_top(order_box, 8);
    gtk_widget_set_margin_bottom(order_box, 8);
    gtk_container_add(GTK_CONTAINER(order_scrolled), order_box);
    gtk_box_pack_start(GTK_BOX(order_label_box), order_scrolled, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(order_frame), order_label_box);
    gtk_widget_set_margin_start(order_frame, 12);
    gtk_widget_set_margin_end(order_frame, 12);
    gtk_widget_set_margin_top(order_frame, 12);
    gtk_widget_set_margin_bottom(order_frame, 8);
    gtk_box_pack_start(GTK_BOX(main_vbox_), order_frame, FALSE, FALSE, 0);

    // Main scrolled window to contain all text views
    GtkWidget* main_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(main_scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_margin_start(main_scrolled, 12);
    gtk_widget_set_margin_end(main_scrolled, 12);
    gtk_widget_set_margin_bottom(main_scrolled, 12);
    gtk_box_pack_start(GTK_BOX(main_vbox_), main_scrolled, TRUE, TRUE, 0);

    // Container for all text views
    GtkWidget* text_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_add(GTK_CONTAINER(main_scrolled), text_container);

    // Create section manager
    section_manager_ = std::make_unique<SectionManager>(text_container, order_box);
}

void MainWindow::show() {
    gtk_widget_show_all(window_);
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
        
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            
            if (window->section_manager_->saveToFile(filename)) {
                window->current_set_file_ = filename;
                window->has_unsaved_changes_ = false;
                window->updateTitle();
            } else {
                g_free(filename);
                gtk_widget_destroy(dialog);
                
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
            gtk_widget_destroy(dialog);
            return; // User cancelled
        }
        
        gtk_widget_destroy(dialog);
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
    
    // Set default filename based on set filename, replacing .txt with .adoc
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
        
        // Replace .txt extension with .adoc
        if (filename.length() >= 4 && 
            filename.substr(filename.length() - 4) == ".txt") {
            filename = filename.substr(0, filename.length() - 4) + ".adoc";
        } else {
            // If not .txt, just append .adoc
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
    
    if (gtk_dialog_run(GTK_DIALOG(doc_dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar* doc_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doc_dialog));
        
        // Extract set name without extension for document title
        std::string doc_title;
        if (!window->current_set_file_.empty()) {
            size_t last_slash = window->current_set_file_.find_last_of("/\\");
            size_t last_dot = window->current_set_file_.find_last_of('.');
            std::string base_name = window->current_set_file_;
            if (last_slash != std::string::npos) {
                base_name = base_name.substr(last_slash + 1);
            }
            if (last_dot != std::string::npos) {
                size_t dot_pos = base_name.find_last_of('.');
                if (dot_pos != std::string::npos) {
                    doc_title = base_name.substr(0, dot_pos);
                } else {
                    doc_title = base_name;
                }
            } else {
                doc_title = base_name;
            }
        }
        
        // Generate AsciiDoc content
        std::string asciidoc_content = window->section_manager_->generateAsciiDoc(doc_title);
        
        // Write to file
        std::ofstream doc_file(doc_filename);
        if (doc_file.is_open()) {
            doc_file << asciidoc_content;
            doc_file.close();
            
            // Show success message
            GtkWidget* success_dialog = gtk_message_dialog_new(window->getWindow(),
                                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                                               GTK_MESSAGE_INFO,
                                                               GTK_BUTTONS_OK,
                                                               "AsciiDoc created successfully.");
            gtk_dialog_run(GTK_DIALOG(success_dialog));
            gtk_widget_destroy(success_dialog);
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
    
    gtk_widget_destroy(doc_dialog);
}

void MainWindow::onClearAll(GtkMenuItem* item, gpointer user_data) {
    (void)item;
    MainWindow* window = static_cast<MainWindow*>(user_data);
    window->section_manager_->clearAll();
    window->has_unsaved_changes_ = false;
    window->current_set_file_ = "";
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
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "section_set.txt");
    }
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        if (window->section_manager_->saveToFile(filename)) {
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
