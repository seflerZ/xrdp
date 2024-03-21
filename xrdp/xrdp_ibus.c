#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <ibus.h>
#include "xrdp_ibus.h"

static IBusEngine *engine;
static IBusBus *bus;
static gint id = 0;

/*****************************************************************************/
void
xrdp_ibus_send_unicode(int code)
{

}

/*****************************************************************************/
void
xrdp_ibus_init()
{
    // LOG(LOG_LEVEL_INFO, "xrdp_ibus_init: Initializing the iBus engine");
}

void 
xrdp_ibus_engine_enable(IBusEngine *engine)
{
    // LOG(LOG_LEVEL_INFO, "xrdp_ibus_engine_enable: IM enabled");
}

void 
xrdp_ibus_engine_disable(IBusEngine *engine)
{
    // LOG(LOG_LEVEL_INFO, "xrdp_ibus_engine_disable: IM disabled");
}

IBusEngine *
xrdp_ibus_create_engine(IBusFactory *factory,
                        gchar *engine_name,
                        gpointer user_data)
{
    id += 1;
    gchar *path = g_strdup_printf("/org/freedesktop/IBus/Engine/xrdp-%i", id);
    engine = ibus_engine_new(engine_name,
                             path,
                             ibus_bus_get_connection(bus));

    // LOG(LOG_LEVEL_INFO, "xrdp_ibus_create_engine: Creating IM Engine with name:%s and id:%d\n", engine_name, id);

    g_signal_connect(engine, "enable", G_CALLBACK(xrdp_ibus_engine_enable), NULL);
    g_signal_connect(engine, "disable", G_CALLBACK(xrdp_ibus_engine_disable), NULL);

    return engine;
}