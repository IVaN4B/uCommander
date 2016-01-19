#ifndef _UCOMMANDER_
#define _UCOMMANDER_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define UCOMMANDER_TYPE_APPLICATION             (ucommander_get_type ())
#define UCOMMANDER_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), UCOMMANDER_TYPE_APPLICATION, Ucommander))
#define UCOMMANDER_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), UCOMMANDER_TYPE_APPLICATION, UcommanderClass))
#define UCOMMANDER_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UCOMMANDER_TYPE_APPLICATION))
#define UCOMMANDER_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), UCOMMANDER_TYPE_APPLICATION))
#define UCOMMANDER_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), UCOMMANDER_TYPE_APPLICATION, UcommanderClass))

typedef struct _UcommanderClass UcommanderClass;
typedef struct _Ucommander Ucommander;
typedef struct _UcommanderPrivate UcommanderPrivate;

struct _UcommanderClass
{
	GtkApplicationClass parent_class;
};

struct _Ucommander
{
	GtkApplication parent_instance;

	UcommanderPrivate *priv;

};

GType ucommander_get_type (void) G_GNUC_CONST;
Ucommander *ucommander_new (void);

/* Callbacks */

G_END_DECLS

#endif /* _APPLICATION_H_ */
