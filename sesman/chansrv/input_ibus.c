#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <ibus.h>
#include "input.h"
#include "thread_calls.h"

static IBusBus *bus;
static IBusEngine *g_engine;
static int id = 0;

int
xrdp_input_send_unicode(uint32_t unicode)
{
    LOG(LOG_LEVEL_INFO, "xrdp_input_send_unicode: received unicode input %i", unicode);

    gunichar chr = unicode;
    ibus_engine_commit_text(g_engine, ibus_text_new_from_unichar(chr));

    return 0;
}

void 
xrdp_input_ibus_engine_enable(IBusEngine *engine)
{
    LOG(LOG_LEVEL_INFO, "xrdp_ibus_engine_enable: IM enabled");
    g_engine = engine;
}

void 
xrdp_input_ibus_engine_disable(IBusEngine *engine)
{
    LOG(LOG_LEVEL_INFO, "xrdp_ibus_engine_disable: IM disabled");
}

void 
xrdp_input_ibus_disconnect(IBusEngine *engine)
{
    LOG(LOG_LEVEL_INFO, "xrdp_ibus_engine_disable: IM disabled");
    g_object_unref(g_engine);
    g_object_unref(bus);
}

gboolean engine_process_key_event_cb(IBusEngine *engine,
                                     guint keyval,
                                     guint keycode,
                                     guint state)
{
    if(state & IBUS_RELEASE_MASK) {
        return FALSE;
    }

    // Pass the keyboard event to system
    return FALSE;
}

IBusEngine *
xrdp_input_ibus_create_engine(IBusFactory *factory,
                        gchar *engine_name,
                        gpointer user_data)
{
    IBusEngine *engine;
    gchar *path = g_strdup_printf("/org/freedesktop/IBus/Engine/%i", 1);
    engine = ibus_engine_new(engine_name,
                             path,
                             ibus_bus_get_connection(bus));

    LOG(LOG_LEVEL_INFO, "xrdp_input_ibus_create_engine: Creating IM Engine with name:%s and id:%d\n", engine_name, ++id);

    g_signal_connect(engine, "process-key-event", G_CALLBACK(engine_process_key_event_cb), NULL);
    g_signal_connect(engine, "enable", G_CALLBACK(xrdp_input_ibus_engine_enable), NULL);
    g_signal_connect(engine, "disable", G_CALLBACK(xrdp_input_ibus_engine_disable), NULL);

    return engine;
}

int
xrdp_input_enable()
{
    if (!bus)
    {
        return 1;
    }

    ibus_bus_set_global_engine_async(bus, "XrdpIme", -1, NULL, NULL, NULL);
    LOG(LOG_LEVEL_INFO, "xrdp_ibus_init: input method switched successfully");

    return 0;
}

/*****************************************************************************/
THREAD_RV THREAD_CC
xrdp_input_main_loop()
{
    IBusFactory *factory;
    IBusComponent *component;
    IBusEngineDesc *desc;
    THREAD_RV rv = 0;

    LOG(LOG_LEVEL_INFO, "xrdp_input_main_loop: Entering iBus loop");

    g_signal_connect(bus, "disconnected", G_CALLBACK(xrdp_input_ibus_disconnect), NULL);

    factory = ibus_factory_new(ibus_bus_get_connection(bus));
    g_object_ref_sink(factory);
    g_signal_connect(factory, "create-engine", G_CALLBACK(xrdp_input_ibus_create_engine), NULL);
    g_signal_connect(factory, "enable", G_CALLBACK(xrdp_input_ibus_engine_enable), NULL);
    g_signal_connect(factory, "disable", G_CALLBACK(xrdp_input_ibus_engine_disable), NULL);

    ibus_factory_add_engine(factory, "XrdpIme", IBUS_TYPE_ENGINE);

    component = ibus_component_new("org.freedesktop.IBus.XrdpIme",
                                       "Xrdp input method",
                                       "1.1",
                                       "MIT",
                                       "seflerZ",
                                       "fake_icon",
                                       "/exec/fake_path",
                                       "xrdpime");

    desc = ibus_engine_desc_new("XrdpIme",
                                       "unicode input method for xrdp",
                                       "unicode input method for xrdp",
                                       "unicode",
                                       "MIT",
                                       "seflerZ",
                                       "fake_icon.png",
                                       "default");

    ibus_component_add_engine(component, desc);
    ibus_bus_register_component(bus, component);

    if (xrdp_input_enable())
    {
        return rv;
    }

    ibus_main();

    g_object_unref(desc);
    g_object_unref(component);
    g_object_unref(factory);

    return rv;
}

int
xrdp_input_destory()
{
    return 0;
}

int
xrdp_input_init()
{
    if (bus != NULL)
    {
        // Already initialized
        return 0;
    }

    LOG(LOG_LEVEL_INFO, "xrdp_ibus_init: Initializing the iBus engine");
    ibus_init();
    bus = ibus_bus_new();
    g_object_ref_sink(bus);

    if (!ibus_bus_is_connected(bus))
    {
        LOG(LOG_LEVEL_INFO, "xrdp_ibus_init: Connect to iBus failed");
        return 1;
    }

    LOG(LOG_LEVEL_INFO, "xrdp_ibus_init: iBus connected");

    tc_thread_create(xrdp_input_main_loop, NULL);

    return 0;
}