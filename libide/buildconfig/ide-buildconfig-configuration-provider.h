/* ide-buildconfig-configuration-provider.h
 *
 * Copyright (C) 2016 Matthew Leeds <mleeds@redhat.com>
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

#ifndef IDE_BUILDCONFIG_CONFIGURATION_PROVIDER_H
#define IDE_BUILDCONFIG_CONFIGURATION_PROVIDER_H

#include <glib.h>

#include "ide-types.h"

G_BEGIN_DECLS

#define IDE_TYPE_BUILDCONFIG_CONFIGURATION_PROVIDER (ide_buildconfig_configuration_provider_get_type())

G_DECLARE_FINAL_TYPE (IdeBuildconfigConfigurationProvider, ide_buildconfig_configuration_provider, IDE, BUILDCONFIG_CONFIGURATION_PROVIDER, GObject)

G_END_DECLS

#endif /* IDE_BUILDCONFIG_CONFIGURATION_PROVIDER_H */
