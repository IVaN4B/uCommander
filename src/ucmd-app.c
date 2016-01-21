#include <gtk/gtk.h>
#include "ucmd-app.h"

#include <glib/gi18n.h>
#include "ucmd-dir-list.h"

#define UI_FILE "src/ui/main_window.ui"
#define TOP_WINDOW "main_window"

G_DEFINE_TYPE (Ucommander, ucommander, GTK_TYPE_APPLICATION);

struct _UcommanderPrivate {
	GtkTreeView *left_view;
	GtkTreeView *right_view;
	UcommanderDirList *left_list;
	UcommanderDirList *right_list;
};



static gchar *ucmd_list_columns[NUM_COLUMNS] = {
		"Path",
		"Name",
		"Type",
		"Size",
		"Date",
		"Attributes",
		"Is dir"
	};


/* Init file lists */
static void ucommander_init_lists(UcommanderPrivate *priv){
	/* TODO: Store this in settings */
	const gchar *init_path = "/";

	int result = ucmd_create_dir_list(init_path, &priv->left_list);
	if( result != 0 ){
		/* TODO: Handle error */
		g_error("Failed to create list");
	}

	result = ucmd_create_dir_list(init_path, &priv->right_list);
	if( result != 0 ){
		/* TODO: Handle error */
	}
}

static void row_clicked(GtkTreeView *view, GtkTreePath *tree_path,
						GtkTreeViewColumn *column,
						gpointer user_data){
	GtkListStore *store;
	gchar *path;
	GtkTreeIter iter;
	gboolean is_dir;
	store = GTK_LIST_STORE(user_data);

	gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, tree_path);
	gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, PATH_COLUMN, &path,
					ISDIR_COLUMN, &is_dir, -1);

	g_message(path);
	if( is_dir ){
		ucmd_read_dir(path, store);
	}

	g_free(path);
}

static void ucommander_init_views(UcommanderPrivate *priv,
									GtkBuilder *builder){
	priv->left_view = (GtkTreeView *)GTK_WIDGET(gtk_builder_get_object(builder,
							"left_view"));
	priv->right_view = (GtkTreeView *)GTK_WIDGET(gtk_builder_get_object(builder,
							"right_view"));

	gtk_tree_view_set_model(priv->left_view,
					GTK_TREE_MODEL(priv->left_list->store));
	gtk_tree_view_set_model(priv->right_view,
					GTK_TREE_MODEL(priv->right_list->store));

	GtkCellRenderer *renderer_left, *renderer_right;
	GtkTreeViewColumn *column_left, *column_right;

	renderer_left = gtk_cell_renderer_text_new();
	for(size_t i = 0; i < NUM_COLUMNS; i++){
		column_left = gtk_tree_view_column_new_with_attributes(
						_(ucmd_list_columns[i]),
						renderer_left,
						"text", i,
						NULL);
		gtk_tree_view_column_set_resizable(column_left, 1);
		gtk_tree_view_column_set_sort_column_id(column_left, i);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->left_view),
										column_left);
	}

	renderer_right = gtk_cell_renderer_text_new();
	for(size_t i = 0; i < NUM_COLUMNS; i++){
		column_right = gtk_tree_view_column_new_with_attributes(
						_(ucmd_list_columns[i]),
						renderer_right,
						"text", i,
						NULL);
		gtk_tree_view_column_set_resizable(column_right, 1);
		gtk_tree_view_column_set_sort_column_id(column_right, i);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->right_view),
										column_right);
	}

	g_signal_connect(priv->left_view, "row-activated",
					G_CALLBACK(row_clicked), priv->left_list->store);

	g_signal_connect(priv->right_view, "row-activated",
					G_CALLBACK(row_clicked), priv->right_list->store);
}

/* Create a new window loading a file */
static void ucommander_new_window (GApplication *app, GFile *file) {
	GtkWidget *window;

	GtkBuilder *builder;
	GError* error = NULL;

	UcommanderPrivate *priv = UCOMMANDER_APPLICATION(app)->priv;

	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error)) {
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, app);

	/* Get the window object from the ui file */
	window = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
    if (!window) {
		g_critical ("Widget \"%s\" is missing in file %s.",
			TOP_WINDOW,
			UI_FILE);
    }

	/* ANJUTA: Widgets initialization for ucommander.ui - DO NOT REMOVE */
	ucommander_init_lists(priv);
	ucommander_init_views(priv, builder);

	g_object_unref (builder);

	gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (app));
	if (file != NULL) {
		/* TODO: Add code here to open the file in the new window */
	}

	gtk_widget_show_all (GTK_WIDGET (window));
}


/* GApplication implementation */
static void ucommander_activate (GApplication *application) {
	ucommander_new_window (application, NULL);
}

static void ucommander_open (GApplication  *application,
                     GFile        **files,
                     gint           n_files,
                     const gchar   *hint) {
	gint i;

	for (i = 0; i < n_files; i++)
		ucommander_new_window (application, files[i]);
}

static void ucommander_init (Ucommander *object) {
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE (object,
					UCOMMANDER_TYPE_APPLICATION, UcommanderPrivate);
}

static void ucommander_finalize (GObject *object) {
	G_OBJECT_CLASS (ucommander_parent_class)->finalize (object);
}

static void ucommander_class_init (UcommanderClass *class) {
	G_APPLICATION_CLASS (class)->activate = ucommander_activate;
	G_APPLICATION_CLASS (class)->open = ucommander_open;

	g_type_class_add_private (class, sizeof (UcommanderPrivate));

	G_OBJECT_CLASS (class)->finalize = ucommander_finalize;
}

Ucommander * ucommander_new (void) {
	return g_object_new (ucommander_get_type (),
					"application-id", "org.gnome.ucommander",
					"flags", G_APPLICATION_HANDLES_OPEN,
					NULL);
}
