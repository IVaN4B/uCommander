#define _DEFAULT_SOURCE
#define BUFF_SIZE 1024
#define SYS_COL_AMOUNT 2
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <assert.h>
#include <string.h>
#include "ucmd-dir-list.h"
static size_t ucmd_list_columns_amount = 0;
static size_t ucmd_list_column_name_index = 0;
static UcommanderDirListColumn **ucmd_list_columns;
UcommanderDirListColumn *ucmd_dir_list_get_column(size_t index){
	if( index < 0 && index >= ucmd_list_columns_amount ) return NULL;

	return ucmd_list_columns[index];
}

UcommanderDirListColumn *ucmd_dir_list_get_name_column(){
	return ucmd_list_columns[ucmd_list_column_name_index];
}

size_t ucmd_dir_list_get_columns_amount(){
	return ucmd_list_columns_amount;
}

/* Get columns list from settings */
void ucmd_dir_list_load_columns(){

	/* TODO: Really get these from GSettings */
	ucmd_list_columns_amount = 6;
	ucmd_list_columns = g_malloc(sizeof(UcommanderDirListColumn)*
								 ucmd_list_columns_amount);

	for(int i = 0; i < ucmd_list_columns_amount; i++){
		ucmd_list_columns[i] = g_new(UcommanderDirListColumn, 1);
		ucmd_list_columns[i]->position = i;
		ucmd_list_columns[i]->visible = 1;
	}
	ucmd_list_columns[5]->visible = 0;

	ucmd_list_column_name_index = 0;
	ucmd_list_columns[0]->name = "Name";
	ucmd_list_columns[1]->name = "Type";
	ucmd_list_columns[2]->name = "Size";
	ucmd_list_columns[3]->name = "Date";
	ucmd_list_columns[4]->name = "Attributes";
	ucmd_list_columns[5]->name = "Path";

	ucmd_list_columns[0]->get_info = &ucmd_column_get_info_name;
	ucmd_list_columns[1]->get_info = &ucmd_column_get_info_ext;
	ucmd_list_columns[2]->get_info = &ucmd_column_get_info_size;
	ucmd_list_columns[3]->get_info = &ucmd_column_get_info_mtime;
	ucmd_list_columns[4]->get_info = &ucmd_column_get_info_mode;
	ucmd_list_columns[5]->get_info = &ucmd_column_get_info_path;
}

/* Read dir from list */
int ucmd_read_dir(const gchar *path, const UcommanderDirList *list){
	g_assert(list != NULL && list->store != NULL);
	GtkTreeIter iter;
	gtk_list_store_clear(list->store);

	/* If path is not root, then append item to get to the parent
	 * directory */
	if( strcmp(path, "/") != 0 ){
		gchar *parent_path = g_path_get_dirname(path);
		gtk_list_store_append(list->store, &iter);
		gtk_list_store_set(list->store, &iter,
							ucmd_dir_list_get_name_column()->position, "..",
							ucmd_list_columns_amount, parent_path,
							ucmd_list_columns_amount+1, TRUE, -1);

		g_free(parent_path);
	}
	/*GCancellable *cancellable;*/
	GError *error = NULL;
	GFile *dir = g_file_new_for_path(path);
	GFileEnumerator *dirent = g_file_enumerate_children(dir,
										"*",
										G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
										NULL,
										&error);
	gchar **column_data = g_malloc(sizeof(gchar*)*ucmd_list_columns_amount);
	while( TRUE ){
		GFileInfo *info;
		gboolean res = g_file_enumerator_iterate(dirent,
												 &info,
												 NULL,
												 NULL,
												 &error);
		if( !res ) break;
		if( !info) break;
		gtk_list_store_append(list->store, &iter);
		for(int k = 0; k < ucmd_list_columns_amount; k++){

			if( list->columns[k]->visible || list->columns[k]->always_process ){
				column_data[k] = g_malloc(BUFF_SIZE);
				int info_result = list->columns[k]->get_info(info,
													&column_data[k]);
				if( info_result != 0 ){
					column_data[k] = "None";
				}
				GValue value = G_VALUE_INIT;
				g_value_init(&value, G_TYPE_STRING);
				g_value_set_string(&value, column_data[k]);
				gtk_list_store_set_value(list->store, &iter, k, &value);
				/*g_free(column_data[k]);*/
			}

		}
		gchar *name = (gchar*)g_file_info_get_attribute_byte_string(info,
					G_FILE_ATTRIBUTE_STANDARD_NAME);
		gchar *cur_path = g_build_filename(path, name, NULL);
		gboolean is_dir = g_file_test(cur_path, G_FILE_TEST_IS_DIR);
		gtk_list_store_set(list->store, &iter,
								 ucmd_list_columns_amount, cur_path,
								 ucmd_list_columns_amount+1, is_dir, -1);
		g_free(cur_path);

	}
	g_free(column_data);
	g_object_unref(dir);

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
                      ucmd_list_columns_amount+1, &is_dir_a,
                      ucmd_list_column_name_index, &name_a,
                      -1);

	gtk_tree_model_get (model, b,
                      ucmd_list_columns_amount+1, &is_dir_b,
                      ucmd_list_column_name_index, &name_b,
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


int ucmd_dir_list_create(const gchar *path, UcommanderDirList **list){
	*list = g_new(UcommanderDirList, 1);
	if( *list == NULL ){
		return ECREATELIST;
	}

	(*list)->path = path;

	if( ucmd_list_columns_amount == 0 ){
		ucmd_dir_list_load_columns();
	}

	(*list)->store = gtk_list_store_new(ucmd_list_columns_amount+SYS_COL_AMOUNT,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_STRING,
										G_TYPE_BOOLEAN);
	/* Set sort column and function */
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE ((*list)->store),
                                           ucmd_dirs_sort_func,
                                           NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE ((*list)->store),
                                     GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                     GTK_SORT_ASCENDING);


	(*list)->columns = ucmd_list_columns;
	int result = ucmd_read_dir(path, *list);
	return result;
}

void ucmd_dir_list_free(UcommanderDirList *list){
	assert(list != NULL && list->store != NULL);

	g_free(list->store);
	g_free(list);
}

int ucmd_column_get_info_name(GFileInfo *info, gchar **output){
	gchar *name = (gchar*)g_file_info_get_attribute_byte_string(info,
					G_FILE_ATTRIBUTE_STANDARD_NAME);
	if( name == NULL ){
		return EGETINFO;
	}
	name = g_filename_to_utf8(name, -1, NULL, NULL, NULL);
	*output = name;
	return 0;
}

int ucmd_column_get_info_ext(GFileInfo *info, gchar **output){
	GFileType type = g_file_info_get_attribute_uint32(info,
					G_FILE_ATTRIBUTE_STANDARD_TYPE);
	if( type == G_FILE_TYPE_REGULAR ){
		gchar *name = (gchar*)g_file_info_get_attribute_byte_string(info,
					G_FILE_ATTRIBUTE_STANDARD_NAME);
		if( name == NULL ){
			return EGETINFO;
		}

		name = g_filename_to_utf8(name, -1, NULL, NULL, NULL);
		gchar *type = g_utf8_strrchr(name, BUFF_SIZE, '.');
		*output = type;
	}else{
		*output = "";
	}
	return 0;
}

int ucmd_column_get_info_type(GFileInfo *info, gchar **output){
	gchar *name = (gchar*)g_file_info_get_attribute_byte_string(info,
					G_FILE_ATTRIBUTE_STANDARD_NAME);
	if( name == NULL ){
		return EGETINFO;
	}
	*output = name;
	return 0;
}

int ucmd_column_get_info_size(GFileInfo *info, gchar **output){
	GFileType type = g_file_info_get_attribute_uint32(info,
					G_FILE_ATTRIBUTE_STANDARD_TYPE);
	if( type != G_FILE_TYPE_DIRECTORY ){
		guint64 size = g_file_info_get_attribute_uint64(info,
						G_FILE_ATTRIBUTE_STANDARD_SIZE);
		g_snprintf(*output, BUFF_SIZE, "%ld", size);
	}else{
		*output = "<DIR>";
	}
	return 0;
}

int ucmd_column_get_info_mtime(GFileInfo *info, gchar **output){
	guint64 mtime = g_file_info_get_attribute_uint64(info,
					G_FILE_ATTRIBUTE_TIME_MODIFIED);
	GDate *f_date = g_date_new();
	g_date_set_time_t(f_date, mtime);
	g_date_strftime(*output, BUFF_SIZE, "%F", f_date);
	g_free(f_date);
	return 0;
}

int ucmd_column_get_info_mode(GFileInfo *info, gchar **output){
	mode_t mode = g_file_info_get_attribute_uint32(info,
					G_FILE_ATTRIBUTE_UNIX_MODE);
	gchar mode_label[BUFF_SIZE];
	mode_label[0] = (S_ISDIR(mode)) ? 'd' : '-';
	mode_label[1] = (mode & S_IRUSR) ? 'r' : '-';
	mode_label[2] = (mode & S_IWUSR) ? 'w' : '-';
	mode_label[3] = (mode & S_IXUSR) ? 'x' : '-';
	mode_label[4] = (mode & S_IRGRP) ? 'r' : '-';
	mode_label[5] = (mode & S_IWGRP) ? 'w' : '-';
	mode_label[6] = (mode & S_IXGRP) ? 'x' : '-';
	mode_label[7] = (mode & S_IROTH) ? 'r' : '-';
	mode_label[8] = (mode & S_IWOTH) ? 'w' : '-';
	mode_label[9] = (mode & S_IXOTH) ? 'x' : '-';
	mode_label[10] = '\0';
	*output = mode_label;
	return 0;
}

int ucmd_column_get_info_path(GFileInfo *info, gchar **output){
	*output = "NYI";
	return 0;
}
