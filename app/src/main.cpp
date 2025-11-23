#include <gtk/gtk.h>
#include "main_window.h"

// External reference to compiled GResource
extern "C" {
    extern GResource* docgen_get_resource(void);
}

static void activate(GtkApplication* app, gpointer user_data) {
    (void)user_data;
    
    MainWindow* main_window = new MainWindow(app);
    main_window->show();
    
    // Store pointer for cleanup
    g_object_set_data_full(G_OBJECT(app), "main_window", main_window,
                          [](gpointer data) { delete static_cast<MainWindow*>(data); });
}

int main(int argc, char** argv) {
    // Register embedded resources
    GResource* resource = docgen_get_resource();
    g_resources_register(resource);
    
    GtkApplication* app = gtk_application_new("com.docgen.textviewer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    // Unregister resources
    g_resources_unregister(resource);
    
    return status;
}
