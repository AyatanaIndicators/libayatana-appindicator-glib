/*
 * Test that creates a small test app which links with libappindicator.
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

static GMainLoop *pMainLoop = NULL;

int main (int argc, char ** argv)
{
    GSimpleActionGroup *pActions = g_simple_action_group_new ();
    GSimpleAction *pSimpleAction = g_simple_action_new ("test", NULL);
    g_action_map_add_action (G_ACTION_MAP (pActions), G_ACTION (pSimpleAction));
    g_object_unref (pSimpleAction);
    GMenu *pMenu = g_menu_new ();
    GMenuItem *pItem = g_menu_item_new ("Test", "indicator.test");
    g_menu_append_item (pMenu, pItem);
    g_object_unref (pItem);

    AppIndicator *pIndicator = app_indicator_new ("test-application", "system-shutdown", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_menu (pIndicator, pMenu);
    g_object_unref (pMenu);
    app_indicator_set_actions (pIndicator, pActions);
    g_object_unref (pActions);

    pMainLoop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (pMainLoop);

    g_object_unref (G_OBJECT (pIndicator));

    g_debug("Quiting");

    return 0;
}
