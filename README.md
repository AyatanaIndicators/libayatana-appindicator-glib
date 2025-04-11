# Ayatana Application Indicator (Shared Library) [![Build Status](https://api.travis-ci.com/AyatanaIndicators/libayatana-appindicator.svg)](https://travis-ci.com/github/AyatanaIndicators/libayatana-appindicator)

## About Ayatana Indicators

The Ayatana Indicators project is the continuation of Application
Indicators and System Indicators, two technologies developed by Canonical
Ltd. for the Unity7 desktop.

Application Indicators are a GTK implementation of the StatusNotifierItem
Specification (SNI) that was originally submitted to freedesktop.org by
KDE.

System Indicators are an extensions to the Application Indicators idea.
System Indicators allow for far more widgets to be displayed in the
indicator's menu.

The Ayatana Indicators project is the new upstream for application
indicators, system indicators and associated projects with a focus on
making Ayatana Indicators a desktop agnostic technology.

On GNU/Linux, Ayatana Indicators are currently available for desktop
envinronments like MATE (used by default in [Ubuntu
MATE](https://ubuntu-mate.com)), XFCE (used by default in
[Xubuntu](https://bluesabre.org/2021/02/25/xubuntu-21-04-progress-update/),
LXDE, and the Budgie Desktop).

The Lomiri Operating Environment (UI of the Ubuntu Touch OS, formerly
known as Unity8) uses Ayatana Indicators for rendering its notification
area and the [UBports](https://ubports.com) project is a core contributor
to the Ayatana Indicators project.

For further info, please visit:
https://ayatana-indicators.org

## The Ayatana Application Indicator (Shared Library)

A library to allow applications to export a menu into an Application
Indicators aware menu bar. Although based on SNI, this new GLib-only
reimplementation uses Gio menus and actions (exported to org.gtk.Menus
and org.gtk.Actions) instead of the old dbusmenu (formerly exported to
com.canonical.dbusmenu).

This code project was originally started by Canonical Ltd. and has been
adapted by various authors with the purpose of making this Application
Indicators available on Ubuntu and non-Ubuntu systems alike.

## Licence and Copyright

See COPYING and AUTHORS file in this project.

## Building and Testing

For instructions on building and running built-in tests, see the
INSTALL.md file.
