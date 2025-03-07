/*
 * Copyright 2011 Michal Hruby <michal.hruby@canonical.com>
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

namespace AyatanaAppIndicatorGlib
{
  [CCode (type_check_function = "APP_IS_INDICATOR", type_id = "app_indicator_get_type ()")]
  public class Indicator : GLib.Object
  {
  }
}
