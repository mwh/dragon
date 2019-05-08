// dragon - very lightweight DnD file source/target
// Copyright 2014 Michael Homer.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define VERSION "1.0.0"


GtkWidget *window;
GtkWidget *vbox;
GtkIconTheme *icon_theme;

char *progname;
bool verbose = false;
int mode = 0;
bool and_exit;
bool keep;

#define MODE_HELP 1
#define MODE_TARGET 2
#define MODE_VERSION 4

#define TARGET_TYPE_TEXT 1
#define TARGET_TYPE_URI 2

struct draggable_thing {
    char *text;
    char *uri;
    guint last_time;
};

// MODE_ALL
#define MAX_SIZE 100
char** uri_collection;
int uri_count;
bool drag_all = false;
// ---

void add_target_button();

void do_quit(GtkWidget *widget, gpointer data) {
    exit(0);
}

void button_clicked(GtkWidget *widget, gpointer user_data) {
    struct draggable_thing *dd = (struct draggable_thing *)user_data;
    if (0 == fork()) {
        execlp("xdg-open", "xdg-open", dd->uri, NULL);
    }
}

void drag_end(GtkWidget *widget, GdkDragContext *context, gpointer user_data) {
    if (and_exit)
        gtk_main_quit();
}

void drag_data_get(GtkWidget    *widget,
               GdkDragContext   *context,
               GtkSelectionData *data,
               guint             info,
               guint             time,
               gpointer          user_data) {
    struct draggable_thing *dd = (struct draggable_thing *)user_data;
    if (info == TARGET_TYPE_URI) {
        if (verbose)
            printf("Writing as URI: %s\n", dd->uri);

        char** uris;
        if(drag_all){
            uri_collection[uri_count] = NULL;
            uris = uri_collection;
        } else {
            char* a[] = {dd->uri, NULL};
            uris = a;
        }

        gtk_selection_data_set_uris(data, uris);
        g_signal_stop_emission_by_name(widget, "drag-data-get");
    } else if (info == TARGET_TYPE_TEXT) {
        if (verbose)
            printf("Writing as TEXT: %s\n", dd->text);
        gtk_selection_data_set_text(data, dd->text, -1);
    } else {
        fprintf(stderr, "Error: bad target type %i\n", info);
    }
}

GtkButton *add_button(char *label, struct draggable_thing *dragdata, int type) {
    GtkWidget *button = gtk_button_new_with_label(label);

    GtkTargetList *targetlist = gtk_drag_source_get_target_list(GTK_WIDGET(button));
    if (targetlist)
        gtk_target_list_ref(targetlist);
    else
        targetlist = gtk_target_list_new(NULL, 0);
    if (type == TARGET_TYPE_URI)
        gtk_target_list_add_uri_targets(targetlist, TARGET_TYPE_URI);
    else
        gtk_target_list_add_text_targets(targetlist, TARGET_TYPE_TEXT);
    gtk_drag_source_set(GTK_WIDGET(button), GDK_BUTTON1_MASK, NULL, 0,
            GDK_ACTION_DEFAULT | GDK_ACTION_LINK | GDK_ACTION_COPY);
    gtk_drag_source_set_target_list(GTK_WIDGET(button), targetlist);
    g_signal_connect(GTK_WIDGET(button), "drag-data-get",
            G_CALLBACK(drag_data_get), dragdata);
    g_signal_connect(GTK_WIDGET(button), "clicked",
            G_CALLBACK(button_clicked), dragdata);
    g_signal_connect(GTK_WIDGET(button), "drag-end",
            G_CALLBACK(drag_end), dragdata);

    gtk_container_add(GTK_CONTAINER(vbox), button);

    if(drag_all)
        uri_collection[uri_count++] = dragdata->uri;

    return (GtkButton *)button;
}

GtkIconInfo* icon_info_from_content_type(char *content_type) {
    GIcon *icon = g_content_type_get_icon(content_type);
    return gtk_icon_theme_lookup_by_gicon(icon_theme, icon, 48, 0);
}

void add_file_button(char *filename) {
    GFile *file = g_file_new_for_path(filename);
    if(!g_file_query_exists(file, NULL)) {
        fprintf(stderr, "The file `%s' does not exist.\n",
                filename);
        exit(1);
    }
    char *uri = g_file_get_uri(file);
    struct draggable_thing *dragdata = malloc(sizeof(struct draggable_thing));
    dragdata->text = filename;
    dragdata->uri = uri;

    GtkButton *button = add_button(filename, dragdata, TARGET_TYPE_URI);
    GFileInfo *fileinfo = g_file_query_info(file, "*", 0, NULL, NULL);
    GIcon *icon = g_file_info_get_icon(fileinfo);
    GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon(icon_theme,
            icon, 48, 0);
    // Try a few fallback mimetypes if no icon can be found
    if (!icon_info)
        icon_info = icon_info_from_content_type("application/octet-stream");
    if (!icon_info)
        icon_info = icon_info_from_content_type("text/x-generic");
    if (!icon_info)
        icon_info = icon_info_from_content_type("text/plain");

    if (icon_info) {
        GtkWidget *image = gtk_image_new_from_pixbuf(
                gtk_icon_info_load_icon(icon_info, NULL));
        gtk_button_set_image(button, image);
        gtk_button_set_always_show_image(button, true);
    }

    GList *child = g_list_first(
            gtk_container_get_children(GTK_CONTAINER(button)));
    if (child)
        gtk_widget_set_halign(GTK_WIDGET(child->data), GTK_ALIGN_START);
}

void add_uri_button(char *uri) {
    struct draggable_thing *dragdata = malloc(sizeof(struct draggable_thing));
    dragdata->text = uri;
    dragdata->uri = uri;
    add_button(uri, dragdata, TARGET_TYPE_URI);
}

bool is_uri(char *uri) {
    for (int i=0; uri[i]; i++)
        if (uri[i] == '/')
            return false;
        else if (uri[i] == ':')
            return true;
    return false;
}

gboolean drag_drop (GtkWidget *widget,
               GdkDragContext *context,
               gint            x,
               gint            y,
               guint           time,
               gpointer        user_data) {
    GtkTargetList *targetlist = gtk_drag_dest_get_target_list(widget);
    GList *list = gdk_drag_context_list_targets(context);
    if (list) {
        while (list) {
            GdkAtom atom = (GdkAtom)g_list_nth_data(list, 0);
            if (gtk_target_list_find(targetlist,
                        GDK_POINTER_TO_ATOM(g_list_nth_data(list, 0)), NULL)) {
                gtk_drag_get_data(widget, context, atom, time);
                return true;
            }
            list = g_list_next(list);
        }
    }
    gtk_drag_finish(context, false, false, time);
    return true;
}

void
drag_data_received (GtkWidget          *widget,
                    GdkDragContext     *context,
                    gint                x,
                    gint                y,
                    GtkSelectionData   *data,
                    guint               info,
                    guint               time) {
    gchar **uris = gtk_selection_data_get_uris(data);
    unsigned char *text = gtk_selection_data_get_text(data);
    if (!uris && !text)
        gtk_drag_finish (context, FALSE, FALSE, time);
    if (uris) {
        gtk_container_remove(GTK_CONTAINER(vbox), widget);
        for (; *uris; uris++) {
            printf("%s\n", *uris);
            if (keep)
                add_uri_button(*uris);
        }
        add_target_button();
        gtk_widget_show_all(window);
    } else if (text) {
        printf("%s\n", text);
    }
    gtk_drag_finish (context, TRUE, FALSE, time);
    if (and_exit)
        gtk_main_quit();
}

void add_target_button() {
    GtkWidget *label = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(label), "Drag something here...");
    gtk_container_add(GTK_CONTAINER(vbox), label);
    GtkTargetList *targetlist = gtk_drag_dest_get_target_list(GTK_WIDGET(label));
    if (targetlist)
        gtk_target_list_ref(targetlist);
    else
        targetlist = gtk_target_list_new(NULL, 0);
    gtk_target_list_add_text_targets(targetlist, TARGET_TYPE_TEXT);
    gtk_target_list_add_uri_targets(targetlist, TARGET_TYPE_URI);
    gtk_drag_dest_set(GTK_WIDGET(label),
            GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT, NULL, 0,
            GDK_ACTION_COPY);
    gtk_drag_dest_set_target_list(GTK_WIDGET(label), targetlist);
    g_signal_connect(GTK_WIDGET(label), "drag-drop",
            G_CALLBACK(drag_drop), NULL);
    g_signal_connect(GTK_WIDGET(label), "drag-data-received",
            G_CALLBACK(drag_data_received), NULL);
}

void target_mode() {
    add_target_button();
    gtk_widget_show_all(window);
    gtk_main();
}

int main (int argc, char **argv) {
    progname = argv[0];
    char *filename = NULL;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            mode = MODE_HELP;
            printf("dragon - lightweight DnD source/target\n");
            printf("Usage: %s [OPTION] [FILENAME]\n", argv[0]);
            printf("  --and-exit, -x  exit after a single completed drop\n");
            printf("  --target,   -t  act as a target instead of source\n");
            printf("  --keep,     -k  with --target, keep files to drag out\n");
            printf("  --all,      -a  drag all files at once\n");
            printf("  --verbose,  -v  be verbose\n");
            printf("  --help          show help\n");
            printf("  --version       show version details\n");
            exit(0);
        } else if (strcmp(argv[i], "--version") == 0) {
            mode = MODE_VERSION;
            puts("dragon " VERSION);
            puts("Copyright (C) 2014-2018 Michael Homer");
            puts("This program comes with ABSOLUTELY NO WARRANTY.");
            puts("See the source for copying conditions.");
            exit(0);
        } else if (strcmp(argv[i], "-v") == 0
                || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-t") == 0
                || strcmp(argv[i], "--target") == 0) {
            mode = MODE_TARGET;
        } else if (strcmp(argv[i], "-x") == 0
                || strcmp(argv[i], "--and-exit") == 0) {
            and_exit = true;
        } else if (strcmp(argv[i], "-k") == 0
                || strcmp(argv[i], "--keep") == 0) {
            keep = true;
        } else if (strcmp(argv[i], "-a") == 0
                || strcmp(argv[i], "--all") == 0) {
            drag_all = true;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "%s: error: unknown option `%s'.\n",
                    progname, argv[i]);
        }
    }
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);

    GtkAccelGroup *accelgroup;
    GClosure *closure;

    gtk_init(&argc, &argv);

    icon_theme = gtk_icon_theme_get_default();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    closure = g_cclosure_new(G_CALLBACK(do_quit), NULL, NULL);
    accelgroup = gtk_accel_group_new();
    gtk_accel_group_connect(accelgroup, GDK_KEY_Escape, 0, 0, closure);
    gtk_window_add_accel_group(GTK_WINDOW(window), accelgroup);

    gtk_window_set_title(GTK_WINDOW(window), "Run");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_window_set_title(GTK_WINDOW(window), "dragon");

    if (mode == MODE_TARGET) {
            target_mode();
            exit(0);
    }

    if(drag_all){
       uri_collection = malloc(sizeof(char*) * (argc > MAX_SIZE? argc: MAX_SIZE));
       uri_count = 0;
    }

    bool had_filename = false;
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') {
            filename = argv[i];
            if (!is_uri(filename)) {
                add_file_button(filename);
            } else {
                add_uri_button(filename);
            }
            had_filename = true;
        }
    }
    if (!had_filename) {
        printf("Usage: %s [OPTIONS] FILENAME\n", progname);
        exit(0);
    }

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
