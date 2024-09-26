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

#include <ayatana-appindicator.h>
#include "test-defines.h"
#include "../src/dbus-shared.h"

static GMainLoop *m_pMainLoop = NULL;
static gboolean m_bPassed = TRUE;
static int m_nPropCount = 0;

static void checkPropCount (void)
{
    if (m_nPropCount >= 6)
    {
        g_main_loop_quit (m_pMainLoop);
    }

    return;
}

static void onPropId (GObject *pObject, GAsyncResult *pResult, void *pData)
{
    m_nPropCount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sId = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sId);
        gint nComp = g_strcmp0 (TEST_ID, sId);

        if (nComp)
        {
            g_debug ("Property ID Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property ID Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting ID failed: %s", pError->message);
        g_error_free (pError);
        m_bPassed = FALSE;
        checkPropCount ();

        return;
    }

    checkPropCount ();

    return;
}

static void onPropCategory (GObject *pObject, GAsyncResult *pResult, void *pData)
{
    m_nPropCount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sCategory = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sCategory);
        gint nComp = g_strcmp0 ("ApplicationStatus", sCategory);

        if (nComp)
        {
            g_debug ("Property category Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property category Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting category failed: %s", pError->message);
        g_error_free (pError);
        m_bPassed = FALSE;
        checkPropCount ();

        return;
    }

    checkPropCount ();

    return;
}

static void onPropStatus (GObject *pObject, GAsyncResult *pResult, void *pData)
{
    m_nPropCount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sStatus = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sStatus);
        gint nComp = g_strcmp0 (TEST_STATE_S, sStatus);

        if (nComp)
        {
            g_debug ("Property status Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property status Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting status failed: %s", pError->message);
        g_error_free (pError);
        m_bPassed = FALSE;
        checkPropCount ();

        return;
    }

    checkPropCount ();

    return;
}

static void onPropIconName (GObject *pObject, GAsyncResult *pResult, void *pData)
{
    m_nPropCount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sIcon = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sIcon);
        gint nComp = g_strcmp0 (TEST_ICON_NAME, sIcon);

        if (nComp)
        {
            g_debug ("Property icon name Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property icon name Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting icon name failed: %s", pError->message);
        g_error_free (pError);
        m_bPassed = FALSE;
        checkPropCount ();

        return;
    }

    checkPropCount ();

    return;
}

static void onPropAttentionIconName (GObject *pObject, GAsyncResult *pResult, void *pData)
{
    m_nPropCount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sAttentionIcon = NULL;
        g_variant_get (pParams, "(ss)", &sInterface, &sAttentionIcon);
        gint nComp = g_strcmp0 (TEST_ATTENTION_ICON_NAME, sAttentionIcon);

        if (nComp)
        {
            g_debug ("Property attention icon name Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property attention icon name Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting attention icon name failed: %s", pError->message);
        g_error_free (pError);
        m_bPassed = FALSE;
        checkPropCount ();

        return;
    }

    checkPropCount ();

    return;
}

static void onPropTooltip (GObject *pObject, GAsyncResult *pResult, void *pData)
{
    m_nPropCount++;
    GError *pError = NULL;
    GVariant *pParams = g_dbus_proxy_call_finish (G_DBUS_PROXY (pObject), pResult, &pError);

    if (pParams)
    {
        gchar *sInterface = NULL;
        gchar *sIcon = NULL;
        gchar *sTitle = NULL;
        gchar *sDescription = NULL;
        g_variant_get (pParams, "(ssa(iiay)ss)", &sInterface, &sIcon, NULL, &sTitle, &sDescription);
        gint nComp = g_strcmp0 (TEST_ICON_NAME, sIcon);

        if (nComp)
        {
            g_debug ("Property tooltip icon name Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property tooltip icon name Returned: PASSED");
        }

        nComp = g_strcmp0 ("tooltip-title", sTitle);

        if (nComp)
        {
            g_debug ("Property tooltip title Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property tooltip title Returned: PASSED");
        }

        nComp = g_strcmp0 ("tooltip-description", sDescription);

        if (nComp)
        {
            g_debug ("Property tooltip description Returned: FAILED");
            m_bPassed = FALSE;
        }
        else
        {
            g_debug ("Property tooltip description Returned: PASSED");
        }

        g_variant_unref (pParams);
    }

    if (pError)
    {
        g_warning ("Getting tooltip failed: %s", pError->message);
        g_error_free (pError);
        m_bPassed = FALSE;
        checkPropCount ();

        return;
    }

    checkPropCount ();

    return;
}

gboolean onKill (gpointer pData)
{
    g_main_loop_quit (m_pMainLoop);
    g_warning ("Forced to Kill");
    m_bPassed = FALSE;

    return FALSE;
}

static GDBusMessage* onDbusFilter (GDBusConnection *pConnection, GDBusMessage *pMessage, gboolean bIncoming, void *pData)
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

    g_dbus_connection_add_filter (pConnection, onDbusFilter, NULL, NULL);
    g_usleep (500000);
    GDBusProxy *pProps = g_dbus_proxy_new_sync (pConnection, G_DBUS_PROXY_FLAGS_NONE, NULL, ":1.2", "/org/ayatana/appindicator/my_id", DBUS_INTERFACE_PROPERTIES, NULL, &pError);

    if (pError != NULL)
    {
        g_error ("Unable to get property proxy: %s", pError->message);
        g_error_free (pError);

        return 1;
    }

    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "Id"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) onPropId, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "Category"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) onPropCategory, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "Status"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) onPropStatus, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "IconName"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) onPropIconName, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "AttentionIconName"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) onPropAttentionIconName, NULL);
    g_dbus_proxy_call (pProps, "Get", g_variant_new ("(ss)", NOTIFICATION_ITEM_DBUS_IFACE, "ToolTip"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) onPropTooltip, NULL);
    g_timeout_add_seconds (2, onKill, NULL);
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
