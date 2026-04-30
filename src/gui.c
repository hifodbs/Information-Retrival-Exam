#include <adwaita.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "gui.h"
#include "queryparser.h"
#include "IO/bin_reader.h"
#include "IO/doc_retriever.h"

// External references to files defined in main.c or elsewhere
extern char *BIN_FILE;
extern char *BIN_REDUCED_FILE;
extern char *SOURCE_FILE;

typedef struct {
    AdwApplicationWindow *window;
    GtkSearchEntry *search_entry;
    GtkListBox *results_list;
    AdwSwitchRow *mode_switch;
    int current_mode; // 0 for standard, 1 for reduced
} GuiContext;

static void on_row_activated(GtkListBox *listbox, GtkListBoxRow *row, gpointer user_data) {
    Document *doc = g_object_get_data(G_OBJECT(row), "document");
    if (doc) {
        GtkWidget *parent = GTK_WIDGET(gtk_widget_get_root(GTK_WIDGET(listbox)));

        // Use a heading for the title and an extra child for the abstract to ensure left alignment
        AdwDialog *dialog = adw_alert_dialog_new(doc->title, NULL);

        GtkWidget *abstract_label = gtk_label_new(doc->abstract ? doc->abstract : "No abstract available.");
        gtk_label_set_wrap(GTK_LABEL(abstract_label), TRUE);
        gtk_label_set_xalign(GTK_LABEL(abstract_label), 0.0); // Left align
        gtk_label_set_selectable(GTK_LABEL(abstract_label), TRUE);

        // Add some padding to the abstract
        gtk_widget_set_margin_start(abstract_label, 12);
        gtk_widget_set_margin_end(abstract_label, 12);
        gtk_widget_set_margin_top(abstract_label, 6);
        gtk_widget_set_margin_bottom(abstract_label, 12);

        adw_alert_dialog_set_extra_child(ADW_ALERT_DIALOG(dialog), abstract_label);

        adw_alert_dialog_add_response(ADW_ALERT_DIALOG(dialog), "close", "Close");
        adw_alert_dialog_choose(ADW_ALERT_DIALOG(dialog), parent, NULL, NULL, NULL);
    }
}


static void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    GuiContext *ctx = (GuiContext *)user_data;
    const char *query_text = gtk_editable_get_text(GTK_EDITABLE(entry));
    
    if (get_dict() == NULL) {
        return;
    }

    // Clear previous results
    GtkListBoxRow *row;
    while ((row = gtk_list_box_get_row_at_index(ctx->results_list, 0)) != NULL) {
        Document *doc = g_object_get_data(G_OBJECT(row), "document");
        if (doc) free_document(doc);
        gtk_list_box_remove(ctx->results_list, GTK_WIDGET(row));
    }

    if (strlen(query_text) == 0) {
        return;
    }

    char *query = strdup(query_text);
    for (size_t i = 0; i < strlen(query); i++)
        *(query + i) = tolower(*(query + i));

    int *result = NULL;
    int len = 0;

    query_parser(&result, &len, query);

    if (len == 0) {
        GtkWidget *label = gtk_label_new("No results found");
        gtk_list_box_append(ctx->results_list, label);
    } else {
        for (int i = 0; i < len; i++) {
            Document *doc = get_document_by_id(SOURCE_FILE, result[i]);
            if (doc) {
                GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
                gtk_widget_set_margin_start(row_box, 8);
                gtk_widget_set_margin_end(row_box, 8);
                gtk_widget_set_margin_top(row_box, 8);
                gtk_widget_set_margin_bottom(row_box, 8);
                
                GtkWidget *title_label = gtk_label_new(doc->title);
                gtk_label_set_wrap(GTK_LABEL(title_label), TRUE);
                gtk_label_set_xalign(GTK_LABEL(title_label), 0.0);
                gtk_widget_add_css_class(title_label, "title-4");
                gtk_box_append(GTK_BOX(row_box), title_label);

                char id_buf[32];
                snprintf(id_buf, sizeof(id_buf), "Document ID: %d", doc->id);
                GtkWidget *id_label = gtk_label_new(id_buf);
                gtk_label_set_xalign(GTK_LABEL(id_label), 0.0);
                gtk_widget_add_css_class(id_label, "caption");
                gtk_box_append(GTK_BOX(row_box), id_label);

                gtk_list_box_append(ctx->results_list, row_box);
                
                // Get the row we just added to attach the document data
                GtkListBoxRow *row_widget = gtk_list_box_get_row_at_index(ctx->results_list, i);
                g_object_set_data(G_OBJECT(row_widget), "document", doc);
            }
        }
    }

    free(query);
    if (result) free(result);
}

static void on_mode_toggled(GObject *object, GParamSpec *pspec, gpointer user_data) {
    GuiContext *ctx = (GuiContext *)user_data;
    gboolean active = adw_switch_row_get_active(ctx->mode_switch);
    
    int new_mode = active ? 1 : 0;
    if (new_mode != ctx->current_mode) {
        ctx->current_mode = new_mode;
        close_bin_reader();
        
        char *filename = (ctx->current_mode == 0) ? BIN_FILE : BIN_REDUCED_FILE;
        init_bin_reader(filename);
        init_query_parser(ctx->current_mode);
        
        // Refresh search if there is text
        on_search_changed(ctx->search_entry, ctx);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    GuiContext *ctx = g_new0(GuiContext, 1);

    GtkWidget *window = adw_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Information Retrieval System");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    ctx->window = ADW_APPLICATION_WINDOW(window);

    GtkWidget *toolbar_view = adw_toolbar_view_new();
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), toolbar_view);

    GtkWidget *header_bar = adw_header_bar_new();
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar_view), header_bar);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar_view), main_box);

    // Mode Selection
    GtkWidget *preferences_group = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(preferences_group), "Settings");
    gtk_box_append(GTK_BOX(main_box), preferences_group);

    ctx->mode_switch = ADW_SWITCH_ROW(adw_switch_row_new());
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(ctx->mode_switch), "Reduced Mode");
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(preferences_group), GTK_WIDGET(ctx->mode_switch));
    g_signal_connect(ctx->mode_switch, "notify::active", G_CALLBACK(on_mode_toggled), ctx);

    // Search Entry
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(search_box, 12);
    gtk_widget_set_margin_end(search_box, 12);
    gtk_widget_set_margin_top(search_box, 12);
    gtk_widget_set_margin_bottom(search_box, 12);
    gtk_box_append(GTK_BOX(main_box), search_box);

    ctx->search_entry = GTK_SEARCH_ENTRY(gtk_search_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(ctx->search_entry), TRUE);
    gtk_search_entry_set_placeholder_text(ctx->search_entry, "Search queries (e.g., 'dog and cat')...");
    gtk_box_append(GTK_BOX(search_box), GTK_WIDGET(ctx->search_entry));
    g_signal_connect(ctx->search_entry, "activate", G_CALLBACK(on_search_changed), ctx);

    // Results List
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(main_box), scrolled_window);

    ctx->results_list = GTK_LIST_BOX(gtk_list_box_new());
    gtk_list_box_set_selection_mode(ctx->results_list, GTK_SELECTION_SINGLE);
    g_signal_connect(ctx->results_list, "row-activated", G_CALLBACK(on_row_activated), ctx);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(preferences_group), gtk_label_new("Results")); // Just a separator-like label
    
    GtkWidget *results_group = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(results_group), "Search Results");
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(results_group), GTK_WIDGET(ctx->results_list));
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), results_group);

    // Initialize IR Backend
    ctx->current_mode = 0;
    init_bin_reader(BIN_FILE);
    init_query_parser(ctx->current_mode);

    if (get_dict() == NULL) {
        AdwDialog *dialog = adw_alert_dialog_new("Error", "Could not initialize Information Retrieval backend. Make sure the binary index files exist in the 'bin/' directory.");
        adw_alert_dialog_add_response(ADW_ALERT_DIALOG(dialog), "ok", "OK");
        adw_alert_dialog_set_default_response(ADW_ALERT_DIALOG(dialog), "ok");
        adw_alert_dialog_choose(ADW_ALERT_DIALOG(dialog), window, NULL, NULL, NULL);
    }

    gtk_window_present(GTK_WINDOW(window));
}

int gui_run(int argc, char *argv[]) {
    AdwApplication *app = adw_application_new("org.example.ir", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    // We only pass the first argument (program name) to avoid GApplication 
    // complaining about the unknown --gui flag.
    int fake_argc = 1;
    char *fake_argv[] = { argv[0], NULL };
    
    int status = g_application_run(G_APPLICATION(app), fake_argc, fake_argv);
    g_object_unref(app);
    return status;
}
