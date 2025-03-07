/*
 * Tests for the libappindicator library that are over DBus. This is
 * the server side of those tests.
 *
 * Copyright 2009 Ted Gould <ted@canonical.com>
 * Copyright 2023-2024 Robert Tari <robert@tari.in>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ayatana-appindicator.h>

static GMainLoop *m_pMainLoop = NULL;
static gboolean m_bActive = FALSE;
static guint m_nToggleCount = 0;
static GDBusConnection *m_pConnection = NULL;

static gboolean onTimeUp (gpointer pData)
{
    g_dbus_connection_flush_sync (m_pConnection, NULL, NULL);
    g_clear_object (&m_pConnection);
    g_main_loop_quit (m_pMainLoop);

    return G_SOURCE_REMOVE;
}

gboolean toggle (gpointer pData)
{
    const AppIndicatorStatus nStatus = m_bActive ? APP_INDICATOR_STATUS_ATTENTION : APP_INDICATOR_STATUS_ACTIVE;
    app_indicator_set_status (APP_INDICATOR (pData), nStatus);
    ++m_nToggleCount;
    m_bActive = !m_bActive;

    if (m_nToggleCount == 100)
    {
        g_timeout_add (100, onTimeUp, NULL);

        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

gint main (gint argc, gchar * argv[])
{
    g_usleep(100000);
    m_pConnection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
    g_debug ("DBus Name: %s", g_dbus_connection_get_unique_name (m_pConnection));

    AppIndicator *pIndicator = app_indicator_new ("my-id", "my-icon-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_attention_icon (pIndicator, "my-attention-icon", NULL);

    GSimpleActionGroup *pActions = g_simple_action_group_new ();
    GSimpleAction *pSimpleAction = g_simple_action_new ("test", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_object_unref (pSimpleAction);
    GMenu *pMenu = g_menu_new ();
    GMenuItem *pItem = g_menu_item_new ("Test", "indicator.test");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    app_indicator_set_menu (pIndicator, pMenu);
    g_object_unref (pMenu);
    app_indicator_set_actions (pIndicator, pActions);
    g_object_unref (pActions);
    g_timeout_add (50, toggle, pIndicator);
    m_pMainLoop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (m_pMainLoop);
    g_object_unref (G_OBJECT (pIndicator));

    g_debug("Quiting");

    return 0;
}
