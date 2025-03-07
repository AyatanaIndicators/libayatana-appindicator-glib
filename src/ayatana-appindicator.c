/*
 * Copyright 2009 Cody Russell <cody.russell@canonical.com>
 * Copyright 2009-2010 Ted Gould <ted@canonical.com>
 * Copyright 2022-2024 Robert Tari <robert@tari.in>
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

#define _GNU_SOURCE

#include "ayatana-appindicator.h"
#include "ayatana-appindicator-enum-types.h"
#include "application-service-marshal.h"
#include "dbus-shared.h"

enum
{
    NEW_ICON,
    NEW_ATTENTION_ICON,
    NEW_STATUS,
    NEW_LABEL,
    NEW_TOOLTIP,
    CONNECTION_CHANGED,
    NEW_ICON_THEME_PATH,
    SCROLL_EVENT,
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_ID,
    PROP_CATEGORY,
    PROP_STATUS,
    PROP_ICON_NAME,
    PROP_ICON_DESC,
    PROP_ATTENTION_ICON_NAME,
    PROP_ATTENTION_ICON_DESC,
    PROP_ICON_THEME_PATH,
    PROP_CONNECTED,
    PROP_LABEL,
    PROP_LABEL_GUIDE,
    PROP_ORDERING_INDEX,
    PROP_TITLE,
    PROP_MENU,
    PROP_ACTIONS,
    PROP_TOOLTIP_ICON,
    PROP_TOOLTIP_TITLE,
    PROP_TOOLTIP_DESCRIPTION
};

static guint signals[LAST_SIGNAL] = {0};
static GDBusNodeInfo *m_pItemNodeInfo = NULL;
static GDBusInterfaceInfo *m_pItemInterfaceInfo = NULL;
static GDBusNodeInfo *m_pWatcherNodeInfo = NULL;
static GDBusInterfaceInfo *m_pWatcherInterfaceInfo = NULL;

typedef struct _AppIndicatorPrivate
{
    gchar *sId;
    gchar *sCleanId;
    AppIndicatorCategory nCategory;
    AppIndicatorStatus nStatus;
    gchar *sIconName;
    gchar *sAbsoluteIconName;
    gchar *sAttentionIconName;
    gchar *sAbsoluteAttentionIconName;
    gchar *sIconThemePath;
    gchar *sAbsoluteIconThemePath;
    gchar *sSecActivateTarget;
    guint32 nOrderingIndex;
    gchar *sTitle;
    gchar *sLabel;
    gchar *sLabelGuide;
    gchar *sAccessibleDesc;
    gchar *sAttAccessibleDesc;
    guint nLabelChangeIdle;
    GMenu *pMenu;
    GSimpleActionGroup *pActions;
    guint nActionsId;
    guint nMenuId;
    GDBusConnection *pConnection;
    guint nDbusRegistration;
    gchar *sPath;
    GDBusProxy *pWatcherProxy;
    guint nWatcherId;
    gchar *sTooltipIcon;
    gchar *sTooltipTitle;
    gchar *sTooltipDescription;
} AppIndicatorPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AppIndicator, app_indicator, G_TYPE_OBJECT);

static guint32 generateId (const AppIndicatorCategory nCat, const gchar *sId)
{
    guchar nCategory = 0;
    guchar nFirst = 0;
    guchar nSecond = 0;
    guchar nThird = 0;
    guchar nMultiplier = 32;

    switch (nCat)
    {
        case APP_INDICATOR_CATEGORY_OTHER:
        {
            nCategory = nMultiplier * 5;

            break;
        }
        case APP_INDICATOR_CATEGORY_APPLICATION_STATUS:
        {
            nCategory = nMultiplier * 4;

            break;
        }
        case APP_INDICATOR_CATEGORY_COMMUNICATIONS:
        {
            nCategory = nMultiplier * 3;

            break;
        }
        case APP_INDICATOR_CATEGORY_SYSTEM_SERVICES:
        {
            nCategory = nMultiplier * 2;

            break;
        }
        case APP_INDICATOR_CATEGORY_HARDWARE:
        {
            nCategory = nMultiplier * 1;

            break;
        }
        default:
        {
            g_warning ("Got an undefined category: %d", nCategory);
            nCategory = 0;

            break;
        }
    }

    if (sId != NULL)
    {
        if (sId[0] != '\0')
        {
            nFirst = sId[0];

            if (sId[1] != '\0')
            {
                nSecond = sId[1];

                if (sId[2] != '\0')
                {
                    nThird = sId[2];
                }
            }
        }
    }

    return (((((nCategory << 8) + nFirst) << 8) + nSecond) << 8) + nThird;
}

static void onBusMethodCall (GDBusConnection *pConnection, const gchar *sSender, const gchar *sPath, const gchar *sInterface, const gchar *sMethod, GVariant *pParams, GDBusMethodInvocation *pInvocation, gpointer pData)
{
    g_return_if_fail (APP_IS_INDICATOR (pData));

    AppIndicator *self = APP_INDICATOR (pData);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    gint nCompare = g_strcmp0 (sMethod, "Scroll");

    if (nCompare == 0)
    {
        guint nDirection = 0;
        gint nDelta = 0;
        const gchar *sOrientation = NULL;
        g_variant_get (pParams, "(i&s)", &nDelta, &sOrientation);
        nCompare = g_strcmp0 (sOrientation, "horizontal");

        if (nCompare == 0)
        {
            nDirection = (nDelta >= 0) ? 3 : 2;
        }
        else
        {
            nCompare = g_strcmp0 (sOrientation, "vertical");

            if (nCompare == 0)
            {
                nDirection = (nDelta >= 0) ? 1 : 0;
            }
        }

        if (nCompare != 0)
        {
            g_dbus_method_invocation_return_value (pInvocation, NULL);

            return;
        }

        nDelta = ABS (nDelta);
        g_signal_emit (self, signals[SCROLL_EVENT], 0, nDelta, nDirection);
    }
    else
    {
        nCompare = g_strcmp0 (sMethod, "SecondaryActivate");

        if (nCompare != 0)
        {
            nCompare = g_strcmp0 (sMethod, "XAyatanaSecondaryActivate");
        }

        if (nCompare == 0)
        {
            if (pPrivate->pActions)
            {
                GAction *pAction = g_action_map_lookup_action (G_ACTION_MAP (pPrivate->pActions), pPrivate->sSecActivateTarget);

                if (pAction)
                {
                    gboolean bEnabled = g_action_get_enabled (pAction);

                    if (bEnabled)
                    {
                        g_action_activate (pAction, NULL);
                    }
                }
            }
        }
    }

    if (nCompare != 0)
    {
        g_warning ("Calling method '%s' on the app indicator and it's unknown", sMethod);
    }

    g_dbus_method_invocation_return_value (pInvocation, NULL);
}

// DBus is asking for a property so we should figure out what it wants and try and deliver
static GVariant* onBusGetProperty (GDBusConnection *pConnection, const gchar *sSender, const gchar *sPath, const gchar *sInterface, const gchar *sProperty, GError **pError, gpointer pData)
{
    g_return_val_if_fail (APP_IS_INDICATOR (pData), NULL);

    AppIndicator *self = APP_INDICATOR (pData);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    gint nCompare = g_strcmp0 (sProperty, "Id");

    if (nCompare == 0)
    {
        return g_variant_new_string (pPrivate->sId ? pPrivate->sId : "");
    }

    nCompare = g_strcmp0 (sProperty, "Category");

    if (nCompare == 0)
    {
        GEnumValue *pEnum = g_enum_get_value ((GEnumClass*) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), pPrivate->nCategory);
        GVariant *pValue = g_variant_new_string (pEnum->value_nick ? pEnum->value_nick : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "Status");

    if (nCompare == 0)
    {
        GEnumValue *pEnum = g_enum_get_value ((GEnumClass*) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), pPrivate->nStatus);
        GVariant *pValue = g_variant_new_string (pEnum->value_nick ? pEnum->value_nick : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "IconName");

    if (nCompare == 0)
    {
        if (pPrivate->sAbsoluteIconName)
        {
            GVariant *pValue = g_variant_new_string (pPrivate->sAbsoluteIconName);

            return pValue;
        }

        GVariant *pValue = g_variant_new_string (pPrivate->sIconName ? pPrivate->sIconName : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "AttentionIconName");

    if (nCompare == 0)
    {
        if (pPrivate->sAbsoluteAttentionIconName)
        {
            GVariant *pValue = g_variant_new_string (pPrivate->sAbsoluteAttentionIconName);

            return pValue;
        }

        GVariant *pValue = g_variant_new_string (pPrivate->sAttentionIconName ? pPrivate->sAttentionIconName : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "Title");

    if (nCompare == 0)
    {
        const gchar *sOutput = NULL;

        if (pPrivate->sTitle == NULL)
        {
            const gchar *sName = g_get_application_name ();

            if (sName != NULL)
            {
                sOutput = sName;
            }
            else
            {
                sOutput = "";
            }
        }
        else
        {
            sOutput = pPrivate->sTitle;
        }

        GVariant *pValue = g_variant_new_string (sOutput);

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "IconThemePath");

    if (nCompare == 0)
    {
        if (pPrivate->sAbsoluteIconThemePath)
        {
            GVariant *pValue = g_variant_new_string (pPrivate->sAbsoluteIconThemePath);

            return pValue;
        }

        GVariant *pValue = g_variant_new_string (pPrivate->sIconThemePath ? pPrivate->sIconThemePath : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "Menu");

    if (nCompare == 0)
    {
        GVariant *pValue = g_variant_new ("o", pPrivate->sPath);

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "XAyatanaLabel");

    if (nCompare == 0)
    {
        GVariant *pValue = g_variant_new_string (pPrivate->sLabel ? pPrivate->sLabel : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "XAyatanaLabelGuide");

    if (nCompare == 0)
    {
        GVariant *pValue = g_variant_new_string (pPrivate->sLabelGuide ? pPrivate->sLabelGuide : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "XAyatanaOrderingIndex");

    if (nCompare == 0)
    {
        GVariant *pValue = g_variant_new_uint32 (pPrivate->nOrderingIndex);

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "IconAccessibleDesc");

    if (nCompare == 0)
    {
        GVariant *pValue = g_variant_new_string (pPrivate->sAccessibleDesc ? pPrivate->sAccessibleDesc : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "AttentionAccessibleDesc");

    if (nCompare == 0)
    {
        GVariant *pValue = g_variant_new_string (pPrivate->sAttAccessibleDesc ? pPrivate->sAttAccessibleDesc : "");

        return pValue;
    }

    nCompare = g_strcmp0 (sProperty, "ToolTip");

    if (nCompare == 0)
    {
        GVariantBuilder cBuilder;
        g_variant_builder_init (&cBuilder, G_VARIANT_TYPE ("(sa(iiay)ss)"));
        g_variant_builder_add (&cBuilder, "s", pPrivate->sTooltipIcon ? pPrivate->sTooltipIcon : "");
        GVariantBuilder cArrayBuilder;
        g_variant_builder_init (&cArrayBuilder, G_VARIANT_TYPE ("a(iiay)"));
        GVariant *pArray = g_variant_builder_end (&cArrayBuilder);
        g_variant_builder_add_value (&cBuilder, pArray);
        g_variant_builder_add (&cBuilder, "s", pPrivate->sTooltipTitle ? pPrivate->sTooltipTitle : "");
        g_variant_builder_add (&cBuilder, "s", pPrivate->sTooltipDescription ? pPrivate->sTooltipDescription : "");
        GVariant *pValue = g_variant_builder_end (&cBuilder);

        return pValue;
    }

    *pError = g_error_new (0, 0, "Unknown property: %s", sProperty);

    return NULL;
}

// Response from the DBus command to register a service with a NotificationWatcher
static void onRegisterService (GObject *pObject, GAsyncResult *pResult, gpointer pData)
{
    GError *pError = NULL;
    GVariant *pReturns = g_dbus_proxy_call_finish (G_DBUS_PROXY(pObject), pResult, &pError);

    // We don't care about any return values
    if (pReturns != NULL)
    {
        g_variant_unref (pReturns);
    }

    if (pError != NULL)
    {
        // They didn't respond, ewww.  Not sure what they could be doing
        g_warning ("Unable to connect to the Notification Watcher: %s", pError->message);
        g_object_unref (G_OBJECT (pData));

        return;
    }

    g_return_if_fail (APP_IS_INDICATOR (pData));
    AppIndicator *self = APP_INDICATOR (pData);

    // Emit the AppIndicator::connection-changed signal
    g_signal_emit (self, signals[CONNECTION_CHANGED], 0, TRUE);
    g_object_unref (G_OBJECT (pData));

    return;
}

// This function is used to see if we have enough information to connect to things.  If we do, and we're not connected, it connects for us.
static void getConnectable (AppIndicator *self)
{
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    if (!pPrivate->pConnection)
    {
        return;
    }

    // If we already have a proxy, let's see if it has someone implementing it. If not, we can't do much more than to do nothing.
    if (pPrivate->pWatcherProxy)
    {
        gchar *sName = g_dbus_proxy_get_name_owner (pPrivate->pWatcherProxy);

        if (!sName)
        {
            return;
        }

        g_free (sName);
    }

    if (!pPrivate->pMenu || !pPrivate->pActions || !pPrivate->sIconName || !pPrivate->sId)
    {
        return;
    }

    if (!pPrivate->sPath)
    {
        pPrivate->sPath = g_strdup_printf ("/org/ayatana/appindicator/%s", pPrivate->sCleanId);
    }

    if (!pPrivate->nDbusRegistration)
    {
        GError *pError = NULL;
        const GDBusInterfaceVTable cTable = {.method_call = onBusMethodCall, .get_property = onBusGetProperty, .set_property = NULL};
        pPrivate->nDbusRegistration = g_dbus_connection_register_object (pPrivate->pConnection, pPrivate->sPath, m_pItemInterfaceInfo, &cTable, self, NULL, &pError);

        if (pError)
        {
            g_warning ("Unable to register object on path '%s': %s", pPrivate->sPath, pError->message);
            g_error_free (pError);

            return;
        }
    }

    // Export the actions
    if (!pPrivate->nActionsId)
    {
        GError *pError = NULL;
        pPrivate->nActionsId = g_dbus_connection_export_action_group (pPrivate->pConnection, pPrivate->sPath, G_ACTION_GROUP (pPrivate->pActions), &pError);

        if (!pPrivate->nActionsId)
        {
            g_warning ("Cannot export %s action group: %s", pPrivate->sPath, pError->message);
            g_clear_error (&pError);
        }
    }

    // Export the menus
    if (!pPrivate->nMenuId)
    {
        GError *pError = NULL;
        pPrivate->nMenuId = g_dbus_connection_export_menu_model (pPrivate->pConnection, pPrivate->sPath, G_MENU_MODEL (pPrivate->pMenu), &pError);

        if (!pPrivate->nMenuId)
        {
            g_warning ("Cannot export %s menu: %s", pPrivate->sPath, pError->message);
            g_clear_error (&pError);
        }
    }

    // NOTE: It's really important the order here.  We make sure to *publish* the object on the bus and *then* get the proxy.  The reason is that we want to ensure all the filters are setup before talking to the watcher and that's where the order is important.
    if (!pPrivate->pWatcherProxy)
    {
        return;
    }

    GVariant *pPath = g_variant_new ("(s)", pPrivate->sPath);
    AppIndicator *pSelf = g_object_ref (self);
    g_dbus_proxy_call (pPrivate->pWatcherProxy, "RegisterStatusNotifierItem", pPath, G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback) onRegisterService, pSelf);
}

static void onWatcherReady (GObject *pObject, GAsyncResult *pResult, gpointer pData)
{
    AppIndicator *self = APP_INDICATOR(pData);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    GError *pError = NULL;
    pPrivate->pWatcherProxy = g_dbus_proxy_new_finish (pResult, &pError);

    if (pError)
    {
        g_object_unref (self);
        g_error_free (pError);

        return;
    }

    getConnectable (self);
    g_object_unref (self);
}

static void onNameAppeared (GDBusConnection *pConnection, const gchar *sName, const gchar *sOwner, gpointer pData)
{
    AppIndicator *self = APP_INDICATOR(pData);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    g_dbus_proxy_new (pPrivate->pConnection, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS, m_pWatcherInterfaceInfo, NOTIFICATION_WATCHER_DBUS_ADDR, NOTIFICATION_WATCHER_DBUS_OBJ, NOTIFICATION_WATCHER_DBUS_IFACE, NULL, (GAsyncReadyCallback) onWatcherReady, (AppIndicator*) g_object_ref (self));
}

static void onNameVanished (GDBusConnection *pConnection, const gchar *sName, gpointer pData)
{
    AppIndicator *self = APP_INDICATOR (pData);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    g_clear_object (&pPrivate->pWatcherProxy);
    g_signal_emit (self, signals[CONNECTION_CHANGED], 0, FALSE);
}

// Free all objects, make sure that all the dbus signals are sent out before we shut this down
static void app_indicator_dispose (GObject *pObject)
{
    AppIndicator *self = APP_INDICATOR (pObject);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    if (pPrivate->nMenuId)
    {
        g_dbus_connection_unexport_menu_model (pPrivate->pConnection, pPrivate->nMenuId);
        pPrivate->nMenuId = 0;
    }

    if (pPrivate->nActionsId)
    {
        g_dbus_connection_unexport_action_group (pPrivate->pConnection, pPrivate->nActionsId);
        pPrivate->nActionsId = 0;
    }

    if (pPrivate->nStatus != APP_INDICATOR_STATUS_PASSIVE)
    {
        app_indicator_set_status (self, APP_INDICATOR_STATUS_PASSIVE);
    }

    if (pPrivate->nLabelChangeIdle != 0)
    {
        g_source_remove (pPrivate->nLabelChangeIdle);
        pPrivate->nLabelChangeIdle = 0;
    }

    g_clear_object (&pPrivate->pMenu);
    g_clear_object (&pPrivate->pActions);

    if (pPrivate->nWatcherId != 0)
    {
        g_bus_unwatch_name (pPrivate->nWatcherId);
        pPrivate->nWatcherId = 0;
    }

    if (pPrivate->pWatcherProxy != NULL)
    {
        g_object_unref (G_OBJECT (pPrivate->pWatcherProxy));
        pPrivate->pWatcherProxy = NULL;
        g_signal_emit (self, signals[CONNECTION_CHANGED], 0, FALSE);
    }

    if (pPrivate->nDbusRegistration != 0)
    {
        g_dbus_connection_unregister_object (pPrivate->pConnection, pPrivate->nDbusRegistration);
        pPrivate->nDbusRegistration = 0;
    }

    g_clear_object (&pPrivate->pConnection);

    if (pPrivate->sSecActivateTarget != NULL)
    {
        g_free (pPrivate->sSecActivateTarget);
        pPrivate->sSecActivateTarget = NULL;
    }

    G_OBJECT_CLASS (app_indicator_parent_class)->dispose (pObject);
}

// Free all of the memory that we could be using in the object
static void app_indicator_finalize (GObject *pObject)
{
    AppIndicator *self = APP_INDICATOR (pObject);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    if (pPrivate->nStatus != APP_INDICATOR_STATUS_PASSIVE)
    {
        g_warning ("Finalizing Application Status with the status set to: %d", pPrivate->nStatus);
    }

    if (pPrivate->sId != NULL)
    {
        g_free (pPrivate->sId);
        pPrivate->sId = NULL;
    }

    if (pPrivate->sCleanId != NULL)
    {
        g_free (pPrivate->sCleanId);
        pPrivate->sCleanId = NULL;
    }

    if (pPrivate->sIconName != NULL)
    {
        g_free (pPrivate->sIconName);
        pPrivate->sIconName = NULL;
    }

    if (pPrivate->sAbsoluteIconName != NULL)
    {
        g_free (pPrivate->sAbsoluteIconName);
        pPrivate->sAbsoluteIconName = NULL;
    }

    if (pPrivate->sAttentionIconName != NULL)
    {
        g_free (pPrivate->sAttentionIconName);
        pPrivate->sAttentionIconName = NULL;
    }

    if (pPrivate->sAbsoluteAttentionIconName != NULL)
    {
        g_free (pPrivate->sAbsoluteAttentionIconName);
        pPrivate->sAbsoluteAttentionIconName = NULL;
    }

    if (pPrivate->sIconThemePath != NULL)
    {
        g_free (pPrivate->sIconThemePath);
        pPrivate->sIconThemePath = NULL;
    }

    if (pPrivate->sAbsoluteIconThemePath != NULL)
    {
        g_free (pPrivate->sAbsoluteIconThemePath);
        pPrivate->sAbsoluteIconThemePath = NULL;
    }

    if (pPrivate->sTitle != NULL)
    {
        g_free (pPrivate->sTitle);
        pPrivate->sTitle = NULL;
    }

    if (pPrivate->sLabel != NULL)
    {
        g_free (pPrivate->sLabel);
        pPrivate->sLabel = NULL;
    }

    if (pPrivate->sLabelGuide != NULL)
    {
        g_free (pPrivate->sLabelGuide);
        pPrivate->sLabelGuide = NULL;
    }

    if (pPrivate->sAccessibleDesc != NULL)
    {
        g_free (pPrivate->sAccessibleDesc);
        pPrivate->sAccessibleDesc = NULL;
    }

    if (pPrivate->sAttAccessibleDesc != NULL)
    {
        g_free (pPrivate->sAttAccessibleDesc);
        pPrivate->sAttAccessibleDesc = NULL;
    }

    if (pPrivate->sPath != NULL)
    {
        g_free (pPrivate->sPath);
        pPrivate->sPath = NULL;
    }

    if (pPrivate->sTooltipIcon != NULL)
    {
        g_free (pPrivate->sTooltipIcon);
        pPrivate->sTooltipIcon = NULL;
    }

    if (pPrivate->sTooltipTitle != NULL)
    {
        g_free (pPrivate->sTooltipTitle);
        pPrivate->sTooltipTitle = NULL;
    }

    if (pPrivate->sTooltipDescription != NULL)
    {
        g_free (pPrivate->sTooltipDescription);
        pPrivate->sTooltipDescription = NULL;
    }

    G_OBJECT_CLASS (app_indicator_parent_class)->finalize (pObject);
}

// Sends the label changed signal and resets the source ID
static gboolean onLabelChangeIdle (gpointer pData)
{
    AppIndicator *self = APP_INDICATOR (pData);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    gchar *sLabel = pPrivate->sLabel != NULL ? pPrivate->sLabel : "";
    gchar *sGuide = pPrivate->sLabelGuide != NULL ? pPrivate->sLabelGuide : "";
    g_signal_emit (G_OBJECT (self), signals[NEW_LABEL], 0, sLabel, sGuide);

    if (pPrivate->nDbusRegistration != 0 && pPrivate->pConnection != NULL)
    {
        GError *pError = NULL;
        GVariant *pValue = g_variant_new ("(ss)", sLabel, sGuide);
        g_dbus_connection_emit_signal (pPrivate->pConnection, NULL, pPrivate->sPath, NOTIFICATION_ITEM_DBUS_IFACE, "XAyatanaNewLabel", pValue, &pError);

        if (pError != NULL)
        {
            g_warning ("Unable to send signal for XAyatanaNewLabel: %s", pError->message);
            g_error_free (pError);
        }
    }

    pPrivate->nLabelChangeIdle = 0;

    return FALSE;
}

// Sets up an idle function to send the label changed signal so that we don't send it too many times
static void signalLabelChange (AppIndicator *self)
{
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    // don't set it twice
    if (pPrivate->nLabelChangeIdle != 0)
    {
        return;
    }

    pPrivate->nLabelChangeIdle = g_idle_add (onLabelChangeIdle, self);
}

static void app_indicator_set_property (GObject *pObject, guint nPropId, const GValue *pValue, GParamSpec *pSpec)
{
    AppIndicator *self = APP_INDICATOR (pObject);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    GEnumValue *pEnum = NULL;

    switch (nPropId)
    {
        case PROP_ID:
        {
            if (pPrivate->sId != NULL)
            {
                g_warning ("Resetting ID value when I already had a value of: %s", pPrivate->sId);

                break;
            }

            const gchar *sValue = g_value_get_string (pValue);
            pPrivate->sId = g_strdup (sValue);
            pPrivate->sCleanId = g_strdup (pPrivate->sId);
            gchar *sCleaner = NULL;

            for (sCleaner = pPrivate->sCleanId; *sCleaner != '\0'; sCleaner++)
            {
                gboolean bAlNum = g_ascii_isalnum (*sCleaner);

                if (!bAlNum)
                {
                    *sCleaner = '_';
                }
            }

            getConnectable (self);

            break;
        }
        case PROP_CATEGORY:
        {
            const gchar *sValue = g_value_get_string (pValue);
            pEnum = g_enum_get_value_by_nick ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), sValue);

            if (pPrivate->nCategory != pEnum->value)
            {
                pPrivate->nCategory = pEnum->value;
            }

            break;
        }
        case PROP_STATUS:
        {
            const gchar *sValue = g_value_get_string (pValue);
            pEnum = g_enum_get_value_by_nick ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), sValue);
            app_indicator_set_status (APP_INDICATOR (pObject), pEnum->value);

            break;
        }
        case PROP_ICON_NAME:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_icon (APP_INDICATOR (pObject), sValue, pPrivate->sAccessibleDesc);
            getConnectable (self);

            break;
        }
        case PROP_ICON_DESC:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_icon (APP_INDICATOR (pObject), pPrivate->sIconName, sValue);
            getConnectable (self);

            break;
        }
        case PROP_ATTENTION_ICON_NAME:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_attention_icon (APP_INDICATOR (pObject), sValue, pPrivate->sAttAccessibleDesc);

            break;
        }
        case PROP_ATTENTION_ICON_DESC:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_attention_icon (APP_INDICATOR (pObject), pPrivate->sAttentionIconName, sValue);

            break;
        }
        case PROP_ICON_THEME_PATH:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_icon_theme_path (APP_INDICATOR (pObject), sValue);
            getConnectable (self);

            break;
        }
        case PROP_LABEL:
        {
            gchar *sOldLabel = pPrivate->sLabel;
            pPrivate->sLabel = g_value_dup_string (pValue);

            if (pPrivate->sLabel != NULL && pPrivate->sLabel[0] == '\0')
            {
                g_free (pPrivate->sLabel);
                pPrivate->sLabel = NULL;
            }

            gint nCompare = g_strcmp0 (sOldLabel, pPrivate->sLabel);

            if (nCompare != 0)
            {
                signalLabelChange (APP_INDICATOR (pObject));
            }

            if (sOldLabel != NULL)
            {
                g_free (sOldLabel);
            }

            break;
        }
        case PROP_TITLE:
        {
            gchar *sOldTitle = pPrivate->sTitle;
            pPrivate->sTitle = g_value_dup_string (pValue);

            if (pPrivate->sTitle != NULL && pPrivate->sTitle[0] == '\0')
            {
                g_free (pPrivate->sTitle);
                pPrivate->sTitle = NULL;
            }

            gint nCompare = g_strcmp0 (sOldTitle, pPrivate->sTitle);

            if (nCompare != 0 && pPrivate->pConnection != NULL)
            {
                GError *pError = NULL;
                g_dbus_connection_emit_signal(pPrivate->pConnection, NULL, pPrivate->sPath, NOTIFICATION_ITEM_DBUS_IFACE, "NewTitle", NULL, &pError);

                if (pError != NULL)
                {
                    g_warning ("Unable to send signal for NewTitle: %s", pError->message);
                    g_error_free (pError);
                }
            }

            if (sOldTitle != NULL)
            {
                g_free (sOldTitle);
            }

            break;
        }
        case PROP_LABEL_GUIDE:
        {
            gchar *sOldGuide = pPrivate->sLabelGuide;
            pPrivate->sLabelGuide = g_value_dup_string (pValue);

            if (pPrivate->sLabelGuide != NULL && pPrivate->sLabelGuide[0] == '\0')
            {
                g_free (pPrivate->sLabelGuide);
                pPrivate->sLabelGuide = NULL;
            }

            gint nCompare = g_strcmp0 (sOldGuide, pPrivate->sLabelGuide);

            if (nCompare != 0)
            {
                signalLabelChange (APP_INDICATOR (pObject));
            }

            if (pPrivate->sLabelGuide != NULL && pPrivate->sLabelGuide[0] == '\0')
            {
                g_free (pPrivate->sLabelGuide);
                pPrivate->sLabelGuide = NULL;
            }

            if (sOldGuide != NULL)
            {
                g_free (sOldGuide);
            }

            break;
        }
        case PROP_ORDERING_INDEX:
        {
          pPrivate->nOrderingIndex = g_value_get_uint (pValue);

          break;
        }
        case PROP_MENU:
        {
            g_clear_object (&pPrivate->pMenu);
            pPrivate->pMenu = G_MENU (g_value_dup_object (pValue));

            break;
        }
        case PROP_ACTIONS:
        {
            g_clear_object (&pPrivate->pActions);
            pPrivate->pActions = G_SIMPLE_ACTION_GROUP (g_value_dup_object (pValue));

            break;
        }
        case PROP_TOOLTIP_ICON:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_tooltip (APP_INDICATOR (pObject), sValue, pPrivate->sTooltipTitle, pPrivate->sTooltipDescription);

            break;
        }
        case PROP_TOOLTIP_TITLE:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_tooltip (APP_INDICATOR (pObject), pPrivate->sTooltipIcon, sValue, pPrivate->sTooltipDescription);

            break;
        }
        case PROP_TOOLTIP_DESCRIPTION:
        {
            const gchar *sValue = g_value_get_string (pValue);
            app_indicator_set_tooltip (APP_INDICATOR (pObject), pPrivate->sTooltipIcon, pPrivate->sTooltipTitle, sValue);

            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID (pObject, nPropId, pSpec);

            break;
        }
    }
}

static void app_indicator_get_property (GObject *pObject, guint nPropId, GValue *pValue, GParamSpec *pSpec)
{
    AppIndicator *self = APP_INDICATOR (pObject);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    GEnumValue *pEnum = NULL;

    switch (nPropId)
    {
        case PROP_ID:
        {
            g_value_set_string (pValue, pPrivate->sId);

            break;
        }
        case PROP_CATEGORY:
        {
            pEnum = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), pPrivate->nCategory);
            g_value_set_string (pValue, pEnum->value_nick);

            break;
        }
        case PROP_STATUS:
        {
            pEnum = g_enum_get_value ((GEnumClass *) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), pPrivate->nStatus);
            g_value_set_string (pValue, pEnum->value_nick);

            break;
        }
        case PROP_ICON_NAME:
        {
            g_value_set_string (pValue, pPrivate->sIconName);

            break;
        }
        case PROP_ICON_DESC:
        {
            g_value_set_string (pValue, pPrivate->sAccessibleDesc);

            break;
        }
        case PROP_ATTENTION_ICON_NAME:
        {
            g_value_set_string (pValue, pPrivate->sAttentionIconName);

            break;
        }
        case PROP_ATTENTION_ICON_DESC:
        {
            g_value_set_string (pValue, pPrivate->sAttAccessibleDesc);

            break;
        }
        case PROP_ICON_THEME_PATH:
        {
            g_value_set_string (pValue, pPrivate->sIconThemePath);

            break;
        }
        case PROP_CONNECTED:
        {
            gboolean bConnected = FALSE;

            if (pPrivate->pWatcherProxy != NULL)
            {
                gchar *sName = g_dbus_proxy_get_name_owner (pPrivate->pWatcherProxy);

                if (sName != NULL)
                {
                    bConnected = TRUE;
                    g_free (sName);
                }
            }

            g_value_set_boolean (pValue, bConnected);

            break;
        }
        case PROP_LABEL:
        {
            g_value_set_string (pValue, pPrivate->sLabel);

            break;
        }
        case PROP_LABEL_GUIDE:
        {
            g_value_set_string (pValue, pPrivate->sLabelGuide);

            break;
        }
        case PROP_ORDERING_INDEX:
        {
            g_value_set_uint(pValue, pPrivate->nOrderingIndex);

            break;
        }
        case PROP_TITLE:
        {
            g_value_set_string(pValue, pPrivate->sTitle);

            break;
        }
        case PROP_MENU:
        {
            g_value_set_object (pValue, pPrivate->pMenu);

            break;
        }
        case PROP_ACTIONS:
        {
            g_value_set_object (pValue, pPrivate->pActions);

            break;
        }
        case PROP_TOOLTIP_ICON:
        {
            g_value_set_string (pValue, pPrivate->sTooltipIcon);

            break;
        }
        case PROP_TOOLTIP_TITLE:
        {
            g_value_set_string (pValue, pPrivate->sTooltipTitle);

            break;
        }
        case PROP_TOOLTIP_DESCRIPTION:
        {
            g_value_set_string (pValue, pPrivate->sTooltipDescription);

            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID (pObject, nPropId, pSpec);

            break;
        }
    }
}

static void app_indicator_class_init (AppIndicatorClass *klass)
{
    GObjectClass *pClass = G_OBJECT_CLASS (klass);
    pClass->dispose = app_indicator_dispose;
    pClass->finalize = app_indicator_finalize;
    pClass->set_property = app_indicator_set_property;
    pClass->get_property = app_indicator_get_property;

    /**
     * AppIndicator:id:
     *
     * The ID for this indicator, which should be unique, but used consistently
     * by this program and its indicator.
     */
    GParamSpec *pSpecId = g_param_spec_string ("id", "The ID for this indicator", "An ID that should be unique, but used consistently by this program and its indicator.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (pClass, PROP_ID, pSpecId);

    /**
     * AppIndicator:category:
     *
     * The type of indicator that this represents. Please don't use 'Other'.
     * Defaults to 'ApplicationStatus'.
     */
    GParamSpec *pSpecCategory = g_param_spec_string ("category", "Indicator Category", "The type of indicator that this represents. Please don't use 'other'. Defaults to 'ApplicationStatus'.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_property (pClass, PROP_CATEGORY, pSpecCategory);

    /**
     * AppIndicator:status:
     *
     * Whether the indicator is shown or requests attention. Can be one of
     * 'Passive' (the indicator should not be shown), 'Active' (the indicator
     * should be shown in its default state), and 'Attention' (the indicator
     * should now show it's attention icon). Defaults to 'Passive'.
     */
    GParamSpec *pSpecStatus = g_param_spec_string ("status", "Indicator Status", "Whether the indicator is shown or requests attention. Can be one of 'Passive' (the indicator should not be shown), 'Active' (the indicator should be shown in its default state), and 'Attention' (the indicator should now show it's attention icon). Defaults to 'Passive'.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_STATUS, pSpecStatus);

    /**
     * AppIndicator:icon-name:
     *
     * The name of the regular icon that is shown for the indicator.
     */
    GParamSpec *pSpecIconName = g_param_spec_string ("icon-name", "An icon for the indicator", "The default icon that is shown for the indicator.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_ICON_NAME, pSpecIconName);

    /**
     * AppIndicator:icon-desc:
     *
     * The description of the regular icon that is shown for the indicator.
     */
    GParamSpec *pSpecIconDesc = g_param_spec_string ("icon-desc", "A description of the icon for the indicator", "A description of the default icon that is shown for the indicator.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_ICON_DESC, pSpecIconDesc);

    /**
     * AppIndicator:attention-icon-name:
     *
     * If the indicator sets its status to %APP_INDICATOR_STATUS_ATTENTION
     * then this icon is shown.
     */
    GParamSpec *pSpecAttentionIconName = g_param_spec_string ("attention-icon-name", "An icon to show when the indicator request attention.", "If the indicator sets it's status to 'attention' then this icon is shown.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_ATTENTION_ICON_NAME, pSpecAttentionIconName);

    /**
     * AppIndicator:attention-icon-desc:
     *
     * If the indicator sets its status to %APP_INDICATOR_STATUS_ATTENTION
     * then this is the textual description of the icon shown.
     */
    GParamSpec *pSpecAttentionIconDesc = g_param_spec_string ("attention-icon-desc", "A description of the icon to show when the indicator request attention.", "When the indicator is an attention mode this should describe the icon shown", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_ATTENTION_ICON_DESC, pSpecAttentionIconDesc);

    /**
     * AppIndicator:icon-theme-path:
     *
     * An additional place to look for icon names that may be installed by the
     * application.
     */
    GParamSpec *pSpecIconThemePath = g_param_spec_string ("icon-theme-path", "An additional path for custom icons.", "An additional place to look for icon names that may be installed by the application.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
    g_object_class_install_property (pClass, PROP_ICON_THEME_PATH, pSpecIconThemePath);

    /**
     * AppIndicator:connected:
     *
     * Whether we're conneced to a watcher. %TRUE if we have a
     * reasonable expectation of being displayed through this object.
     */
    GParamSpec *pSpecConnected = g_param_spec_boolean ("connected", "Whether we're conneced to a watcher", "Pretty simple, true if we have a reasonable expectation of being displayed through this object. You should hide your TrayIcon if so.", FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_CONNECTED, pSpecConnected);

    /**
     * AppIndicator:label:
     *
     * A label that can be shown next to the string in the application
     * indicator. The label will not be shown unless there is an icon
     * as well. The label is useful for numerical and other frequently
     * updated information. In general, it shouldn't be shown unless a
     * user requests it as it can take up a significant amount of space
     * on the user's panel. This may not be shown in all visualizations.
     */
    GParamSpec *pSpecLabel = g_param_spec_string ("label", "A label next to the icon", "A label to provide dynamic information.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_LABEL, pSpecLabel);

    /**
     * AppIndicator:label-guide:
     *
     * An optional string to provide guidance to the panel on how big
     * the #AppIndicator:label string could get. If this is set
     * correctly then the panel should never 'jiggle' as the string
     * adjusts throughout the range of options. For instance, if you
     * were providing a percentage like "54% thrust" in
     * #AppIndicator:label you'd want to set this string to
     * "100% thrust" to ensure space when Scotty can get you enough
     * power.
     */
    GParamSpec *pSpecLabelGuide = g_param_spec_string ("label-guide", "A string to size the space available for the label.", "To ensure that the label does not cause the panel to 'jiggle' this string should provide information on how much space it could take.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_LABEL_GUIDE, pSpecLabelGuide);

    /**
     * AppIndicator:ordering-index:
     *
     * The ordering index is an odd parameter, and if you think you
     * don't need it you're probably right. In general, the application
     * indicator service will try to place the indicators in a
     * recreatable place taking into account which category they're in
     * to try and group them. But, there are some cases when you'd want
     * to ensure indicators are next to each other. To do that you can
     * override the generated ordering index and replace it with a new
     * one. Again, you probably don't want to do this, but in case you
     * do, this is the way.
     */
    GParamSpec *pSpecOrderingIndex = g_param_spec_uint ("ordering-index", "The location that this app indicator should be in the list.", "A way to override the default ordering of the applications by providing a very specific idea of where this entry should be placed.", 0, G_MAXUINT32, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_ORDERING_INDEX, pSpecOrderingIndex);

    /**
     * AppIndicator:title:
     *
     * Provides a way to refer to this application indicator in a human
     * readable form.
     */
    GParamSpec *pSpecTitle = g_param_spec_string ("title", "Title of the application indicator", "A human readable way to refer to this application indicator in the UI.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_TITLE, pSpecTitle);

    /**
     * AppIndicator:menu:
     *
     * The menu that should be shown when the Application Indicator
     * is clicked on in the panel.
     */
    GParamSpec *pSpecMenu = g_param_spec_string ("menu", "The menu of the application indicator", "The menu that should be shown when the Application Indicator is clicked on in the panel.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_MENU, pSpecMenu);

    /**
     * AppIndicator:actions:
     *
     * The action group that is associated with the menu items.
     */
    GParamSpec *pSpecActions = g_param_spec_string ("actions", "The action group of the application indicator", "The action group that is associated with the menu items.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_ACTIONS, pSpecActions);

    /**
     * AppIndicator:tooltip-icon-name:
     *
     * The name of the tooltip's themed icon.
     */
    GParamSpec *pSpecTooltipIconName = g_param_spec_string ("tooltip-icon-name", "Tooltip icon", "The name of the tooltip's themed icon.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_TOOLTIP_ICON, pSpecTooltipIconName);

    /**
     * AppIndicator:tooltip-title:
     *
     * The title of the indicator's tooltip.
     */
    GParamSpec *pSpecTooltipTitle = g_param_spec_string ("tooltip-title", "The tooltip's title", "The title of the indicator's tooltip.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_TOOLTIP_TITLE, pSpecTooltipTitle);

    /**
     * AppIndicator:tooltip-description:
     *
     * The text of the indicator's tooltip.
     */
    GParamSpec *pSpecTooltipDescription = g_param_spec_string ("tooltip-description", "The tooltip's text", "The text of the indicator's tooltip.", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
    g_object_class_install_property (pClass, PROP_TOOLTIP_DESCRIPTION, pSpecTooltipDescription);

    /**
     * AppIndicator::new-icon:
     * @arg0: The #AppIndicator object
     *
     * Emitted when #AppIndicator:icon-name has changed.
     */
    signals[NEW_ICON] = g_signal_new ("new-icon", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, new_icon), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    /**
     * AppIndicator::new-attention-icon:
     * @arg0: The #AppIndicator object
     *
     * Emitted when #AppIndicator:attention-icon-name has changed.
     */
    signals[NEW_ATTENTION_ICON] = g_signal_new ("new-attention-icon", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, new_attention_icon), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    /**
     * AppIndicator::new-status:
     * @arg0: The #AppIndicator object
     * @arg1: The string value of the #AppIndicatorStatus enum
     *
     * Emitted when #AppIndicator:status has changed.
     */
    signals[NEW_STATUS] = g_signal_new ("new-status", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, new_status), NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

    /**
     * AppIndicator::new-label:
     * @arg0: The #AppIndicator object
     * @arg1: The string for the label
     * @arg2: The string for the guide
     *
     * Emitted when either #AppIndicator:label or
     * #AppIndicator:label-guide have changed.
    */
    signals[NEW_LABEL] = g_signal_new ("new-label", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, new_label), NULL, NULL, _application_service_marshal_VOID__STRING_STRING, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

    /**
     * AppIndicator::new-tooltip:
     * @arg0: The #AppIndicator object
     *
     * Emitted when #AppIndicator:tooltip-icon-name,
     * #AppIndicator:tooltip-title or #AppIndicator:tooltip-description have
     * changed.
     */
    signals[NEW_TOOLTIP] = g_signal_new ("new-tooltip", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, new_tooltip), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    /**
     * AppIndicator::connection-changed:
     * @arg0: The #AppIndicator object
     * @arg1: Whether we're connected or not
     *
     * Emitted when we connect to a watcher, or when it drops away.
     */
    signals[CONNECTION_CHANGED] = g_signal_new ("connection-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, connection_changed), NULL, NULL, g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    /**
     * AppIndicator::new-icon-theme-path:
     * @arg0: The #AppIndicator object
     * @arg1: The icon theme path
     *
     * Emittod when there is a new icon set for the object.
     */
    signals[NEW_ICON_THEME_PATH] = g_signal_new ("new-icon-theme-path", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, new_icon_theme_path), NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

    /**
     * AppIndicator::scroll-event:
     * @arg0: The #AppIndicator object
     * @arg1: How many steps the scroll wheel has taken
     * @arg2: Which direction the wheel went in
     *
     * Signaled when the #AppIndicator receives a scroll event.
     */
    signals[SCROLL_EVENT] = g_signal_new ("scroll-event", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (AppIndicatorClass, scroll_event), NULL, NULL, _application_service_marshal_VOID__INT_UINT, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_UINT);

    if (!m_pItemNodeInfo)
    {
        GError *pError = NULL;
        gchar *sXml = NULL;
        g_file_get_contents (SOURCE_DIR "/notification-item.xml", &sXml, NULL, &pError);

        if (pError)
        {
            g_error ("Error reading file: %s", pError->message);
            g_error_free (pError);
        }

        m_pItemNodeInfo = g_dbus_node_info_new_for_xml (sXml, &pError);
        g_free (sXml);

        if (pError)
        {
            g_error ("Unable to parse Notification Item DBus interface: %s", pError->message);
            g_error_free (pError);
        }
    }

    if (!m_pItemInterfaceInfo && m_pItemNodeInfo)
    {
        m_pItemInterfaceInfo = g_dbus_node_info_lookup_interface (m_pItemNodeInfo, NOTIFICATION_ITEM_DBUS_IFACE);

        if (!m_pItemInterfaceInfo)
        {
            g_error ("Unable to find interface '" NOTIFICATION_ITEM_DBUS_IFACE "'");
        }
    }

    if (!m_pWatcherNodeInfo)
    {
        GError *pError = NULL;
        gchar *sXml = NULL;
        g_file_get_contents (SOURCE_DIR "/notification-watcher.xml", &sXml, NULL, &pError);

        if (pError)
        {
            g_error ("Error reading file: %s", pError->message);
            g_error_free (pError);
        }

        m_pWatcherNodeInfo = g_dbus_node_info_new_for_xml (sXml, &pError);
        g_free (sXml);

        if (pError)
        {
            g_error ("Unable to parse Notification Item DBus interface: %s", pError->message);
            g_error_free (pError);
        }
    }

    if (!m_pWatcherInterfaceInfo && m_pWatcherNodeInfo)
    {
        m_pWatcherInterfaceInfo = g_dbus_node_info_lookup_interface (m_pWatcherNodeInfo, NOTIFICATION_WATCHER_DBUS_IFACE);

        if (!m_pWatcherInterfaceInfo)
        {
            g_error ("Unable to find interface '" NOTIFICATION_WATCHER_DBUS_IFACE "'");
        }
    }
}

static const gchar* getSnapPrefix ()
{
    const gchar *sSnap = g_getenv ("SNAP");

    return (sSnap && *sSnap != '\0') ? sSnap : NULL;
}

static gchar* appendSnapPrefix (const gchar *path)
{
    const gchar *sSnap = getSnapPrefix ();

    if (sSnap && path)
    {
        g_autofree gchar *sCanonPath = realpath (path, NULL);
        gboolean bPrefix = g_str_has_prefix (sCanonPath, "/tmp/");

        if (bPrefix)
        {
            g_warning ("Using '/tmp' paths in SNAP environment will lead to unreadable resources");

            return NULL;
        }

        gboolean bSnapPrefix = g_str_has_prefix (sCanonPath, sSnap);
        const gchar *sHome = g_get_home_dir ();
        gboolean bHomePrefix = g_str_has_prefix (sCanonPath, sHome);
        const gchar *sCache = g_get_user_cache_dir ();
        gboolean bCachePrefix = g_str_has_prefix (sCanonPath, sCache);
        const gchar *sConfig = g_get_user_config_dir ();
        gboolean bConfigPrefix = g_str_has_prefix (sCanonPath, sConfig);
        const gchar *sData = g_get_user_data_dir ();
        gboolean bDataPrefix = g_str_has_prefix (sCanonPath, sData);
        const gchar *sRuntime = g_get_user_runtime_dir ();
        gboolean bRuntimePrefix = g_str_has_prefix (sCanonPath, sRuntime);

        if (bSnapPrefix || bHomePrefix || bCachePrefix || bConfigPrefix || bDataPrefix || bRuntimePrefix)
        {
            return g_strdup (sCanonPath);
        }

        for (gint nDir = 0; nDir < G_USER_N_DIRECTORIES; ++nDir)
        {
            const gchar *sDir = g_get_user_special_dir (nDir);
            gboolean bPrefix = g_str_has_prefix (sCanonPath, sDir);

            if (bPrefix)
            {
                return g_strdup (sCanonPath);
            }
        }

        gchar *sPath = g_build_path (G_DIR_SEPARATOR_S, sSnap, sCanonPath, NULL);

        return sPath;
    }

    return NULL;
}

static gchar* getRealThemePath (AppIndicator *self)
{
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    const gchar *sThemePath = pPrivate->sIconThemePath;
    gchar *sSnappedPath = appendSnapPrefix (sThemePath);

    if (sSnappedPath)
    {
        return sSnappedPath;
    }
    else
    {
        const gchar *sPrefix = getSnapPrefix ();

        if (sPrefix)
        {
            const gchar *sDir = g_get_user_data_dir ();
            gchar *sPath = g_build_path (G_DIR_SEPARATOR_S, sDir, "icons", NULL);

            return sPath;
        }
    }

    return NULL;
}

static void onBusAcquired (GObject *pObject, GAsyncResult *pResult, gpointer pData)
{
    GError *pError = NULL;
    GDBusConnection *pConnection = g_bus_get_finish (pResult, &pError);

    if (pError)
    {
        g_warning ("Unable to get the session bus: %s", pError->message);
        g_error_free (pError);
        g_object_unref (G_OBJECT (pData));

        return;
    }

    AppIndicator *self = APP_INDICATOR (pData);
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    pPrivate->pConnection = pConnection;

    // If the connection was blocking the exporting of the object this function will export everything
    getConnectable (self);
    g_object_unref (G_OBJECT (self));
}

static void app_indicator_init (AppIndicator *self)
{
    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    pPrivate->sId = NULL;
    pPrivate->sCleanId = NULL;
    pPrivate->nCategory = APP_INDICATOR_CATEGORY_OTHER;
    pPrivate->nStatus = APP_INDICATOR_STATUS_PASSIVE;
    pPrivate->sIconName = NULL;
    pPrivate->sAttentionIconName = NULL;
    pPrivate->sIconThemePath = NULL;
    pPrivate->sAbsoluteIconThemePath = getRealThemePath (self);
    pPrivate->nOrderingIndex = 0;
    pPrivate->sTitle = NULL;
    pPrivate->sLabel = NULL;
    pPrivate->sLabelGuide = NULL;
    pPrivate->nLabelChangeIdle = 0;
    pPrivate->pConnection = NULL;
    pPrivate->nDbusRegistration = 0;
    pPrivate->sPath = NULL;
    pPrivate->pMenu = NULL;
    pPrivate->pActions = NULL;
    pPrivate->nActionsId = 0;
    pPrivate->nMenuId = 0;
    pPrivate->sSecActivateTarget = NULL;
    pPrivate->pWatcherProxy = NULL;
    pPrivate->nWatcherId = g_bus_watch_name (G_BUS_TYPE_SESSION, NOTIFICATION_WATCHER_DBUS_ADDR, G_BUS_NAME_WATCHER_FLAGS_NONE, (GBusNameAppearedCallback) onNameAppeared, (GBusNameVanishedCallback) onNameVanished, self, NULL);
    pPrivate->sTooltipIcon = NULL;
    pPrivate->sTooltipTitle = NULL;
    pPrivate->sTooltipDescription = NULL;

    // Start getting the session bus
    g_object_ref (self);
    g_bus_get (G_BUS_TYPE_SESSION, NULL, onBusAcquired, self);
}

static const gchar* categoryFromEnum (AppIndicatorCategory category)
{
    GEnumValue *pValue = g_enum_get_value ((GEnumClass*) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_CATEGORY), category);

    return pValue->value_nick;
}

/**
 * app_indicator_new:
 * @id: The unique id of the indicator to create
 * @icon_name: The icon name for this indicator
 * @category: The category of the indicator
 *
 * Creates a new #AppIndicator setting the properties:
 * #AppIndicator:id with @id, #AppIndicator:category with @category
 * and #AppIndicator:icon-name with @icon_name.
 *
 * Return value: A pointer to a new #AppIndicator object
 */
AppIndicator *app_indicator_new (const gchar *id, const gchar *icon_name, AppIndicatorCategory category)
{
    const gchar *sCategory = categoryFromEnum (category);
    AppIndicator *pIndicator = g_object_new (APP_INDICATOR_TYPE, "id", id, "category", sCategory, "icon-name", icon_name, NULL);

    return pIndicator;
}

/**
 * app_indicator_new_with_path:
 * @id: The unique id of the indicator to create
 * @icon_name: The icon name for this indicator
 * @category: The category of the indicator.
 * @icon_theme_path: A custom path for finding icons

 * Creates a new #AppIndicator setting the properties:
 * #AppIndicator:id with @id, #AppIndicator:category with @category,
 * #AppIndicator:icon-name with @icon_name and #AppIndicator:icon-theme-path
 * with @icon_theme_path.
 *
 * Return value: A pointer to a new #AppIndicator object
 */
AppIndicator *app_indicator_new_with_path (const gchar *id, const gchar *icon_name, AppIndicatorCategory category, const gchar *icon_theme_path)
{
    const gchar *sCategory = categoryFromEnum (category);
    AppIndicator *pIndicator = g_object_new (APP_INDICATOR_TYPE, "id", id, "category", sCategory, "icon-name", icon_name, "icon-theme-path", icon_theme_path, NULL);

    return pIndicator;
}

/**
 * app_indicator_set_status:
 * @self: The #AppIndicator object to use
 * @status: The status to set for this indicator
 *
 * Wrapper function for property #AppIndicator:status
 */
void app_indicator_set_status (AppIndicator *self, AppIndicatorStatus status)
{
    g_return_if_fail (APP_IS_INDICATOR (self));

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    if (pPrivate->nStatus != status)
    {
        GEnumValue *pValue = g_enum_get_value ((GEnumClass*) g_type_class_ref (APP_INDICATOR_TYPE_INDICATOR_STATUS), status);
        pPrivate->nStatus = status;
        g_signal_emit (self, signals[NEW_STATUS], 0, pValue->value_nick);

        if (pPrivate->nDbusRegistration != 0 && pPrivate->pConnection)
        {
            GError *pError = NULL;
            GVariant *pStatus = g_variant_new ("(s)", pValue->value_nick);
            g_dbus_connection_emit_signal (pPrivate->pConnection, NULL, pPrivate->sPath, NOTIFICATION_ITEM_DBUS_IFACE, "NewStatus", pStatus, &pError);

            if (pError)
            {
                g_warning ("Unable to send signal for NewStatus: %s", pError->message);
                g_error_free (pError);
            }
        }
    }
}

/**
 * app_indicator_set_attention_icon:
 * @self: The #AppIndicator object to use
 * @icon_name: The name of the attention icon to set for this indicator
 * @icon_desc: (nullable): A textual description of the icon
 *
 * Wrapper function for property #AppIndicator:attention-icon-name
 */
void app_indicator_set_attention_icon (AppIndicator *self, const gchar *icon_name, const gchar *icon_desc)
{
    g_return_if_fail (APP_IS_INDICATOR (self));
    g_return_if_fail (icon_name != NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    gboolean bChanged = FALSE;
    gint nResult = g_strcmp0 (pPrivate->sAttentionIconName, icon_name);

    if (nResult != 0)
    {
        g_free (pPrivate->sAttentionIconName);
        pPrivate->sAttentionIconName = g_strdup (icon_name);
        g_free (pPrivate->sAbsoluteAttentionIconName);
        pPrivate->sAbsoluteAttentionIconName = NULL;

        if (icon_name && icon_name[0] == '/')
        {
            pPrivate->sAbsoluteAttentionIconName = appendSnapPrefix (icon_name);
        }

        bChanged = TRUE;
    }

    nResult = g_strcmp0 (pPrivate->sAttAccessibleDesc, icon_desc);

    if (nResult != 0)
    {
        g_free (pPrivate->sAttAccessibleDesc);
        pPrivate->sAttAccessibleDesc = g_strdup (icon_desc);
        bChanged = TRUE;
    }

    if (bChanged)
    {
        g_signal_emit (self, signals[NEW_ATTENTION_ICON], 0);

        if (pPrivate->nDbusRegistration != 0 && pPrivate->pConnection)
        {
            GError *pError = NULL;
            g_dbus_connection_emit_signal(pPrivate->pConnection, NULL, pPrivate->sPath, NOTIFICATION_ITEM_DBUS_IFACE, "NewAttentionIcon", NULL, &pError);

            if (pError)
            {
                g_warning ("Unable to send signal for NewAttentionIcon: %s", pError->message);
                g_error_free (pError);
            }
        }
    }
}

/**
 * app_indicator_set_icon:
 * @self: The #AppIndicator object to use
 * @icon_name: The icon name to set
 * @icon_desc: (nullable): A description of the icon for accessibility
 *
 * Sets the default icon to use when the status is active but not set to
 * attention. In most cases, this should be the application icon for the
 * program.
 *
 * Wrapper function for property #AppIndicator:icon-name and
 * #AppIndicator:icon-desc
 */
void app_indicator_set_icon (AppIndicator *self, const gchar *icon_name, const gchar *icon_desc)
{
    g_return_if_fail (APP_IS_INDICATOR (self));
    g_return_if_fail (icon_name != NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    gboolean bChanged = FALSE;
    gint nResult = g_strcmp0 (pPrivate->sIconName, icon_name);

    if (nResult != 0)
    {
        if (pPrivate->sIconName)
        {
            g_free (pPrivate->sIconName);
        }

        pPrivate->sIconName = g_strdup (icon_name);
        g_free (pPrivate->sAbsoluteIconName);
        pPrivate->sAbsoluteIconName = NULL;

        if (icon_name && icon_name[0] == '/')
        {
            pPrivate->sAbsoluteIconName = appendSnapPrefix (icon_name);
        }

        bChanged = TRUE;
    }

    nResult = g_strcmp0 (pPrivate->sAccessibleDesc, icon_desc);

    if (nResult != 0)
    {
        if (pPrivate->sAccessibleDesc)
        {
            g_free (pPrivate->sAccessibleDesc);
        }

        pPrivate->sAccessibleDesc = g_strdup (icon_desc);
        bChanged = TRUE;
    }

    if (bChanged)
    {
        g_signal_emit (self, signals[NEW_ICON], 0);

        if (pPrivate->nDbusRegistration != 0 && pPrivate->pConnection != NULL)
        {
            GError *pError = NULL;
            g_dbus_connection_emit_signal (pPrivate->pConnection, NULL, pPrivate->sPath, NOTIFICATION_ITEM_DBUS_IFACE, "NewIcon", NULL, &pError);

            if (pError != NULL)
            {
                g_warning ("Unable to send signal for NewIcon: %s", pError->message);
                g_error_free (pError);
            }
        }
    }
}

/**
 * app_indicator_set_label:
 * @self: The #AppIndicator object to use
 * @label: The label to show next to the icon
 * @guide: A guide to size the label correctly
 *
 * This is a wrapper function for the #AppIndicator:label and
 * #AppIndicator:label-guide properties. This function can take #NULL
 * as either @label or @guide and will clear the entries.
*/
void app_indicator_set_label (AppIndicator *self, const gchar *label, const gchar *guide)
{
    g_return_if_fail (APP_IS_INDICATOR (self));

    g_object_set(G_OBJECT(self), "label", label == NULL ? "" : label, "label-guide", guide == NULL ? "" : guide, NULL);
}

/**
 * app_indicator_set_icon_theme_path:
 * @self: The #AppIndicator object to use
 * @icon_theme_path: The icon theme path to set
 *
 * Sets the path to use when searching for icons.
 */
void app_indicator_set_icon_theme_path (AppIndicator *self, const gchar *icon_theme_path)
{
    g_return_if_fail (APP_IS_INDICATOR (self));

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    gint nResult = g_strcmp0 (pPrivate->sIconThemePath, icon_theme_path);

    if (nResult != 0)
    {
        if (pPrivate->sIconThemePath)
        {
            g_free (pPrivate->sIconThemePath);
        }

        pPrivate->sIconThemePath = g_strdup (icon_theme_path);
        g_free (pPrivate->sAbsoluteIconThemePath);
        pPrivate->sAbsoluteIconThemePath = getRealThemePath (self);
        g_signal_emit (self, signals[NEW_ICON_THEME_PATH], 0, pPrivate->sIconThemePath);

        if (pPrivate->nDbusRegistration != 0 && pPrivate->pConnection)
        {
            const gchar *sThemePath = pPrivate->sAbsoluteIconThemePath ? pPrivate->sAbsoluteIconThemePath : pPrivate->sIconThemePath;
            GError *pError = NULL;
            GVariant *pThemePath = g_variant_new ("(s)", sThemePath ? sThemePath : "");
            g_dbus_connection_emit_signal (pPrivate->pConnection, NULL, pPrivate->sPath, NOTIFICATION_ITEM_DBUS_IFACE, "NewIconThemePath", pThemePath, &pError);

            if (pError)
            {
                g_warning ("Unable to send signal for NewIconThemePath: %s", pError->message);
                g_error_free (pError);
            }
        }
    }
}

/**
 * app_indicator_set_menu:
 * @self: The #AppIndicator
 * @menu: (allow-none): A `GMenu` to set
 *
 * Sets the menu that should be shown when the Application Indicator
 * is activated in the panel. An application indicator will not be
 * rendered unless it has a menu.
 *
 * ::: important
 *     All menu item actions must be prefixed with the ``indicator.``
 *     namespace:
 *
 *     GSimpleaction *pAction = g_simple_action_new ("newimage", NULL);
 *
 *     ...
 *
 *     GMenuItem *pItem = g_menu_item_new ("New Image", "indicator.newimage");
 *
 *     ...
 *
 * Wrapper function for property #AppIndicator:menu.
 */
void app_indicator_set_menu (AppIndicator *self, GMenu *pMenu)
{
    g_return_if_fail (APP_IS_INDICATOR (self));
    g_return_if_fail (G_IS_MENU (pMenu));

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    g_clear_object (&pPrivate->pMenu);
    pPrivate->pMenu = pMenu;
    g_object_ref (pPrivate->pMenu);
    getConnectable (self);
}

/**
 * app_indicator_set_actions:
 * @self: The #AppIndicator
 * @actions: A `GSimpleActionGroup` to set
 *
 * Sets the action group that will be associated with the menu items. An
 * application indicator will not be rendered unless it has actions.
 *
 * Wrapper function for property #AppIndicator:actions.
 */
void app_indicator_set_actions (AppIndicator *self, GSimpleActionGroup *actions)
{
    g_return_if_fail (APP_IS_INDICATOR (self));
    g_return_if_fail (G_IS_SIMPLE_ACTION_GROUP (actions));

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    g_clear_object (&pPrivate->pActions);
    pPrivate->pActions = actions;
    g_object_ref (pPrivate->pActions);
    getConnectable (self);
}

/**
 * app_indicator_set_ordering_index:
 * @self: The #AppIndicator
 * @ordering_index: A value for the ordering of this app indicator
 *
 * Sets the ordering index for the app indicator which affects its
 * placement on the panel. For almost all app indicators this is not the
 * function you're looking for.
 *
 * Wrapper function for property #AppIndicator:ordering-index.
 */
void app_indicator_set_ordering_index (AppIndicator *self, guint32 ordering_index)
{
    g_return_if_fail (APP_IS_INDICATOR (self));

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    pPrivate->nOrderingIndex = ordering_index;
}

/**
 * app_indicator_set_secondary_activate_target:
 * @self: The #AppIndicator
 * @action: (allow-none): The action to be activated on secondary activation
 *
 * Set the @action to be activated when a secondary activation event
 * (i.e. a middle-click) is emitted over the #AppIndicator icon/label.
 *
 * For the @action to get activated when a secondary activation occurs
 * in the #AppIndicator, it must be defined as a stateless #GAction with
 * no parameter, it needs to be enabled and associated with an item of
 * the #AppIndicator:menu.
 *
 * Setting @action to %NULL disables this feature.
 */
void app_indicator_set_secondary_activate_target (AppIndicator *self, const gchar *action)
{
    g_return_if_fail (APP_IS_INDICATOR (self));

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    if (pPrivate->sSecActivateTarget)
    {
        g_free (pPrivate->sSecActivateTarget);
        pPrivate->sSecActivateTarget = NULL;
    }

    if (action == NULL)
    {
        return;
    }

    pPrivate->sSecActivateTarget = g_strdup (action);
}

/**
 * app_indicator_set_title:
 * @self: The #AppIndicator
 * @title: (allow-none): Title of the app indicator
 *
 * Sets the title of the application indicator, or how it should be
 * referred to in a human readable form. This string should be UTF-8 and
 * localized, as it is expected that users will set it.
 *
 * Setting @title to %NULL removes the title.
 */
void app_indicator_set_title (AppIndicator *self, const gchar *title)
{
    g_return_if_fail (APP_IS_INDICATOR (self));

    g_object_set (G_OBJECT(self), "title", title == NULL ? "": title, NULL);
}

/**
 * app_indicator_set_tooltip:
 * @self: The #AppIndicator
 * @icon_name: (allow-none): The name of the tooltip's themed icon
 * @title: The tooltip's title
 * @description: (allow-none): The tooltip's detailed description
 *
 * If @icon_name is %NULL, the tooltip will not have an icon.
 * If @title is %NULL, the indicator will not have a tooltip.
 * If @description is %NULL, the tooltip will not have a description.
 *
 * Sets the #AppIndicator:tooltip-icon-name, #AppIndicator:tooltip-title and
 * #AppIndicator:tooltip-description properties.
 */
void app_indicator_set_tooltip (AppIndicator *self, const gchar *icon_name, const gchar *title, const gchar *description)
{
    g_return_if_fail (APP_IS_INDICATOR (self));

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);
    const gchar *sIconName = NULL;
    const gchar *sTitle = NULL;

    if (description)
    {
        sIconName = icon_name;
        sTitle = title;
    }

    gboolean bChanged = FALSE;
    gint nResult = g_strcmp0 (pPrivate->sTooltipDescription, description);

    if (nResult != 0)
    {
        if (pPrivate->sTooltipDescription)
        {
            g_free (pPrivate->sTooltipDescription);
            pPrivate->sTooltipDescription = NULL;
        }

        if (description)
        {
            pPrivate->sTooltipDescription = g_strdup (description);
        }

        bChanged = TRUE;
    }

    nResult = g_strcmp0 (pPrivate->sTooltipIcon, sIconName);

    if (nResult != 0)
    {
        if (pPrivate->sTooltipIcon)
        {
            g_free (pPrivate->sTooltipIcon);
            pPrivate->sTooltipIcon = NULL;
        }

        if (sIconName)
        {
            pPrivate->sTooltipIcon = g_strdup (sIconName);
        }

        bChanged = TRUE;
    }

    nResult = g_strcmp0 (pPrivate->sTooltipTitle, sTitle);

    if (nResult != 0)
    {
        if (pPrivate->sTooltipTitle)
        {
            g_free (pPrivate->sTooltipTitle);
            pPrivate->sTooltipTitle = NULL;
        }

        if (sTitle)
        {
            pPrivate->sTooltipTitle = g_strdup (sTitle);
        }

        bChanged = TRUE;
    }

    if (bChanged)
    {
        g_signal_emit (self, signals[NEW_TOOLTIP], 0);

        if (pPrivate->nDbusRegistration != 0 && pPrivate->pConnection != NULL)
        {
            GError *pError = NULL;
            g_dbus_connection_emit_signal (pPrivate->pConnection, NULL, pPrivate->sPath, NOTIFICATION_ITEM_DBUS_IFACE, "NewToolTip", NULL, &pError);

            if (pError != NULL)
            {
                g_warning ("Unable to send signal for NewToolTip: %s", pError->message);
                g_error_free (pError);
            }
        }
    }
}

/**
 * app_indicator_get_id:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:id.
 *
 * Return value: The current ID
 */
const gchar* app_indicator_get_id (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sId;
}

/**
 * app_indicator_get_category:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:category.
 *
 * Return value: The current category
 */
AppIndicatorCategory app_indicator_get_category (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->nCategory;
}

/**
 * app_indicator_get_status:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:status.
 *
 * Return value: The current status
 */
AppIndicatorStatus app_indicator_get_status (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), APP_INDICATOR_STATUS_PASSIVE);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->nStatus;
}

/**
 * app_indicator_get_icon:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:icon-name.
 *
 * Return value: The current icon name
 */
const gchar* app_indicator_get_icon (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sIconName;
}

/**
 * app_indicator_get_icon_desc:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:icon-desc.
 *
 * Return value: The current icon description
*/
const gchar* app_indicator_get_icon_desc (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sAccessibleDesc;
}

/**
 * app_indicator_get_icon_theme_path:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:icon-theme-path.
 *
 * Return value: The current icon theme path
 */
const gchar* app_indicator_get_icon_theme_path (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sIconThemePath;
}

/**
 * app_indicator_get_attention_icon:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:attention-icon-name.
 *
 * Return value: The current attention icon name
 */
const gchar* app_indicator_get_attention_icon (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sAttentionIconName;
}

/**
 * app_indicator_get_attention_icon_desc:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:attention-icon-desc.
 *
 * Return value: The current attention icon description
 */
const gchar* app_indicator_get_attention_icon_desc (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sAttAccessibleDesc;
}

/**
 * app_indicator_get_title:
 * @self: The #AppIndicator object to use
 *
 * Gets the title of the application indicator. See the function
 * app_indicator_set_title () for information on the title.
 *
 * Return value: The current title
 *
 */
const gchar* app_indicator_get_title (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sTitle;
}

/**
 * app_indicator_get_menu:
 * @self: The #AppIndicator object to use
 *
 * Gets the menu being used for this application indicator.
 *
 * Wrapper function for property #AppIndicator:menu.
 *
 * Returns: (transfer none): The `GMenu` object or %NULL if it hasn't
 * been set
 */
GMenu* app_indicator_get_menu (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->pMenu;
}

/**
 * app_indicator_get_actions:
 * @self: The #AppIndicator object to use
 *
 * Gets the action group associated with the menu items.
 *
 * Wrapper function for property #AppIndicator:actions.
 *
 * Returns: (transfer none): The `GSimpleActionGroup` object or %NULL if
 * it hasn't been set
 */
GSimpleActionGroup* app_indicator_get_actions (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->pActions;
}

/**
 * app_indicator_get_label:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:label.
 *
 * Return value: The current label
 */
const gchar* app_indicator_get_label (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sLabel;
}

/**
 * app_indicator_get_label_guide:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:label-guide.
 *
 * Return value: The current label guide
 */
const gchar* app_indicator_get_label_guide (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sLabelGuide;
}

/**
 * app_indicator_get_ordering_index:
 * @self: The #AppIndicator object to use
 *
 * Wrapper function for property #AppIndicator:ordering-index.
 *
 * Return value: The current ordering index
 */
guint32 app_indicator_get_ordering_index (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), 0);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    if (pPrivate->nOrderingIndex == 0)
    {
        return generateId (pPrivate->nCategory, pPrivate->sId);
    }
    else
    {
        return pPrivate->nOrderingIndex;
    }
}

/**
 * app_indicator_get_secondary_activate_target:
 * @self: The #AppIndicator object to use
 *
 * Gets the action being activated on secondary-activate event.
 *
 * Returns: (transfer none): The action name or %NULL if none has been set.
 */
const gchar* app_indicator_get_secondary_activate_target (AppIndicator *self)
{
    g_return_val_if_fail (APP_IS_INDICATOR (self), NULL);

    AppIndicatorPrivate *pPrivate = app_indicator_get_instance_private (self);

    return pPrivate->sSecActivateTarget;
}
