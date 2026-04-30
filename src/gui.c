#include <adwaita.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "gui.h"
#include "queryparser.h"
#include "IO/bin_reader.h"

// External references to files defined in main.c or elsewhere
extern char *BIN_FILE;
extern char *BIN_REDUCED_FILE;

typedef struct {
    AdwApplicationWindow *window;
    GtkSearchEntry *search_entry;
    GtkListBox *results_list;
    AdwSwitchRow *mode_switch;
    int current_mode; // 0 for standard, 1 for reduced
} GuiContext;

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data) {
    GuiContext *ctx = (GuiContext *)user_data;
    const char *query_text = gtk_editable_get_text(GTK_EDITABLE(entry));
    
    if (get_dict() == NULL) {
        return;
    }

    if (strlen(query_text) == 0) {
        // Clear results
        GtkListBoxRow *row;
        while ((row = gtk_list_box_get_row_at_index(ctx->results_list, 0)) != NULL) {
            gtk_list_box_remove(ctx->results_list, GTK_WIDGET(row));
        }
        return;
    }

    char *query = strdup(query_text);
    for (size_t i = 0; i < strlen(query); i++)
        *(query + i) = tolower(*(query + i));

    int *result = NULL;
    int len = 0;

    query_parser(&result, &len, query);

    // Clear previous results
    GtkListBoxRow *row;
    while ((row = gtk_list_box_get_row_at_index(ctx->results_list, 0)) != NULL) {
        gtk_list_box_remove(ctx->results_list, GTK_WIDGET(row));
    }

    if (len == 0) {
        GtkWidget *label = gtk_label_new("No results found");
        gtk_list_box_append(ctx->results_list, label);
    } else {
        for (int i = 0; i < len; i++) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "Document ID: %d", result[i]);
            GtkWidget *label = gtk_label_new(buffer);
            gtk_widget_set_halign(label, GTK_ALIGN_START);
            gtk_widget_set_margin_start(label, 12);
            gtk_widget_set_margin_end(label, 12);
            gtk_widget_set_margin_top(label, 6);
            gtk_widget_set_margin_bottom(label, 6);
            gtk_list_box_append(ctx->results_list, label);
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
    gtk_list_box_set_selection_mode(ctx->results_list, GTK_SELECTION_NONE);
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
