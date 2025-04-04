#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2024-2025 Robert Tari <robert@tari.in>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranties of
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

import gi

gi.require_version('AyatanaAppIndicatorGlib', '2.0')

from gi.repository import AyatanaAppIndicatorGlib
from gi.repository import GLib, Gio
import os
import sys

class SimpleClient:

    pIndicator = None
    nPercentage = 0
    bActive = True
    bHasLabel = True
    pMainloop = None

    def __init__ (self):

        self.pIndicator = AyatanaAppIndicatorGlib.Indicator.new ("example-simple-client", "indicator-messages", AyatanaAppIndicatorGlib.IndicatorCategory.APPLICATION_STATUS)
        self.pIndicator.set_status (AyatanaAppIndicatorGlib.IndicatorStatus.ACTIVE)
        self.pIndicator.set_attention_icon ("indicator-messages-new", "System Messages Icon Highlighted")
        self.pIndicator.set_label ("1%", "100%");
        self.pIndicator.set_title ("Test Indicator (Python)")
        self.pIndicator.set_tooltip ("indicator-messages-new", "tooltip-title", "tooltip-description")

        self.pIndicator.connect ("scroll-event", self.onScroll)
        GLib.timeout_add_seconds (1, self.onPercentChange, None)

        pMenu = Gio.Menu.new ()
        pActions = Gio.SimpleActionGroup.new ()

        pCheck = GLib.Variant.new_boolean (False)
        pSimpleAction = Gio.SimpleAction.new_stateful ("check", None, pCheck)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onCheckActivate, "1")
        pItem = Gio.MenuItem.new ("1", "indicator.check")
        pMenu.append_item (pItem)

        pRadio = GLib.Variant.new_string ("2")
        pType = GLib.VariantType.new ("s")
        pSimpleAction = Gio.SimpleAction.new_stateful ("radio", pType, pRadio)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onItemActivate, "2")
        pItem = Gio.MenuItem.new ("2", "indicator.radio::2")
        pMenu.append_item (pItem)

        pSimpleAction = Gio.SimpleAction.new ("sub", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction = Gio.SimpleAction.new ("sub1", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onItemActivate, "Sub 1")
        pSimpleAction = Gio.SimpleAction.new ("sub2", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onSensitivityActivate, "sub1")
        pSimpleAction = Gio.SimpleAction.new ("sub3", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onItemActivate, "Sub 3")
        self.addSubmenu (pMenu, True)

        pSimpleAction = Gio.SimpleAction.new ("sensitivity", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onSensitivityActivate, "sub")
        pItem = Gio.MenuItem.new ("Toggle 3", "indicator.sensitivity")
        pMenu.append_item (pItem)

        pSimpleAction = Gio.SimpleAction.new ("setimage", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onImageActivate, 4)
        pItem = Gio.MenuItem.new ("New", "indicator.setimage")
        pIcon = Gio.ThemedIcon.new_with_default_fallbacks ("document-new")
        pItem.set_icon (pIcon)
        pMenu.append_item (pItem)

        pSimpleAction = Gio.SimpleAction.new ("attention", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onAttentionActivate, 5)
        pItem = Gio.MenuItem.new ("Get Attention", "indicator.attention")
        pMenu.append_item (pItem)
        self.pIndicator.set_secondary_activate_target ("attention")

        pSimpleAction = Gio.SimpleAction.new ("showlabel", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onLabelToggle, 6)
        pItem = Gio.MenuItem.new ("Show label", "indicator.showlabel")
        pMenu.append_item (pItem)

        pSimpleAction = Gio.SimpleAction.new ("setlocalicon", None)
        pActions.add_action (pSimpleAction)
        pSimpleAction.connect ("activate", self.onLocalIcon, None)
        pItem = Gio.MenuItem.new ("Set Local Icon", "indicator.setlocalicon")
        pMenu.append_item (pItem)

        self.pIndicator.set_menu (pMenu)
        self.pIndicator.set_actions (pActions)
        self.onLabelToggle (None, None, 6)
        self.pMainloop = GLib.MainLoop.new (None, False)

        try:

            self.pMainloop.run ()

        except KeyboardInterrupt:

            sys.exit (0)

    def onLabelToggle (self, pAction, pValue, nPosition):

        self.bHasLabel = not self.bHasLabel
        sLabel = None

        if self.bHasLabel:

            sLabel = "Hide label"

        else:

            sLabel = "Show label"

        pMenu = self.pIndicator.get_menu ()
        pMenu.remove (nPosition)
        pItem = Gio.MenuItem.new (sLabel, "indicator.showlabel")
        pMenu.insert_item (nPosition, pItem)

    def onAttentionActivate (self, pAction, pValue, nPosition):

        pMenu = self.pIndicator.get_menu ()
        pMenu.remove (nPosition)
        pItem = None

        if self.bActive:

            self.pIndicator.set_status (AyatanaAppIndicatorGlib.IndicatorStatus.ATTENTION)
            pItem = Gio.MenuItem.new ("I'm okay now", "indicator.attention")
            self.bActive = False

        else:

            self.pIndicator.set_status (AyatanaAppIndicatorGlib.IndicatorStatus.ACTIVE)
            pItem = Gio.MenuItem.new ("Get Attention", "indicator.attention")
            self.bActive = True

        pMenu.insert_item (nPosition, pItem)

    def onLocalIcon (self, pAction, pValue, pData):

        pActions = self.pIndicator.get_actions ()
        pCheckAction = pActions.lookup_action ("check")
        pState = pCheckAction.get_state ()
        bActive = pState.get_boolean ()

        if bActive:

            sRealPath = os.path.realpath (__file__)
            sDirname = os.path.dirname (sRealPath)
            sIconPath = os.path.join (sDirname, "simple-client-test-icon.png")
            self.pIndicator.set_icon (sIconPath, "Local Icon")

        else:

            self.pIndicator.set_icon ("indicator-messages", "System Icon")

    def onItemActivate (self, pAction, pValue, sName):

        print (f"{sName} clicked!")


    def onCheckActivate (self, pAction, pValue, sName):

        pState = pAction.get_state ()
        bActive = pState.get_boolean ()
        pNewState = GLib.Variant.new_boolean (not bActive)
        pAction.change_state (pNewState)
        self.onItemActivate (None, None, sName)

    def onSensitivityActivate (self, pAction, pValue, sAction):

        pActions = self.pIndicator.get_actions ()
        pSensitivityAction = pActions.lookup_action (sAction)
        bEnabled = pSensitivityAction.get_enabled ()
        pSensitivityAction.props.enabled = not bEnabled

        if sAction == "sub":

            pMenu = self.pIndicator.get_menu ()
            pMenu.remove (2)
            self.addSubmenu (pMenu, not bEnabled)

    def onImageActivate (self, pAction, pValue, nPosition):

        pMenu = self.pIndicator.get_menu ()
        pMenu.remove (nPosition)
        pItem = Gio.MenuItem.new ("New", "indicator.setimage")
        pIcon = Gio.ThemedIcon.new_with_default_fallbacks ("document-open")
        pItem.set_icon (pIcon)
        pMenu.insert_item (nPosition, pItem)

    def onScroll (self, pIndicator, nDelta, nDirection):

        print (f"Got scroll event! delta: {nDelta}, direction: {nDirection}")


    def onPercentChange (self, pData):

        self.nPercentage = (self.nPercentage + 1) % 100

        if self.bHasLabel:

            self.pIndicator.set_label (f"{self.nPercentage}%", "100%")

        else:

            self.pIndicator.set_label ("", "")

        return True

    def addSubmenu (self, pMenu, bEnabled):

        pMenuItem = Gio.MenuItem.new ("3", "indicator.sub")

        if bEnabled:

            pSubmenu = Gio.Menu.new ()
            pItem = Gio.MenuItem.new ("Sub 1", "indicator.sub1")
            pSubmenu.append_item (pItem)
            pItem = Gio.MenuItem.new ("Sub 2", "indicator.sub2")
            pSubmenu.append_item (pItem)
            pItem = Gio.MenuItem.new ("Sub 3", "indicator.sub3")
            pSubmenu.append_item (pItem)
            pMenuItem.set_submenu (pSubmenu)

        pMenu.insert_item (2, pMenuItem)

if __name__ == "__main__":

    SimpleClient ()
