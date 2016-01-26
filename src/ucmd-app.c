#include <gtk/gtk.h>
#include "ucmd-app.h"

#include <glib/gi18n.h>
#include "ucmd-dir-list.h"
#include "ucmd-dir-view.h"
G_DEFINE_TYPE(Ucommander, ucmd, GTK_TYPE_APPLICATION);

struct _UcommanderPrivate{
	UcommanderDirView *left_view;
	UcommanderDirView *right_view;
};

static void activate_action(GSimpleAction *action,
							GVariant *parameter,
							gpointer window){
	const gchar *name = g_action_get_name(G_ACTION(action));
	g_message(name);
}

static void activate_quit(GSimpleAction *action,
					      GVariant *parameter,
						  gpointer user_data){
	GtkApplication *app = user_data;
	GtkWidget *win;
	GList *list, *next;

	list = gtk_application_get_windows(app);
	while(list){
		win = list->data;
		next = list->next;

		gtk_widget_destroy(GTK_WIDGET(win));

		list = next;
	}
}

static void activate_about(GSimpleAction *action,
						   GVariant *parameter,
						   gpointer user_data){
	GtkApplication *app = (GtkApplication*)user_data;
	GtkWindow *window = gtk_application_get_active_window(app);
	const gchar *author[] = {
		"Ivan Chebykin",
		NULL
	};

	gtk_show_about_dialog(GTK_WINDOW(window),
						  "program-name", APP_NAME,
						  "version", APP_VERSION,
						  "copyright", "(C) 2016 Ivan Chebykin",
						  "license-type", GTK_LICENSE_GPL_3_0,
						  "website", "http://ivan4b.ru",
						  "comments", "Simple and efficient file manager",
						  "authors", author,
						  "title", "About", NULL);
}

static GActionEntry app_entries[] = {
	{ "view", activate_action, NULL, NULL, NULL },
	{ "edit", activate_action, NULL, NULL, NULL },
	{ "copy", activate_action, NULL, NULL, NULL },
	{ "move", activate_action, NULL, NULL, NULL },
	{ "mkdir", activate_action, NULL, NULL, NULL },
	{ "term", activate_action, NULL, NULL, NULL },
	{ "remove", activate_action, NULL, NULL, NULL },
	{ "exit", activate_quit, NULL, NULL, NULL },
	{ "about", activate_about, NULL, NULL, NULL },
	{ "history-go-back", activate_action, NULL, NULL, NULL },
	{ "history-go-forward", activate_action, NULL, NULL, NULL }
};

static void ucmd_init_list(UcommanderPrivate *priv, GtkBuilder *builder){

} 

static void ucmd_init_view(UcommanderPrivate *priv, GtkBuilder *builder){

}

/* Create a new window loading from UI file */
static void ucmd_new_window(GApplication *app, GFile *file){
	GtkWidget *window;
	GtkBuilder *builder;
	GError *error = NULL;

	UcommanderPrivate *priv = UCOMMANDER_APPLICATION(app)->priv;

	/* Load UI from file */
	builder = gtk_builder_new();
	if( !gtk_builder_add_from_file(builder, UI_FILE, &error) ){
		g_critical("Couldn't load builder file: %s", error->message);
		g_error_free(error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals(builder, app);

	/* Get the window object from UI file */
	window = GTK_WIDGET(gtk_builder_get_object(builder, TOP_WINDOW));
	if( !window ){
		g_critical("Widget \"%s\" is missing in file %s.",
					TOP_WINDOW,
					UI_FILE);
	}
	g_object_unref(builder);

	gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (app));
	g_action_map_add_action_entries(G_ACTION_MAP(app),
									app_entries,
									G_N_ELEMENTS(app_entries),
									app);
	gtk_widget_show_all(GTK_WIDGET(window));
}

static void ucmd_activate(GApplication *app){
	ucmd_new_window(app, NULL);
}

static void ucmd_open(GApplication *app,
				GFile **files,
				gint n_files,
				const gchar *hint){
	for(gint i = 0; i < n_files; i++){
		ucmd_new_window(app, files[i]);
	}
}

static void ucmd_init(Ucommander *object){
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,
					UCOMMANDER_TYPE_APPLICATION, UcommanderPrivate);
}

static void ucmd_finalize(GObject *object){
	G_OBJECT_CLASS(ucmd_parent_class)->finalize(object);
}

static void ucmd_class_init(UcommanderClass *cclass){
	G_APPLICATION_CLASS(cclass)->activate = ucmd_activate;
	G_APPLICATION_CLASS(cclass)->open = ucmd_open;

	g_type_class_add_private(cclass, sizeof(UcommanderPrivate));

	G_OBJECT_CLASS(cclass)->finalize = ucmd_finalize;
}

Ucommander *ucmd_new(void){
	return g_object_new(ucmd_get_type(),
					"application-id", "org.ivan4b.ucommander",
					"flags", G_APPLICATION_HANDLES_OPEN,
					NULL);
}
