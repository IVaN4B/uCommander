#pragma once

typedef enum _UcmdJobType{
	COPY_JOB,
	MOVE_JOB,
	MKDIR_JOB,
	DELETE_JOB,
	DEFAULT_JOBS_AMOUNT
} UcmdJobType;

typedef int (*ucmd_job_callback_t)(GList *files, gpointer data);

typedef struct _UcommanderJobOps{
	gint type;
	ucmd_job_callback_t start_callback;
	ucmd_job_callback_t process_callback;
	ucmd_job_callback_t end_callback;

} UcommanderJobOps;

typedef struct _UcommanderJob{
	gint type;
	GList *files;
	UcommanderDirView *dir_view;
	gpointer data;	
} UcommanderJob;

void ucmd_job_create(gint type, GList *files, UcommanderDirView *dir_view,
				gpointer data,
				UcommanderJob **job);

void ucmd_job_free(UcommanderJob *job);

void ucmd_job_run(UcommanderJob *job, gpointer data, GCancellable *cancellable);
