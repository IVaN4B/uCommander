#include "ucmd-app.h"

#include <glib/gi18n.h>

/* For testing propose use the local (not installed) ui file */
#define UI_FILE "src/ui/main_window.ui"
#define TOP_WINDOW "main_window"


G_DEFINE_TYPE (Ucommander, ucommander, GTK_TYPE_APPLICATION);

/* ANJUTA: Macro UCOMMANDER_APPLICATION gets Ucommander - DO NOT REMOVE */
struct _UcommanderPrivate {
	/* ANJUTA: Widgets declaration for ucommander.ui - DO NOT REMOVE */
};

/* Create a new window loading a file */
static void ucommander_new_window (GApplication *app, GFile *file) {
	GtkWidget *window;

	GtkBuilder *builder;
	GError* error = NULL;

	UcommanderPrivate *priv = UCOMMANDER_APPLICATION(app)->priv;

	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
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
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE (object, UCOMMANDER_TYPE_APPLICATION, UcommanderPrivate);
}

static void ucommander_finalize (GObject *object) {
	G_OBJECT_CLASS (ucommander_parent_class)->finalize (object);
}

static void ucommander_class_init (UcommanderClass *klass) {
	G_APPLICATION_CLASS (klass)->activate = ucommander_activate;
	G_APPLICATION_CLASS (klass)->open = ucommander_open;

	g_type_class_add_private (klass, sizeof (UcommanderPrivate));

	G_OBJECT_CLASS (klass)->finalize = ucommander_finalize;
}

Ucommander * ucommander_new (void) {
	return g_object_new (ucommander_get_type (),
	                     "application-id", "org.gnome.ucommander",
	                     "flags", G_APPLICATION_HANDLES_OPEN,
	                     NULL);
}
