/* DBus defaults for everyone to share in the project.
 *
 * Copyright 2009 Ted Gould <ted@canonical.com>
 * Copyright 2024 Robert Tari <robert@tari.in>
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

#define DBUS_SERVICE_DBUS "org.freedesktop.DBus"
#define DBUS_PATH_DBUS "/org/freedesktop/DBus"
#define DBUS_INTERFACE_DBUS "org.freedesktop.DBus"
#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"
#define DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER 1
#define INDICATOR_APPLICATION_DBUS_ADDR "org.ayatana.indicator.application"
#define INDICATOR_APPLICATION_DBUS_OBJ "/org/ayatana/indicator/application/service"
#define INDICATOR_APPLICATION_DBUS_IFACE "org.ayatana.indicator.application.service"
#define NOTIFICATION_WATCHER_DBUS_ADDR "org.kde.StatusNotifierWatcher"
#define NOTIFICATION_WATCHER_DBUS_OBJ "/StatusNotifierWatcher"
#define NOTIFICATION_WATCHER_DBUS_IFACE "org.kde.StatusNotifierWatcher"
#define NOTIFICATION_ITEM_DBUS_IFACE "org.kde.StatusNotifierItem"
#define NOTIFICATION_ITEM_DEFAULT_OBJ "/StatusNotifierItem"
#define NOTIFICATION_APPROVER_DBUS_IFACE "org.ayatana.StatusNotifierApprover"
