/*
 * Tests for the libappindicator library that are over DBus.  This is
 * the client side of those tests.
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

#include <gio/gio.h>
#include "../src/dbus-shared.h"

static GMainLoop *m_pMainLoop = NULL;
static gboolean m_bPassed = TRUE;
static gboolean m_bWatchdogHit = TRUE;
static gboolean m_bActive = FALSE;
static guint m_nToggleCount = 0;

static GDBusMessage* dbusFilter (GDBusConnection *pConnection, GDBusMessage *pMessage, gboolean bIncoming, void *pUserData)
{
    GDBusMessageType cType = g_dbus_message_get_message_type (pMessage);
    const gchar *sInterface = g_dbus_message_get_interface (pMessage);
    const gchar *sMember = g_dbus_message_get_member (pMessage);
    int nInterfaceComp = g_strcmp0 (sInterface, NOTIFICATION_WATCHER_DBUS_ADDR);
    int nMemberComp = g_strcmp0 (sMember, "RegisterStatusNotifierItem");

    if (cType == G_DBUS_MESSAGE_TYPE_METHOD_CALL && nInterfaceComp == 0 && nMemberComp == 0)
    {
        GDBusMessage *pReply = g_dbus_message_new_method_reply (pMessage);
        g_dbus_connection_send_message (pConnection, pReply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, NULL, NULL);
        g_object_unref (pReply);
        g_object_unref (pMessage);
        pMessage = NULL;
    }

    return pMessage;
}

void onNewStatus (GDBusConnection* pConnection, const gchar *sSender, const gchar *sPath, const gchar *sInterface, const gchar *sSignal, GVariant *pParameters, gpointer pUserData)
{
    const gchar *sParam = NULL;
    g_variant_get (pParameters, "(s)", &sParam);

    if (!sParam)
    {
        g_warning ("Couldn't get parameter");

        return;
    }

    m_bWatchdogHit = TRUE;
    int nComp = g_strcmp0 (sParam, "Active");

    if (!nComp)
    {
        if (m_bActive)
        {
            g_warning ("Got active when already active");
            m_bPassed = FALSE;

            return;
        }

        m_bActive = TRUE;
    }
    else
    {
        m_bActive = FALSE;
    }

    m_nToggleCount++;

    if (m_nToggleCount == 100)
    {
        g_main_loop_quit (m_pMainLoop);
    }
}

gboolean onKill (gpointer pData)
{
    if (m_bWatchdogHit == FALSE)
    {
        g_main_loop_quit (m_pMainLoop);
        g_warning ("Forced to Kill");
        g_warning ("Toggle count: %d", m_nToggleCount);
        m_bPassed = FALSE;

        return FALSE;
    }

    m_bWatchdogHit = FALSE;

    return TRUE;
}

gint main (gint argc, gchar * argv[])
{
    GError *pError = NULL;
    GDBusProxy *pProxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, NULL, &pError);

    if (pError != NULL)
    {
        g_error ("Unable to get session bus: %s", pError->message);
        g_error_free (pError);

        return 1;
    }

    GDBusConnection *pConnection = g_dbus_proxy_get_connection (pProxy);
    guint nName = g_bus_own_name_on_connection (pConnection, NOTIFICATION_WATCHER_DBUS_ADDR, G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);

    if (!nName)
    {
        g_error ("Unable to call to request name");

        return 1;
    }

    if (nName != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
        g_error ("Unable to get name");

        return 1;
    }

    g_dbus_connection_add_filter (pConnection, dbusFilter, NULL, NULL);
    g_dbus_connection_signal_subscribe (pConnection, NULL, NOTIFICATION_ITEM_DBUS_IFACE, "NewStatus", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, onNewStatus, NULL, NULL);

    m_bWatchdogHit = TRUE;
    g_timeout_add_seconds (20, onKill, NULL);

    m_pMainLoop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (m_pMainLoop);

    if (m_bPassed)
    {
        g_debug ("Quiting");

        return 0;

    }
    else
    {
        g_debug ("Quiting as we're a failure");

        return 1;
    }

    return 0;
}
