#include <gtk/gtk.h>

int main(int argc,char **argv){
	GtkWidget  *window;
	GtkBuilder *builder;
	GError     *error = NULL;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();

	if( !gtk_builder_add_from_file(builder, "../src/ui/main_window.ui", &error) ){
		g_print("Unable to load UI file\n");
		g_print("Error: %s\n", error->message);
		g_free(error);
		return 1;
	}
	window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(G_OBJECT(builder));

	gtk_widget_show(window);

	gtk_main();

	return 0;
}
