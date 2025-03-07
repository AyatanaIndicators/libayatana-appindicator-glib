/*
 * Copyright 2009 Ted Gould <ted@canonical.com>
 * Copyright 2009 Cody Russell <cody.russell@canonical.com>
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

#ifndef __APP_INDICATOR_H__
#define __APP_INDICATOR_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define APP_INDICATOR_TYPE (app_indicator_get_type ())
#define APP_INDICATOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), APP_INDICATOR_TYPE, AppIndicator))
#define APP_INDICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), APP_INDICATOR_TYPE, AppIndicatorClass))
#define APP_IS_INDICATOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APP_INDICATOR_TYPE))
#define APP_IS_INDICATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APP_INDICATOR_TYPE))
#define APP_INDICATOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), APP_INDICATOR_TYPE, AppIndicatorClass))

/**
 * AppIndicatorCategory:
 * @APP_INDICATOR_CATEGORY_APPLICATION_STATUS: The indicator is used to display the status of the application.
 * @APP_INDICATOR_CATEGORY_COMMUNICATIONS: The application is used for communication with other people.
 * @APP_INDICATOR_CATEGORY_SYSTEM_SERVICES: A system indicator relating to something in the user's system.
 * @APP_INDICATOR_CATEGORY_HARDWARE: An indicator relating to the user's hardware.
 * @APP_INDICATOR_CATEGORY_OTHER: Something not defined in this enum, please don't use unless you really need it.
 *
 * The category provides grouping for the indicators so that
 * users can find indicators that are similar together.
 */
typedef enum { /*< prefix=APP_INDICATOR_CATEGORY >*/
    APP_INDICATOR_CATEGORY_APPLICATION_STATUS, /*< nick=ApplicationStatus >*/
    APP_INDICATOR_CATEGORY_COMMUNICATIONS, /*< nick=Communications >*/
    APP_INDICATOR_CATEGORY_SYSTEM_SERVICES, /*< nick=SystemServices >*/
    APP_INDICATOR_CATEGORY_HARDWARE, /*< nick=Hardware >*/
    APP_INDICATOR_CATEGORY_OTHER /*< nick=Other >*/
} AppIndicatorCategory;

/**
 * AppIndicatorStatus:
 * @APP_INDICATOR_STATUS_PASSIVE: The indicator should not be shown to the user.
 * @APP_INDICATOR_STATUS_ACTIVE: The indicator should be shown in it's default state.
 * @APP_INDICATOR_STATUS_ATTENTION: The indicator should show it's attention icon.
 *
 * These are the states that the indicator can be on in
 * the user's panel.  The indicator by default starts
 * in the state @APP_INDICATOR_STATUS_PASSIVE and can be
 * shown by setting it to @APP_INDICATOR_STATUS_ACTIVE.
 */
typedef enum { /*< prefix=APP_INDICATOR_STATUS >*/
    APP_INDICATOR_STATUS_PASSIVE, /*< nick=Passive >*/
    APP_INDICATOR_STATUS_ACTIVE, /*< nick=Active >*/
    APP_INDICATOR_STATUS_ATTENTION /*< nick=NeedsAttention >*/
} AppIndicatorStatus;

/**
 * AppIndicator:
 *
 * An application indicator represents the values that are needed to
 * show a unique status in the panel for an application. In general,
 * applications should try to fit in the other indicators that are
 * available on the panel before creating a new indicator.
 *
 */
typedef struct _AppIndicator
{
    GObject parent;
} AppIndicator;

typedef struct _AppIndicatorClass
{
    GObjectClass parent_class;
    void (*new_icon) (AppIndicator *indicator, gpointer user_data);
    void (*new_attention_icon) (AppIndicator *indicator, gpointer user_data);
    void (*new_status) (AppIndicator *indicator, const gchar *status, gpointer user_data);
    void (*new_icon_theme_path) (AppIndicator *indicator, const gchar *icon_theme_path, gpointer user_data);
    void (*new_label) (AppIndicator *indicator, const gchar *label, const gchar *guide, gpointer user_data);
    void (*new_tooltip) (AppIndicator *indicator, gpointer user_data);
    void (*connection_changed) (AppIndicator *indicator, gboolean connected, gpointer user_data);
    void (*scroll_event) (AppIndicator *indicator, gint delta, guint direction, gpointer user_data);
} AppIndicatorClass;

GType app_indicator_get_type ();
AppIndicator* app_indicator_new (const gchar *id, const gchar *icon_name, AppIndicatorCategory category);
AppIndicator* app_indicator_new_with_path (const gchar *id, const gchar *icon_name, AppIndicatorCategory  category, const gchar *icon_theme_path);
void app_indicator_set_status (AppIndicator *self, AppIndicatorStatus status);
void app_indicator_set_attention_icon (AppIndicator *self, const gchar *icon_name, const gchar *icon_desc);
void app_indicator_set_menu (AppIndicator *self, GMenu *menu);
void app_indicator_set_actions (AppIndicator *self, GSimpleActionGroup *actions);
void app_indicator_set_icon (AppIndicator *self, const gchar *icon_name, const gchar *icon_desc);
void app_indicator_set_label (AppIndicator *self, const gchar *label, const gchar *guide);
void app_indicator_set_icon_theme_path (AppIndicator *self, const gchar *icon_theme_path);
void app_indicator_set_ordering_index (AppIndicator *self, guint32 ordering_index);
void app_indicator_set_secondary_activate_target (AppIndicator *self, const gchar *action);
void app_indicator_set_title (AppIndicator *self, const gchar *title);
void app_indicator_set_tooltip (AppIndicator *self, const gchar *icon_name, const gchar *title, const gchar *description);
const gchar* app_indicator_get_id (AppIndicator *self);
AppIndicatorCategory app_indicator_get_category (AppIndicator *self);
AppIndicatorStatus app_indicator_get_status (AppIndicator *self);
const gchar* app_indicator_get_icon (AppIndicator *self);
const gchar* app_indicator_get_icon_desc (AppIndicator *self);
const gchar* app_indicator_get_icon_theme_path (AppIndicator *self);
const gchar* app_indicator_get_attention_icon (AppIndicator *self);
const gchar* app_indicator_get_attention_icon_desc (AppIndicator *self);
const gchar* app_indicator_get_title (AppIndicator *self);
GMenu* app_indicator_get_menu (AppIndicator *self);
GSimpleActionGroup* app_indicator_get_actions (AppIndicator *self);
const gchar* app_indicator_get_label (AppIndicator *self);
const gchar* app_indicator_get_label_guide (AppIndicator *self);
guint32 app_indicator_get_ordering_index (AppIndicator *self);
const gchar* app_indicator_get_secondary_activate_target (AppIndicator *self);

G_END_DECLS

#endif
