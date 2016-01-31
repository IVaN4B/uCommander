#pragma once
#define UCMD_ACTIONS_AMOUNT 2
typedef struct{
	gchar *action;
	GtkTreeSelectionForeachFunc callback;
} ucmd_dir_view_action_callback_t;

typedef struct _UcommanderDirView{
	UcommanderDirList *list;
	GtkTreeView *view;
	GtkLabel *path_label;
	GtkWindow *window;
} UcommanderDirView;

int ucmd_dir_view_create(const gchar *path, GtkTreeView *view, GtkLabel *label,
				GtkWindow *window, UcommanderDirView **dir_view);

int ucmd_dir_view_free(UcommanderDirView *dir_view);

void ucmd_dir_view_show_popup_menu(GtkTreeView *view, GdkEventButton *event,
				gpointer user_data);

void ucmd_dir_view_on_row_activated(GtkTreeView *view, GtkTreePath *tree_path,
				GtkTreeViewColumn *column,
				gpointer user_data);

gboolean ucmd_dir_view_on_button_pressed(GtkTreeView *view, GdkEventButton *event,
				gpointer user_data);

gboolean ucmd_dir_view_on_popup_menu(GtkTreeView *view, gpointer user_data);

void ucmd_dir_view_on_action(GSimpleAction *action, GVariant *param,
				gpointer user_data);
