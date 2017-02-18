/* ide-vcs-config.c
 *
 * Copyright (C) 2016 Akshaya Kakkilaya <akshaya.kakkilaya@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ide-vcs-config.h"

G_DEFINE_INTERFACE (IdeVcsConfig, ide_vcs_config, G_TYPE_OBJECT)

static void
ide_vcs_config_default_init (IdeVcsConfigInterface *iface)
{
}

/*
 * ide_vcs_config_get_config:
 *
 * Retrieves the value of the underlying VCS configuration type.  (e.g. email address for git)
 *
 * Returns: null
 */
void
ide_vcs_config_get_config (IdeVcsConfig    *self,
                           IdeVcsConfigType type,
                           GValue          *value)
{
  g_return_if_fail (IDE_IS_VCS_CONFIG (self));

  IDE_VCS_CONFIG_GET_IFACE (self)->get_config (self, type, value);
}

/*
 * ide_vcs_config_set_config:
 *
 * Sets the value of a VCS configuration type, (e.g. email address for git)
 *
 */
void
ide_vcs_config_set_config (IdeVcsConfig    *self,
                           IdeVcsConfigType type,
                           const GValue    *value)
{
  g_return_if_fail (IDE_IS_VCS_CONFIG (self));

  IDE_VCS_CONFIG_GET_IFACE (self)->set_config (self, type, value);
}
