#include <gtk/gtk.h>
#include "ucmd-app.h"


#include <glib/gi18n.h>


int
main (int argc, char *argv[]){
	Ucommander *app;
	int status;


#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif


	app = ucommander_new ();
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
