#pragma once

enum DirListErrors{
	EOPENDIR,
	ECREATELIST
};

enum DefaultListColumns{
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
	gchar *path;
	GtkListStore *store;
} UcommanderDirList;

void ucmd_get_dir_list_columns(gchar **columns, size_t amount);

int ucmd_read_dir(const gchar *path, GtkListStore *store);

int ucmd_create_dir_list(const gchar *path, UcommanderDirList **list);

void ucmd_free_dir_list(UcommanderDirList *list);
