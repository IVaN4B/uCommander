#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <assert.h>
#include <string.h>
#include "ucmd-dir-list.h"

static gchar *ucmd_list_columns[NUM_COLUMNS] = {
	"Path",
	"Name",
	"Type",
	"Size",
	"Date",
	"Attributes",
	"Is dir"
};

gchar *ucmd_dir_list_get_column_name(size_t index){
	if( index < 0 && index >= NUM_COLUMNS ) return NULL;

	return ucmd_list_columns[index];
}

/* Get columns list from settings */
void ucmd_dir_list_get_visible_columns(size_t **columns, size_t *amount){

	/* TODO: Really get these from GSettings */	
	size_t cur_amount = 5;
	size_t *cur_columns = g_malloc(sizeof(size_t)*cur_amount);
	cur_columns[0] = NAME_COLUMN;
	cur_columns[1] = TYPE_COLUMN;
	cur_columns[2] = SIZE_COLUMN;
	cur_columns[3] = DATE_COLUMN;
	cur_columns[4] = ATTR_COLUMN;
	*columns = cur_columns;
	*amount = cur_amount;
}

/* Read dir from list */
int ucmd_read_dir(const gchar *path, GtkListStore *store){
	assert(store != NULL);
	GDir *dir;
	const gchar *name;
	GtkTreeIter iter;
	gtk_list_store_clear(store);

	/* If path is not root, then append item to get to the parent
	 * directory */
	if( strcmp(path, "/") != 0 ){
		gchar *parent_path = g_path_get_dirname(path);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, PATH_COLUMN, parent_path,
									NAME_COLUMN, "..",
									ISDIR_COLUMN, TRUE, -1);

		g_free(parent_path);
	}
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
