#define _DEFAULT_SOURCE
#define BUFF_SIZE 1024
#define BLOCK_SIZE 128
#define MAX_JOBS 10
#define SYS_COL_AMOUNT 2
#define DEFAULT_COLUMNS_AMOUNT 8
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <assert.h>
#include <string.h>
#include "ucmd-dir-list.h"
static size_t ucmd_list_columns_amount = 0;
static size_t ucmd_list_column_name_index = 1;
static size_t async_jobs_amount = 0;
static UcommanderDirListColumn default_columns[DEFAULT_COLUMNS_AMOUNT] = {
	{"Icon", 0, 0, FALSE, TRUE, TRUE, &ucmd_column_get_info_icon},
	{"Name", 0, 1, TRUE, TRUE, TRUE, &ucmd_column_get_info_name},
	{"Type", 0, 2, TRUE, TRUE, TRUE, &ucmd_column_get_info_ext},
	{"Size", 0, 3, TRUE, TRUE, TRUE, &ucmd_column_get_info_size},
	{"Date", 0, 4, TRUE, TRUE, TRUE, &ucmd_column_get_info_mtime},
	{"Attributes", 0, 5, TRUE, TRUE, TRUE, &ucmd_column_get_info_mode},
	{"Path", 0, 6, FALSE, FALSE, TRUE, &ucmd_column_get_info_path},
	{"MIME Type", 0, 7, TRUE, TRUE, TRUE, &ucmd_column_get_info_type}
};
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
	ucmd_list_columns_amount = DEFAULT_COLUMNS_AMOUNT;
	ucmd_list_columns = g_malloc(sizeof(UcommanderDirListColumn)*
								 ucmd_list_columns_amount);
	for(size_t i = 0; i < DEFAULT_COLUMNS_AMOUNT; i++){
		ucmd_list_columns[i] = &default_columns[i];
	}
}

static void ucmd_dir_list_add_file_callback(GObject *direnum,
				GAsyncResult *result,
				gpointer user_data){
	g_return_if_fail(async_jobs_amount < MAX_JOBS);
	async_jobs_amount++;

	GError *error = NULL;
	GList *file_list = g_file_enumerator_next_files_finish(
					G_FILE_ENUMERATOR(direnum),
					result, &error);
	UcommanderDirList *list = (UcommanderDirList*)user_data;
	if( error ){
		async_jobs_amount--;
		if( error->code != G_IO_ERROR_CANCELLED ){
			g_critical("Unable to add files to list, error: %s", error->message);
		}
		g_object_unref(direnum);
		g_error_free(error);
		return;
	}else if( file_list == NULL ){
		async_jobs_amount--;
		g_object_unref(direnum);
		g_object_unref(list->cancellable);
		list->cancellable = NULL;
		return;
	}else{

		GList *node = file_list;
		GFileInfo *info;
		GtkTreeIter iter;
		while(node){
			info = node->data;
			node = node->next;

			gtk_list_store_append(list->store, &iter);
			size_t amount = ucmd_dir_list_get_columns_amount();
			for(int k = 0; k < amount; k++){

				if( list->columns[k]->visible
							   	|| list->columns[k]->always_process ){
					gchar *column_data;
					int info_result = list->columns[k]->get_info(info,
														&column_data);
					if( info_result != 0 ){
						column_data = "None";
					}
					GValue value = G_VALUE_INIT;
					g_value_init(&value, G_TYPE_STRING);
					g_value_set_string(&value, column_data);
					gtk_list_store_set_value(list->store, &iter, k, &value);
					g_free(column_data);
				}

			}
			gchar *name = (gchar*)g_file_info_get_attribute_byte_string(info,
						G_FILE_ATTRIBUTE_STANDARD_NAME);
			gchar *cur_path = g_build_filename(list->path, name, NULL);
			gboolean is_dir = g_file_test(cur_path, G_FILE_TEST_IS_DIR);
			gtk_list_store_set(list->store, &iter,
									 ucmd_list_columns_amount, cur_path,
									 ucmd_list_columns_amount+1, is_dir, -1);
			g_free(cur_path);
			g_object_unref(info);
		}
		g_file_enumerator_next_files_async(G_FILE_ENUMERATOR(direnum),
						BLOCK_SIZE,
						G_PRIORITY_LOW,
						list->cancellable,
						ucmd_dir_list_add_file_callback,
						list);
	}
	g_list_free(file_list);
	async_jobs_amount--;
}

static void ucmd_dir_list_enum_files_callback(GObject *dir,
				GAsyncResult *result,
				gpointer user_data){
	GError *error = NULL;

	UcommanderDirList *list = (UcommanderDirList*)user_data;

	GFileEnumerator *direnum = g_file_enumerate_children_finish(G_FILE(dir),
				result, &error);
	if( error ){
		g_critical("Dir enum error: %s", error->message);
		g_error_free(error);
		return;
	}
	
	g_file_enumerator_next_files_async(direnum,
						BLOCK_SIZE,
						G_PRIORITY_LOW,
						list->cancellable,
						ucmd_dir_list_add_file_callback,
						list);
}

/* Read dir from list */
int ucmd_read_dir(const gchar *path, UcommanderDirList *list){
	g_assert(list != NULL && list->store != NULL);
	GtkTreeIter iter;
	gtk_list_store_clear(list->store);
	if( list->path != NULL ){
		g_free((gchar*)list->path);
	}
	list->path = g_strdup(path);
	
	/* Stop previous job before running next one */
	if( list->cancellable != NULL ){
		g_cancellable_cancel(list->cancellable);

	}
	list->cancellable = g_cancellable_new();

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
	GFile *dir = g_file_new_for_path(path);
	g_file_enumerate_children_async(dir,
							"*",
							G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
							G_PRIORITY_DEFAULT,
							list->cancellable,
							ucmd_dir_list_enum_files_callback,
							list);
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
		/* If we met last row */
		if( name_b != NULL ){
			ret = g_utf8_collate (name_a, name_b);
		}else{
			ret = 1;
		}
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

	(*list)->path = NULL;
	(*list)->cancellable = NULL;

	if( ucmd_list_columns_amount == 0 ){
		ucmd_dir_list_load_columns();
	}
	size_t types_amount = ucmd_list_columns_amount+SYS_COL_AMOUNT;
	GType types[types_amount];
	for(size_t i = 0; i < types_amount-1; i++){
		types[i] = G_TYPE_STRING;
	}

	/* Is dir flag */
	types[types_amount-1] = G_TYPE_BOOLEAN;
	(*list)->store = gtk_list_store_newv(types_amount, types);
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
	assert(list != NULL && list->store != NULL && list->path != NULL);

	g_free(list->store);
	g_free((gchar*)list->path);
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
		*output = g_strdup(type);
		g_free(name);
	}else{
		*output = g_strdup("");
	}
	return 0;
}

int ucmd_column_get_info_type(GFileInfo *info, gchar **output){
	gchar *type = (gchar*)g_file_info_get_attribute_string(info,
					G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
	if( type == NULL ){
		return EGETINFO;
	}
	*output = g_strdup(type);
	return 0;
}

int ucmd_column_get_info_size(GFileInfo *info, gchar **output){
	GFileType type = g_file_info_get_attribute_uint32(info,
					G_FILE_ATTRIBUTE_STANDARD_TYPE);
	if( type != G_FILE_TYPE_DIRECTORY ){
		guint64 size = g_file_info_get_attribute_uint64(info,
						G_FILE_ATTRIBUTE_STANDARD_SIZE);
		gchar tmp[BUFF_SIZE];
		g_snprintf(tmp, BUFF_SIZE, "%ld", size);
		*output = g_strndup(tmp, BUFF_SIZE);
	}else{
		*output = g_strdup("<DIR>");
	}
	return 0;
}

int ucmd_column_get_info_mtime(GFileInfo *info, gchar **output){
	guint64 mtime = g_file_info_get_attribute_uint64(info,
					G_FILE_ATTRIBUTE_TIME_MODIFIED);
	GDate *f_date = g_date_new();
	g_date_set_time_t(f_date, mtime);
	gchar tmp[BUFF_SIZE];
	g_date_strftime(tmp, BUFF_SIZE, "%F", f_date);
	*output = g_strndup(tmp, BUFF_SIZE);
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
	*output = g_strndup(mode_label, 11);
	return 0;
}

int ucmd_column_get_info_path(GFileInfo *info, gchar **output){
	*output = g_strdup("NYI");
	return 0;
}

int ucmd_column_get_info_icon(GFileInfo *info, gchar **output){
	gchar *type = (gchar*)g_file_info_get_attribute_string(info,
					G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
	if( type == NULL ){
		return EGETINFO;
	}
	gchar *icon = g_content_type_get_generic_icon_name(type);
	if( icon != NULL ){
		*output = icon;
	}else{
		*output = g_strdup("text-x-generic");
	}
	return 0;
}
