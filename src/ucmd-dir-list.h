#pragma once
#define BUFF_SIZE 1024
enum DirListErrors{
	EOPENDIR,
	ECREATELIST
};

enum _UcmdDirListColumns{
	PATH_COLUMN,
	NAME_COLUMN,
	TYPE_COLUMN,
	SIZE_COLUMN,
	DATE_COLUMN,
	ATTR_COLUMN,
	ISDIR_COLUMN,
	NUM_COLUMNS
};

typedef int (*ucmd_finfo_callback_t)(GFile *file, gchar output[BUFF_SIZE]);

typedef struct _UcommanderDirListColumn{
	const gchar *name;
	size_t type;
	size_t index;
	gboolean is_sortable;
	ucmd_finfo_callback_t get_info_func;
} UcommanderDirListColumn;

typedef struct _UcommanderDirList{
	const gchar *path;
	UcommanderDirListColumn **columns;
	GtkLabel *path_label;
	GtkListStore *store;
} UcommanderDirList;

UcommanderDirListColumn **ucmd_list_columns;

size_t ucmd_dir_list_get_columns_amount();

void ucmd_dir_list_get_visible_columns();

UcommanderDirListColumn *ucmd_dir_list_get_column(size_t index);

int ucmd_read_dir(const gchar *path, GtkListStore *store);

int ucmd_create_dir_list(const gchar *path, UcommanderDirList **list);

void ucmd_free_dir_list(UcommanderDirList *list);

int ucmd_column_get_file_name(GFileInfo *file, gchar output[BUFF_SIZE]);

int ucmd_column_get_file_ext(GFileInfo *file, gchar output[BUFF_SIZE]);

int ucmd_column_get_file_type(GFileInfo *file, gchar output[BUFF_SIZE]);

int ucmd_column_get_file_size(GFileInfo *file, gchar output[BUFF_SIZE]);

int ucmd_column_get_file_mtime(GFileInfo *file, gchar output[BUFF_SIZE]);

int ucmd_column_get_file_mode(GFileInfo *file, gchar output[BUFF_SIZE]);

int ucmd_column_get_file_path(GFileInfo *file, gchar output[BUFF_SIZE]);
