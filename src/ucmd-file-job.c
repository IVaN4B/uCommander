#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "ucmd-dir-list.h"
#include "ucmd-dir-view.h"
#include "ucmd-file-job.h"

int start_copy_files(GList *files, gpointer data){
	return 0;
}

int do_copy_files(GList *files, gpointer data){
	return 0;
}

int end_copy_files(GList *files, gpointer data){
	return 0;
}

int start_move_files(GList *files, gpointer data){
	return 0;
}

int do_move_files(GList *files, gpointer data){
	return 0;
}

int end_move_files(GList *files, gpointer data){
	return 0;
}

int do_mkdir_job(GList *files, gpointer data){
	return 0;
}

int start_delete_files(GList *files, gpointer data){
	return 0;
}

int do_delete_files(GList *files, gpointer data){
	return 0;
}

int end_delete_files(GList *files, gpointer data){
	return 0;
}

UcommanderJobOps default_job_ops[DEFAULT_JOBS_AMOUNT] = {
	{COPY_JOB, &start_copy_files, &do_copy_files, &end_copy_files},
	{MOVE_JOB, &start_move_files, &do_move_files, &end_move_files},
	{MKDIR_JOB, NULL, &do_mkdir_job, NULL},
	{DELETE_JOB, &start_delete_files, &do_delete_files, &end_delete_files}
};

void ucmd_job_create(gint type, GList *files, UcommanderDirView *dir_view,
				gpointer data,
				UcommanderJob **job){
}

void ucmd_job_free(UcommanderJob *job){
}

void ucmd_job_run(UcommanderJob *job, gpointer data, GCancellable *cancellable){
}
