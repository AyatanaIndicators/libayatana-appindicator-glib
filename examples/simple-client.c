/*
 * A small piece of sample code demonstrating a very simple application
 * with an indicator.
 *
 * Copyright 2009 Ted Gould <ted@canonical.com>
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

#include <ayatana-appindicator.h>

GMainLoop * m_pMainloop = NULL;
guint m_nPercentage = 0;
static gboolean m_bActive = TRUE;
static gboolean m_bHasLabel = TRUE;

typedef struct
{
    AppIndicator *pIndicator;
    gpointer pData;
} IndicatorData;

static void addSubmenu (GMenu *pMenu, gboolean bEnabled);

static void onLabelToggle (GSimpleAction *pAction, GVariant *pValue, gpointer pData)
{
    m_bHasLabel = !m_bHasLabel;
    IndicatorData *pIndicatorData = (IndicatorData*) pData;
    gint nPosition = GPOINTER_TO_INT (pIndicatorData->pData);
    gchar *sLabel = NULL;

    if (m_bHasLabel)
    {
        sLabel = "Hide label";
    }
    else
    {
        sLabel = "Show label";
    }

    GMenu *pMenu = app_indicator_get_menu (pIndicatorData->pIndicator);
    g_menu_remove (pMenu, nPosition);
    GMenuItem *pItem = g_menu_item_new (sLabel, "indicator.showlabel");
    g_menu_insert_item (pMenu, nPosition, pItem);
    g_object_unref (pItem);
}

static void onAttentionActivate (GSimpleAction *pAction, GVariant *pValue, gpointer pData)
{
    IndicatorData *pIndicatorData = (IndicatorData*) pData;
    GMenu *pMenu = app_indicator_get_menu (pIndicatorData->pIndicator);
    gint nPosition = GPOINTER_TO_INT (pIndicatorData->pData);
    g_menu_remove (pMenu, nPosition);
    GMenuItem *pItem = NULL;

    if (m_bActive)
    {
        app_indicator_set_status (pIndicatorData->pIndicator, APP_INDICATOR_STATUS_ATTENTION);
        pItem = g_menu_item_new ("I'm okay now", "indicator.attention");
        m_bActive = FALSE;
    }
    else
    {
        app_indicator_set_status (pIndicatorData->pIndicator, APP_INDICATOR_STATUS_ACTIVE);
        pItem = g_menu_item_new ("Get Attention", "indicator.attention");
        m_bActive = TRUE;
    }

    g_menu_insert_item (pMenu, nPosition, pItem);
    g_object_unref (pItem);
}

static void onLocalIcon (GSimpleAction *pAction, GVariant *pValue, gpointer pData)
{
    AppIndicator *pIndicator = APP_INDICATOR (pData);
    GSimpleActionGroup *pActions = app_indicator_get_actions (pIndicator);
    GAction *pCheckAction = g_action_map_lookup_action (G_ACTION_MAP (pActions), "check");
    GVariant *pState = g_action_get_state (pCheckAction);
    gboolean bActive = g_variant_get_boolean (pState);

    if (bActive)
    {
        app_indicator_set_icon (pIndicator, LOCAL_ICON, "Local Icon");
    }
    else
    {
        app_indicator_set_icon (pIndicator, "indicator-messages", "System Icon");
    }
}

static void onItemActivate (GSimpleAction *pAction, GVariant *pValue, gpointer pData)
{
    g_print ("%s clicked!\n", (const gchar*) pData);
}

static void onCheckActivate (GSimpleAction *pAction, GVariant *pValue, gpointer pData)
{
    GVariant *pState = g_action_get_state (G_ACTION (pAction));
    gboolean bActive = g_variant_get_boolean (pState);
    GVariant *pNewState = g_variant_new_boolean (!bActive);
    g_action_change_state (G_ACTION (pAction), pNewState);
    onItemActivate (NULL, NULL, pData);
}

static void onSensitivityActivate (GSimpleAction *pAction, GVariant *pValue, gpointer pData)
{
    IndicatorData *pIndicatorData = (IndicatorData*) pData;
    GSimpleActionGroup *pActions = app_indicator_get_actions (pIndicatorData->pIndicator);
    GAction *pSensitivityAction = g_action_map_lookup_action (G_ACTION_MAP (pActions), pIndicatorData->pData);
    gboolean bEnabled = g_action_get_enabled (pSensitivityAction);
    g_object_set (G_OBJECT (pSensitivityAction), "enabled", !bEnabled, NULL);
    gboolean bSub = g_str_equal (pIndicatorData->pData, "sub");

    if (bSub)
    {
        GMenu *pMenu = app_indicator_get_menu (pIndicatorData->pIndicator);
        g_menu_remove (pMenu, 2);
        addSubmenu (pMenu, !bEnabled);
    }
}

static void onImageActivate (GSimpleAction *pAction, GVariant *pValue, gpointer pData)
{
    IndicatorData *pIndicatorData = (IndicatorData*) pData;
    GMenu *pMenu = app_indicator_get_menu (pIndicatorData->pIndicator);
    gint nPosition = GPOINTER_TO_INT (pIndicatorData->pData);
    g_menu_remove (pMenu, nPosition);
    GMenuItem *pItem = g_menu_item_new ("New", "indicator.setimage");
    GIcon *pIcon = g_themed_icon_new_with_default_fallbacks ("document-open");
    g_menu_item_set_icon (pItem, pIcon);
    g_object_unref (pIcon);
    g_menu_insert_item (pMenu, nPosition, pItem);
    g_object_unref (pItem);
}

static void onScroll (AppIndicator *pIndicator, gint nDelta, guint nDirection, gpointer pData)
{
    g_print ("Got scroll event! delta: %d, direction: %d\n", nDelta, nDirection);
}

static gboolean onPercentChange (gpointer pData)
{
    m_nPercentage = (m_nPercentage + 1) % 100;

    if (m_bHasLabel)
    {
        gchar *sPercentage = g_strdup_printf ("%d%%", m_nPercentage + 1);
        app_indicator_set_label (APP_INDICATOR (pData), sPercentage, "100%");
        g_free (sPercentage);
    }
    else
    {
        app_indicator_set_label (APP_INDICATOR(pData), NULL, NULL);
    }

    return TRUE;
}

static void addSubmenu (GMenu *pMenu, gboolean bEnabled)
{
    GMenuItem *pMenuItem = g_menu_item_new ("3", "indicator.sub");

    if (bEnabled)
    {
        GMenu *pSubmenu = g_menu_new ();
        GMenuItem *pItem = g_menu_item_new ("Sub 1", "indicator.sub1");
        g_menu_append_item (pSubmenu, pItem);
        g_object_unref (pItem);
        pItem = g_menu_item_new ("Sub 2", "indicator.sub2");
        g_menu_append_item (pSubmenu, pItem);
        g_object_unref (pItem);
        pItem = g_menu_item_new ("Sub 3", "indicator.sub3");
        g_menu_append_item (pSubmenu, pItem);
        g_object_unref (pItem);
        g_menu_item_set_submenu (pMenuItem, G_MENU_MODEL (pSubmenu));
        g_object_unref (pSubmenu);
    }

    g_menu_insert_item (pMenu, 2, pMenuItem);
    g_object_unref (pMenuItem);
}

int main (int argc, char ** argv)
{
    AppIndicator *pIndicator = app_indicator_new ("example-simple-client", "indicator-messages", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_assert (APP_IS_INDICATOR (pIndicator));
    g_assert (G_IS_OBJECT (pIndicator));

    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_attention_icon (pIndicator, "indicator-messages-new", "System Messages Icon Highlighted");
    app_indicator_set_label (pIndicator, "1%", "100%");
    app_indicator_set_title (pIndicator, "Test Indicator (C)");
    app_indicator_set_tooltip (pIndicator, "indicator-messages-new", "tooltip-title", "tooltip-description");

    g_signal_connect (pIndicator, "scroll-event", G_CALLBACK (onScroll), NULL);
    g_timeout_add_seconds (1, onPercentChange, pIndicator);

    GMenu *pMenu = g_menu_new ();
    GSimpleActionGroup *pActions = g_simple_action_group_new ();

    GVariant *pCheck = g_variant_new_boolean (FALSE);
    GSimpleAction *pSimpleAction = g_simple_action_new_stateful ("check", NULL, pCheck);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onCheckActivate), "1");
    g_object_unref (pSimpleAction);
    GMenuItem *pItem = g_menu_item_new ("1", "indicator.check");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    GVariant *pRadio = g_variant_new_string ("2");
    pSimpleAction = g_simple_action_new_stateful ("radio", G_VARIANT_TYPE_STRING, pRadio);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onItemActivate), "2");
    g_object_unref (pSimpleAction);
    pItem = g_menu_item_new ("2", "indicator.radio::2");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    pSimpleAction = g_simple_action_new ("sub", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_object_unref (pSimpleAction);
    pSimpleAction = g_simple_action_new ("sub1", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onItemActivate), "Sub 1");
    g_object_unref (pSimpleAction);
    pSimpleAction = g_simple_action_new ("sub2", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    IndicatorData cSub2Data = {pIndicator, "sub1"};
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onSensitivityActivate), &cSub2Data);
    g_object_unref (pSimpleAction);
    pSimpleAction = g_simple_action_new ("sub3", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onItemActivate), "Sub 3");
    g_object_unref (pSimpleAction);
    addSubmenu (pMenu, TRUE);

    pSimpleAction = g_simple_action_new ("sensitivity", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    IndicatorData cSensitivityData = {pIndicator, "sub"};
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onSensitivityActivate), &cSensitivityData);
    g_object_unref (pSimpleAction);
    pItem = g_menu_item_new ("Toggle 3", "indicator.sensitivity");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    pSimpleAction = g_simple_action_new ("setimage", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    IndicatorData cSetImageData = {pIndicator, GINT_TO_POINTER (4)};
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onImageActivate), &cSetImageData);
    g_object_unref (pSimpleAction);
    pItem = g_menu_item_new ("New", "indicator.setimage");
    GIcon *pIcon = g_themed_icon_new_with_default_fallbacks ("document-new");
    g_menu_item_set_icon (pItem, pIcon);
    g_object_unref (pIcon);
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    pSimpleAction = g_simple_action_new ("attention", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    IndicatorData cAttentionData = {pIndicator, GINT_TO_POINTER (5)};
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onAttentionActivate), &cAttentionData);
    g_object_unref (pSimpleAction);
    pItem = g_menu_item_new ("Get Attention", "indicator.attention");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);
    app_indicator_set_secondary_activate_target (pIndicator, "attention");

    pSimpleAction = g_simple_action_new ("showlabel", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    IndicatorData cShowLabelData = {pIndicator, GINT_TO_POINTER (6)};
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onLabelToggle), &cShowLabelData);
    g_object_unref (pSimpleAction);
    pItem = g_menu_item_new ("Show label", "indicator.showlabel");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    pSimpleAction = g_simple_action_new ("setlocalicon", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_signal_connect (pSimpleAction, "activate", G_CALLBACK (onLocalIcon), pIndicator);
    g_object_unref (pSimpleAction);
    pItem = g_menu_item_new ("Set Local Icon", "indicator.setlocalicon");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    app_indicator_set_menu (pIndicator, pMenu);
    g_object_unref (pMenu);
    app_indicator_set_actions (pIndicator, pActions);
    g_object_unref (pActions);
    onLabelToggle (NULL, NULL, &cShowLabelData);
    m_pMainloop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (m_pMainloop);

    return 0;
}
