#pragma once
enum DirListErrors{
	EOPENDIR,
	ECREATELIST,
	EGETINFO
};

typedef int (*ucmd_finfo_callback_t)(GFileInfo *file, gchar **output);

typedef struct _UcommanderDirListColumn{
	const gchar *name;
	size_t type;
	size_t position;
	gboolean visible;
	gboolean always_process;
	gboolean is_sortable;
	ucmd_finfo_callback_t get_info;
} UcommanderDirListColumn;

typedef struct _UcommanderDirList{
	const gchar *path;
	UcommanderDirListColumn **columns;
	GtkListStore *store;
} UcommanderDirList;

size_t ucmd_dir_list_get_columns_amount();

void ucmd_dir_list_get_visible_columns();

UcommanderDirListColumn *ucmd_dir_list_get_column(size_t index);

UcommanderDirListColumn *ucmd_dir_list_get_name_column();

int ucmd_read_dir(const gchar *path, const UcommanderDirList *list);

int ucmd_dir_list_create(const gchar *path, UcommanderDirList **list);

void ucmd_dir_list_free(UcommanderDirList *list);

int ucmd_column_get_info_name(GFileInfo *info, gchar **output);

int ucmd_column_get_info_ext(GFileInfo *info, gchar **output);

int ucmd_column_get_info_type(GFileInfo *info, gchar **output);

int ucmd_column_get_info_size(GFileInfo *info, gchar **output);

int ucmd_column_get_info_mtime(GFileInfo *info, gchar **output);

int ucmd_column_get_info_mode(GFileInfo *info, gchar **output);

int ucmd_column_get_info_path(GFileInfo *info, gchar **output);
