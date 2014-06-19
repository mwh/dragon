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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define VERSION "0.3.0"


GtkWidget *window;
GtkWidget *vbox;

char *progname;
bool verbose = false;
int mode = 0;

#define MODE_HELP 1
#define MODE_LIST_CONFIGS 2
#define MODE_BACKENDS 3
#define MODE_VERSION 4

#define TARGET_TYPE_TEXT 1
#define TARGET_TYPE_URI 2

struct draggable_thing {
    char *text;
    char *uri;
    guint last_time;
};

void do_quit(GtkWidget *widget, gpointer data) {
    exit(0);
}

void button_clicked(GtkWidget *widget, gpointer user_data) {
    struct draggable_thing *dd = (struct draggable_thing *)user_data;
    if (0 == fork()) {
        execlp("xdg-open", "xdg-open", dd->uri, NULL);
    }
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
        char *uris[] = {dd->uri, NULL};
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

void add_button(char *label, struct draggable_thing *dragdata, int type) {
    GtkWidget *button = gtk_button_new();
    gtk_button_set_label(GTK_BUTTON(button), label);
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
 
    gtk_container_add(GTK_CONTAINER(vbox), button);
}

void add_file_button(char *ufilename, char *filename) {
    char *uri = malloc(strlen(filename) + 8); // file:// + NULL
    strcpy(uri, "file://");
    strcat(uri, filename);
    struct draggable_thing *dragdata = malloc(sizeof(struct draggable_thing));
    dragdata->text = filename;
    dragdata->uri = uri;
    add_button(ufilename, dragdata, TARGET_TYPE_URI);
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

int main (int argc, char **argv) {
    progname = argv[0];
    char *filename = NULL;
    char *ufilename = NULL;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            mode = MODE_HELP;
        } else if (strcmp(argv[i], "--version") == 0) {
            mode = MODE_VERSION;
        } else if (strcmp(argv[i], "-v") == 0
                || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "%s: error: unknown option `%s'.\n",
                    progname, argv[i]);
        }
    }
    switch(mode) {
        case MODE_VERSION:
            puts("dragon " VERSION);
            puts("Copyright (C) 2014 Michael Homer");
            puts("This program comes with ABSOLUTELY NO WARRANTY.");
            puts("See the source for copying conditions.");
            exit(0);
        case MODE_HELP:
            printf("dragon - lightweight DnD source/target\n");
            printf("Usage: %s [OPTION] [FILENAME]\n", argv[0]);
            printf("  --verbose, -v   be verbose\n");
            printf("  --help          show help\n");
            printf("  --version       show version details\n");
            exit(0);
    }
    GtkAccelGroup *accelgroup;
    GClosure *closure;

    gtk_init(&argc, &argv);
 
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    closure = g_cclosure_new(G_CALLBACK(do_quit), NULL, NULL);
    accelgroup = gtk_accel_group_new();
    gtk_accel_group_connect(accelgroup, GDK_KEY_Escape, 0, 0, closure);
    gtk_window_add_accel_group(GTK_WINDOW(window), accelgroup);
 
    gtk_window_set_title(GTK_WINDOW(window), "Run");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
 
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    bool had_filename = false;
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') {
            filename = ufilename = argv[i];
            if (!is_uri(filename)) {
                filename = realpath(filename, NULL);
                if (!filename) {
                    fprintf(stderr, "The file `%s' does not exist.\n",
                            ufilename);
                    exit(1);
                }
                add_file_button(ufilename, filename);
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
 
    gtk_container_add(GTK_CONTAINER(window), vbox);

    gtk_window_set_title(GTK_WINDOW(window), "dragon");
 
    gtk_widget_show_all(window);
 
    gtk_main();
 
    return 0;
}
