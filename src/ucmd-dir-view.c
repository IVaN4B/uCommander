#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "ucmd-dir-list.h"
#include "ucmd-dir-view.h"

static void launch_associated_app(GtkTreeModel *model, GtkTreePath *tree_path,
				GtkTreeIter *iter, gpointer data);
static void start_copy(GtkTreeModel *model, GtkTreePath *tree_path,
				GtkTreeIter *iter, gpointer data);

static ucmd_dir_view_action_callback_t ucmd_actions[UCMD_ACTIONS_AMOUNT] = {
	{ "view", &launch_associated_app },
	{ "copy", &start_copy }
};
int ucmd_dir_view_create(const gchar *path, GtkTreeView *view, GtkLabel *label,
				GtkWindow *window, UcommanderDirView **dir_view){
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
	(*dir_view)->window = window;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	
	size_t amount = ucmd_dir_list_get_columns_amount();
	UcommanderDirListColumn *name_col = ucmd_dir_list_get_name_column();
	size_t name_pos = name_col->position;
	UcommanderDirListColumn **columns = (*dir_view)->list->columns;
	for(size_t i = 0; i < amount; i++){
		if( !columns[i]->visible ) continue;
		size_t cindex = columns[i]->position;
		const gchar *name = columns[i]->name;
		column = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(column, _(name));
		if( cindex == name_pos ){
			GtkCellRenderer *icon_renderer = gtk_cell_renderer_pixbuf_new();
			gtk_tree_view_column_pack_start(column, icon_renderer, FALSE);
			gtk_tree_view_column_add_attribute(column, icon_renderer,
							"icon-name", cindex-1); 
		}
		gtk_tree_view_column_pack_end(column, renderer, FALSE);
		gtk_tree_view_column_add_attribute(column, renderer, "text", cindex);
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

static void on_open_item_activated(GtkMenuItem *item, gpointer user_data){
	GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(
						GTK_TREE_VIEW(tree_view));

	gtk_tree_selection_selected_foreach(sel,
								ucmd_actions[0].callback, NULL);
}

void ucmd_dir_view_show_popup_menu(GtkTreeView *view, GdkEventButton *event,
				gpointer user_data){

	GtkWidget *menu, *menu_item;
	
	menu = gtk_menu_new();

	menu_item = gtk_menu_item_new_with_label(_("Open"));
	
	g_signal_connect(menu_item, "activate", G_CALLBACK(on_open_item_activated), view);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

	gtk_widget_show_all(menu);
	
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 
				   (event != NULL) ? event->button : 0,
					gdk_event_get_time((GdkEvent*)event));
}

static void launch_associated_app(GtkTreeModel *model, GtkTreePath *tree_path,
				GtkTreeIter *iter, gpointer data){
	gchar *path;
	gboolean is_dir;
	if( data != NULL ){
		path = (gchar*)data;
		is_dir = FALSE;
	}else{
		gtk_tree_model_get(model, iter,
					ucmd_dir_list_get_columns_amount(), &path,
					ucmd_dir_list_get_columns_amount()+1, &is_dir, -1);
	}
	
	if( is_dir ) return;

	gchar *content_type = g_content_type_guess(path, NULL, 0, NULL);
	if( content_type != NULL ){
		GAppInfo *app_info = g_app_info_get_default_for_type(content_type,
						FALSE);
		if( app_info == NULL ){
			/* TODO: find recommended or show dialog */
		}else{
			GList *files = NULL;
			GError *error = NULL;
			GFile *arg = g_file_new_for_path(path);
			files = g_list_append(files, arg);
			g_app_info_launch(app_info, files, NULL, &error);
			if( error ){
				g_critical("Unable to launch application: %s",
								error->message);
				g_error_free(error);
			}
		}
		g_free(content_type);
	}else{
			
	}
}

static void start_copy(GtkTreeModel *model, GtkTreePath *tree_path,
				GtkTreeIter *iter, gpointer data){
	g_message("copy");
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
	}else{
		launch_associated_app(GTK_TREE_MODEL(store), tree_path, &iter, path);
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

static GtkTreeView *get_focused_tree_view(GtkApplication *app){
	g_assert(app != NULL);
	GtkWindow *window = gtk_application_get_active_window(app);
	if( window == NULL ) return NULL;
	GtkWidget *focused = gtk_window_get_focus(window);
	if( focused != NULL && GTK_IS_TREE_VIEW(focused) ){
		return GTK_TREE_VIEW(focused);
	}
	return NULL;
}

void ucmd_dir_view_on_action(GSimpleAction *action, GVariant *param,
				gpointer user_data){
	GtkTreeView *tree_view = get_focused_tree_view(GTK_APPLICATION(user_data));
	if( tree_view != NULL ){
		GtkTreeSelection *sel = gtk_tree_view_get_selection(
						GTK_TREE_VIEW(tree_view));
		const gchar *action_name = g_action_get_name(G_ACTION(action));
		for(int i = 0; i < UCMD_ACTIONS_AMOUNT; i++){
			if( strcmp(ucmd_actions[i].action, action_name) == 0 ){
				gtk_tree_selection_selected_foreach(sel,
								ucmd_actions[i].callback, NULL);
				break;
			}
		}
	}
}
