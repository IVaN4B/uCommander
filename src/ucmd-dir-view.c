#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "ucmd-dir-list.h"
#include "ucmd-dir-view.h"

int ucmd_dir_view_create(const gchar *path, GtkTreeView *view, GtkLabel *label,
				UcommanderDirView **dir_view){
	g_assert(view != NULL && label != NULL);

	*dir_view = g_new(UcommanderDirView, 1);
	int result = ucmd_dir_list_create(path, &(*dir_view)->list);
	if( result != 0 ){
		return result;
	}

	gtk_label_set_text(label, path);

	gtk_tree_view_set_model(view, GTK_TREE_MODEL((*dir_view)->list->store));

	(*dir_view)->path_label = label;
	(*dir_view)->view = view;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	
	size_t amount = ucmd_dir_list_get_columns_amount();
	UcommanderDirListColumn **columns = (*dir_view)->list->columns;
	for(size_t i = 0; i < amount; i++){
		if( !columns[i]->visible ) continue;
		size_t cindex = columns[i]->position;
		const gchar *name = columns[i]->name;
		column = gtk_tree_view_column_new_with_attributes(
						_(name),
						renderer,
						"text", cindex,
						NULL);
		gtk_tree_view_column_set_resizable(column, 1);
		gtk_tree_view_column_set_sort_column_id(column, cindex);
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	}

	g_signal_connect(view, "row-activated",
				G_CALLBACK(ucmd_dir_view_on_row_activated), (*dir_view));
					
	g_signal_connect(view, "button-press-event",
				G_CALLBACK(ucmd_dir_view_on_button_pressed), (*dir_view));

	g_signal_connect(view, "popup-menu",
				G_CALLBACK(ucmd_dir_view_on_popup_menu), (*dir_view));
	return 0;
}

int ucmd_dir_view_free(UcommanderDirView *dir_view){
	ucmd_dir_list_free(dir_view->list);
	g_free(dir_view);
	return 0;
}

void ucmd_dir_view_show_popup_menu(GtkTreeView *view, GdkEventButton *event,
				gpointer user_data){

	GtkWidget *menu, *menu_item;
	
	menu = gtk_menu_new();

	menu_item = gtk_menu_item_new_with_label("Not yet implemented");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	gtk_widget_show_all(menu);
	
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 
				   (event != NULL) ? event->button : 0,
					gdk_event_get_time((GdkEvent*)event));
}

void ucmd_dir_view_on_row_activated(GtkTreeView *view, GtkTreePath *tree_path,
				GtkTreeViewColumn *column,
				gpointer user_data){
	UcommanderDirList *list;
	UcommanderDirView *dir_view;
	GtkListStore *store;
	gchar *path;
	GtkTreeIter iter;
	gboolean is_dir;
	dir_view = (UcommanderDirView*)user_data;
	list = dir_view->list;
	store = list->store;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, tree_path);
	gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
					ucmd_dir_list_get_columns_amount(), &path,
					ucmd_dir_list_get_columns_amount()+1, &is_dir, -1);

	g_message(path);
	if( is_dir ){
		ucmd_read_dir(path, list);
		gtk_label_set_text(dir_view->path_label, path);
	}

	g_free(path);
}

gboolean ucmd_dir_view_on_button_pressed(GtkTreeView *view, GdkEventButton *event,
				gpointer user_data){
	
	if( event->type == GDK_BUTTON_PRESS && event->button == 3 ){
        GtkTreeSelection *selection;

        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        if (gtk_tree_selection_count_selected_rows(selection) <= 1){
           GtkTreePath *path;

           /* Get tree path for row that was clicked */
           if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(view),
                                             (gint) event->x, 
                                             (gint) event->y,
                                             &path, NULL, NULL, NULL)){
             gtk_tree_selection_unselect_all(selection);
             gtk_tree_selection_select_path(selection, path);
             gtk_tree_path_free(path);
           }
        }
		ucmd_dir_view_show_popup_menu(view, event, user_data);
		return TRUE;
	}
	return FALSE;
}

gboolean ucmd_dir_view_on_popup_menu(GtkTreeView *view, gpointer user_data){
	ucmd_dir_view_show_popup_menu(view, NULL, user_data);
	return TRUE;
}
