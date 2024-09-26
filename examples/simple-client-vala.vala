/*
 * Copyright 2011 Marco Trevisan <mail@3v1n0.net>
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

private int main (string[] args)
{
    SimpleClient pClient = new SimpleClient (args);
    pClient.run ();

    return 0;
}

class SimpleClient
{
    AppIndicator.Indicator pIndicator = null;
    int nPercentage = 0;
    bool bActive = true;
    bool bHasLabel = true;
    GLib.MainLoop pMainloop = null;

    public SimpleClient (string[] args)
    {
        this.pIndicator = new AppIndicator.Indicator ("example-simple-client", "indicator-messages", AppIndicator.IndicatorCategory.APPLICATION_STATUS);
        this.pIndicator.set_status (AppIndicator.IndicatorStatus.ACTIVE);
        this.pIndicator.set_attention_icon ("indicator-messages-new", "System Messages Icon Highlighted");
        this.pIndicator.set_label ("1%", "100%");
        this.pIndicator.set_title ("Test Indicator (Vala)");
        this.pIndicator.set_tooltip ("indicator-messages-new", "tooltip-title", "tooltip-description");
    }

    private void onLabelToggle (int nPosition)
    {
        this.bHasLabel = !this.bHasLabel;
        string sLabel = null;

        if (this.bHasLabel)
        {
            sLabel = "Hide label";
        }
        else
        {
            sLabel = "Show label";
        }

        GLib.Menu pMenu = this.pIndicator.get_menu ();
        pMenu.remove (nPosition);
        GLib.MenuItem pItem = new GLib.MenuItem (sLabel, "indicator.showlabel");
        pMenu.insert_item (nPosition, pItem);
    }

    private void onAttentionActivate (int nPosition)
    {
        GLib.Menu pMenu = this.pIndicator.get_menu ();
        pMenu.remove (nPosition);
        GLib.MenuItem pItem = null;

        if (this.bActive)
        {
            this.pIndicator.set_status (AppIndicator.IndicatorStatus.ATTENTION);
            pItem = new GLib.MenuItem ("I'm okay now", "indicator.attention");
            this.bActive = false;
        }
        else
        {
            this.pIndicator.set_status (AppIndicator.IndicatorStatus.ACTIVE);
            pItem = new GLib.MenuItem ("Get Attention", "indicator.attention");
            this.bActive = true;
        }

        pMenu.insert_item (nPosition, pItem);
    }

    private void onLocalIcon (GLib.SimpleAction pAction, GLib.Variant? pValue)
    {
        GLib.SimpleActionGroup pActions = this.pIndicator.get_actions ();
        GLib.Action pCheckAction = pActions.lookup_action ("check");
        GLib.Variant pState = pCheckAction.get_state ();
        bool bActive = pState.get_boolean ();

        if (bActive)
        {
            this.pIndicator.set_icon ("simple-client-test-icon.png", "Local Icon");
        }
        else
        {
            this.pIndicator.set_icon ("indicator-messages", "System Icon");
        }
    }

    private void onItemActivate (string sName)
    {
        GLib.print ("%s clicked!\n", sName);
    }

    private void onCheckActivate (string sAction, string sName)
    {
        GLib.SimpleActionGroup pActions = this.pIndicator.get_actions ();
        GLib.Action pAction = pActions.lookup_action ("check");
        GLib.Variant pState = pAction.get_state ();
        bool bActive = pState.get_boolean ();
        GLib.Variant pNewState = new GLib.Variant.boolean (!bActive);
        pAction.change_state (pNewState);
        this.onItemActivate (sName);
    }

    private void onSensitivityActivate (string sAction)
    {
        GLib.SimpleActionGroup pActions = this.pIndicator.get_actions ();
        GLib.Action pSensitivityAction = pActions.lookup_action (sAction);
        bool bEnabled = pSensitivityAction.get_enabled ();
        pSensitivityAction.set ("enabled", !bEnabled);

        if (sAction == "sub")
        {
            GLib.Menu pMenu = this.pIndicator.get_menu ();
            pMenu.remove (2);
            this.addSubmenu (pMenu, !bEnabled);
        }
    }

    private void onImageActivate (int nPosition)
    {
        GLib.Menu pMenu = this.pIndicator.get_menu ();
        pMenu.remove (nPosition);
        GLib.MenuItem pItem = new GLib.MenuItem ("New", "indicator.setimage");
        GLib.Icon pIcon = new GLib.ThemedIcon.with_default_fallbacks ("document-open");
        pItem.set_icon (pIcon);
        pMenu.insert_item (nPosition, pItem);
    }

    private void onScroll (AppIndicator.Indicator pIndicator, int nDelta, uint nDirection)
    {
        GLib.print ("Got scroll event! delta: %d, direction: %u\n", nDelta, nDirection);
    }

    private bool onPercentChange ()
    {
        this.nPercentage = (this.nPercentage + 1) % 100;

        if (this.bHasLabel)
        {
            this.pIndicator.set_label (@"$(this.nPercentage+1)%", "100%");
        }
        else
        {
            this.pIndicator.set_label ("", "");
        }

        return true;
    }

    private void addSubmenu (GLib.Menu pMenu, bool bEnabled)
    {
        GLib.MenuItem pMenuItem = new GLib.MenuItem ("3", "indicator.sub");

        if (bEnabled)
        {
            GLib.Menu pSubmenu = new GLib.Menu ();
            GLib.MenuItem pItem = new GLib.MenuItem ("Sub 1", "indicator.sub1");
            pSubmenu.append_item (pItem);
            pItem = new GLib.MenuItem ("Sub 2", "indicator.sub2");
            pSubmenu.append_item (pItem);
            pItem = new GLib.MenuItem ("Sub 3", "indicator.sub3");
            pSubmenu.append_item (pItem);
            pMenuItem.set_submenu (pSubmenu);
        }

        pMenu.insert_item (2, pMenuItem);
    }

    public void run()
    {
        this.pIndicator.scroll_event.connect (this.onScroll);
        GLib.Timeout.add_seconds (1, this.onPercentChange);

        GLib.Menu pMenu = new GLib.Menu ();
        GLib.SimpleActionGroup pActions = new GLib.SimpleActionGroup ();

        GLib.Variant pCheck = new GLib.Variant.boolean (false);
        GLib.SimpleAction pSimpleAction = new GLib.SimpleAction.stateful ("check", null, pCheck);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onCheckActivate ("check", "1"));
        GLib.MenuItem pItem = new GLib.MenuItem ("1", "indicator.check");
        pMenu.append_item (pItem);

        GLib.Variant pRadio = new GLib.Variant.string ("2");
        pSimpleAction = new GLib.SimpleAction.stateful ("radio", GLib.VariantType.STRING, pRadio);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onItemActivate ("2"));
        pItem = new GLib.MenuItem ("2", "indicator.radio::2");
        pMenu.append_item (pItem);

        pSimpleAction = new GLib.SimpleAction ("sub", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction = new GLib.SimpleAction ("sub1", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onItemActivate ("Sub 1"));
        pSimpleAction = new GLib.SimpleAction ("sub2", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onSensitivityActivate ("sub1"));
        pSimpleAction = new GLib.SimpleAction ("sub3", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onItemActivate ("Sub 3"));
        this.addSubmenu (pMenu, true);

        pSimpleAction = new GLib.SimpleAction ("sensitivity", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onSensitivityActivate ("sub"));
        pItem = new GLib.MenuItem ("Toggle 3", "indicator.sensitivity");
        pMenu.append_item (pItem);

        pSimpleAction = new GLib.SimpleAction ("setimage", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onImageActivate (4));
        pItem = new GLib.MenuItem ("New", "indicator.setimage");
        GLib.Icon pIcon = new GLib.ThemedIcon.with_default_fallbacks ("document-new");
        pItem.set_icon (pIcon);
        pMenu.append_item (pItem);

        pSimpleAction = new GLib.SimpleAction ("attention", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onAttentionActivate (5));
        pItem = new GLib.MenuItem ("Get Attention", "indicator.attention");
        pMenu.append_item (pItem);
        this.pIndicator.set_secondary_activate_target ("attention");

        pSimpleAction = new GLib.SimpleAction ("showlabel", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect ((pValue) => this.onLabelToggle (6));
        pItem = new GLib.MenuItem ("Show label", "indicator.showlabel");
        pMenu.append_item (pItem);

        pSimpleAction = new GLib.SimpleAction ("setlocalicon", null);
        pActions.add_action (pSimpleAction);
        pSimpleAction.activate.connect (this.onLocalIcon);
        pItem = new GLib.MenuItem ("Set Local Icon", "indicator.setlocalicon");
        pMenu.append_item (pItem);

        this.pIndicator.set_menu (pMenu);
        this.pIndicator.set_actions (pActions);
        this.onLabelToggle (6);
        this.pMainloop = new GLib.MainLoop (null, false);
        this.pMainloop.run ();
    }
}
