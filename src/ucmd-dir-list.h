#pragma once

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

typedef struct _UcommanderDirList{
	const gchar *path;
	GtkLabel *path_label;
	GtkListStore *store;
} UcommanderDirList;

void ucmd_dir_list_get_visible_columns(size_t **columns, size_t *amount);

gchar *ucmd_dir_list_get_column_name(size_t index);

int ucmd_read_dir(const gchar *path, GtkListStore *store);

int ucmd_create_dir_list(const gchar *path, UcommanderDirList **list);

void ucmd_free_dir_list(UcommanderDirList *list);
