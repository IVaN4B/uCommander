#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <assert.h>
#include "ucmd-dir-list.h"

/* Get columns list from settings */
void ucmd_get_dir_list_columns(gchar **columns, size_t amount);

/* Read dir from list */
int ucmd_read_dir(const gchar *path, GtkListStore *store){
	assert(store != NULL);
	GDir *dir;
	const gchar *name;
	GtkTreeIter iter;

	gtk_list_store_clear(store);

	dir = g_dir_open(path, 0, NULL);
	if( !dir ){
		return EOPENDIR;
	}

	name = g_dir_read_name(dir);
	while(name != NULL){
		gchar *cur_path, *display_name;
		gboolean is_dir;
		GStatBuf *buf;
		cur_path = g_build_filename(path, name, NULL);

		is_dir = g_file_test(cur_path, G_FILE_TEST_IS_DIR);

		display_name = g_filename_to_utf8(name, -1, NULL, NULL, NULL);
		int result = g_lstat(name, buf);
		off_t file_size;
		mode_t file_mode;
		gchar *size_label = "0";
		gchar *attr_label = "";
		if( result < 0 ){
			/* TODO: Default values */
		}else{
			file_size = buf->st_size;
			file_mode = buf->st_mode;
			g_sprintf(size_label, "%d", file_size);
			g_sprintf(attr_label, "%d", file_mode);
		}
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
						PATH_COLUMN, cur_path,
						NAME_COLUMN, display_name,
						TYPE_COLUMN, "NYI",
						SIZE_COLUMN, size_label,
						DATE_COLUMN, "NYI",
						ATTR_COLUMN, attr_label,
						ISDIR_COLUMN, is_dir,
						-1);
		g_free(cur_path);
		g_free(display_name);
		name = g_dir_read_name(dir);
	}
	g_dir_close(dir);

	return 0;
}

int ucmd_create_dir_list(const gchar *path, UcommanderDirList **list){
	*list = g_new(UcommanderDirList, 1);
	if( *list == NULL ){
		return ECREATELIST;
	}

	(*list)->store = gtk_list_store_new(NUM_COLUMNS,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_BOOLEAN);
	int result = ucmd_read_dir(path, (*list)->store);
	return result;
}

void ucmd_free_dir_list(UcommanderDirList *list){
	assert(list != NULL && list->store != NULL);

	g_free(list->store);
	g_free(list);
}
