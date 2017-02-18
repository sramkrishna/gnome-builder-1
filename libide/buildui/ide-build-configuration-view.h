/* ide-build-configuration-view.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#ifndef IDE_BUILD_CONFIGURATION_VIEW_H
#define IDE_BUILD_CONFIGURATION_VIEW_H

#include <ide.h>

#include "egg-column-layout.h"

G_BEGIN_DECLS

#define IDE_TYPE_BUILD_CONFIGURATION_VIEW (ide_build_configuration_view_get_type())

G_DECLARE_FINAL_TYPE (IdeBuildConfigurationView, ide_build_configuration_view, IDE, BUILD_CONFIGURATION_VIEW, EggColumnLayout)

IdeConfiguration *ide_build_configuration_view_get_configuration (IdeBuildConfigurationView *self);
void              ide_build_configuration_view_set_configuration (IdeBuildConfigurationView *self,
                                                                  IdeConfiguration          *configuration);

G_END_DECLS

#endif /* IDE_BUILD_CONFIGURATION_VIEW_H */
