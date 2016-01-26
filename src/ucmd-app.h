#pragma once

G_BEGIN_DECLS

#define UCOMMANDER_TYPE_APPLICATION             (ucmd_get_type ())
#define UCOMMANDER_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), UCOMMANDER_TYPE_APPLICATION, Ucommander))
#define UCOMMANDER_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), UCOMMANDER_TYPE_APPLICATION, UcommanderClass))
#define UCOMMANDER_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UCOMMANDER_TYPE_APPLICATION))
#define UCOMMANDER_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), UCOMMANDER_TYPE_APPLICATION))
#define UCOMMANDER_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), UCOMMANDER_TYPE_APPLICATION, UcommanderClass))

#define UI_FILE "src/ui/main_window.ui"
#define TOP_WINDOW "main_window"
#define APP_NAME "uCommander"
#define APP_VERSION "0.1a"

typedef struct _UcommanderClass UcommanderClass;
typedef struct _Ucommander Ucommander;
typedef struct _UcommanderPrivate UcommanderPrivate;

struct _UcommanderClass {
	GtkApplicationClass parent_class;
};

struct _Ucommander {
	GtkApplication parent_instance;

	UcommanderPrivate *priv;

};

GType ucmd_get_type (void) G_GNUC_CONST;
Ucommander *ucmd_new (void);

/* Callbacks */

G_END_DECLS
