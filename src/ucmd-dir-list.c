#define _DEFAULT_SOURCE
#include <gtk/gtk.h>
#include <glib/gi18n.h>
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
		GStatBuf *buff = g_new(GStatBuf, 1);
		cur_path = g_build_filename(path, name, NULL);
		is_dir = g_file_test(cur_path, G_FILE_TEST_IS_DIR);

		display_name = g_filename_to_utf8(name, -1, NULL, NULL, NULL);

		int result = g_lstat(cur_path, buff);
		size_t buff_size = 255;
		GDate *f_date = g_date_new();

		gchar size_label[buff_size],
			  mode_label[buff_size],
			  date_label[buff_size],
			  *type_label = "";

		if( result < 0 ){
			/* TODO: Default values */
			g_error("Stat failed");
		}else{
			g_snprintf(size_label, buff_size, "%d", buff->st_size);
			mode_label[0] = (S_ISDIR(buff->st_mode)) ? 'd' : '-';
			mode_label[1] = (buff->st_mode & S_IRUSR) ? 'r' : '-';
			mode_label[2] = (buff->st_mode & S_IWUSR) ? 'w' : '-';
			mode_label[3] = (buff->st_mode & S_IXUSR) ? 'x' : '-';
			mode_label[4] = (buff->st_mode & S_IRGRP) ? 'r' : '-';
			mode_label[5] = (buff->st_mode & S_IWGRP) ? 'w' : '-';
			mode_label[6] = (buff->st_mode & S_IXGRP) ? 'x' : '-';
			mode_label[7] = (buff->st_mode & S_IROTH) ? 'r' : '-';
			mode_label[8] = (buff->st_mode & S_IWOTH) ? 'w' : '-';
			mode_label[9] = (buff->st_mode & S_IXOTH) ? 'x' : '-';
			mode_label[10] = '\0';
			/*g_snprintf(mode_label, buff_size, "%o", buff->st_mode);*/

			g_date_set_time_t (f_date, buff->st_mtime);
			g_date_strftime(date_label, buff_size, "%F", f_date);
			if( !is_dir ){
				type_label = g_utf8_strchr(display_name, buff_size, '.');
				if( display_name[0] == '.' ){
					gboolean has_ext = FALSE;
					gint i = 1;
					while( !has_ext &&
						   i < g_utf8_strlen(type_label, buff_size) ){
						has_ext = (type_label[i] == '.');
						i++;
					}
					if( has_ext ){
						*type_label++;
						type_label = g_utf8_strchr(type_label, buff_size, '.');
					}else{
						type_label = "";
					}
				}
			}
			/*g_snprintf(date_label, buff_size, "%o", buff->st_mtime);*/
		}
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
						PATH_COLUMN, cur_path,
						NAME_COLUMN, display_name,
						TYPE_COLUMN, is_dir ? "" : type_label,
						SIZE_COLUMN, is_dir ? _("<DIR>") : size_label,
						DATE_COLUMN, date_label,
						ATTR_COLUMN, mode_label,
						ISDIR_COLUMN, is_dir,
						-1);
		g_free(cur_path);
		g_free(display_name);
		g_free(buff);
		g_free(f_date);
		name = g_dir_read_name(dir);
	}
	g_dir_close(dir);


	return 0;
}


static gint ucmd_dirs_sort_func (GtkTreeModel *model,
           GtkTreeIter  *a,
           GtkTreeIter  *b,
           gpointer      user_data){
	gboolean is_dir_a, is_dir_b;
	gchar *name_a, *name_b;
	int ret;

	/* We need this function because we want to sort
	* folders before files.
	*/

	gtk_tree_model_get (model, a,
                      ISDIR_COLUMN, &is_dir_a,
                      NAME_COLUMN, &name_a,
                      -1);

	gtk_tree_model_get (model, b,
                      ISDIR_COLUMN, &is_dir_b,
                      NAME_COLUMN, &name_b,
                      -1);

	if (!is_dir_a && is_dir_b)
		ret = 1;
	else if (is_dir_a && !is_dir_b)
		ret = -1;
	else{
		ret = g_utf8_collate (name_a, name_b);
	}

	g_free (name_a);
	g_free (name_b);

	return ret;
}


int ucmd_create_dir_list(const gchar *path, UcommanderDirList **list){
	*list = g_new(UcommanderDirList, 1);
	if( *list == NULL ){
		return ECREATELIST;
	}

	(*list)->path = path;

	(*list)->store = gtk_list_store_new(NUM_COLUMNS,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_BOOLEAN);
	/* Set sort column and function */
	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE ((*list)->store),
                                           ucmd_dirs_sort_func,
                                           NULL, NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE ((*list)->store),
                                        GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                        GTK_SORT_ASCENDING);

	int result = ucmd_read_dir(path, (*list)->store);
	return result;
}

void ucmd_free_dir_list(UcommanderDirList *list){
	assert(list != NULL && list->store != NULL);

	g_free(list->store);
	g_free(list);
}
