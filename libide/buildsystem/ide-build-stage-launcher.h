/* ide-build-stage-launcher.h
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

#ifndef IDE_BUILD_STAGE_LAUNCHER_H
#define IDE_BUILD_STAGE_LAUNCHER_H

#include <gio/gio.h>

#include "ide-context.h"

#include "buildsystem/ide-build-stage.h"
#include "subprocess/ide-subprocess-launcher.h"

G_BEGIN_DECLS

#define IDE_TYPE_BUILD_STAGE_LAUNCHER (ide_build_stage_launcher_get_type())

G_DECLARE_DERIVABLE_TYPE (IdeBuildStageLauncher, ide_build_stage_launcher, IDE, BUILD_STAGE_LAUNCHER, IdeBuildStage)

struct _IdeBuildStageLauncherClass
{
  IdeBuildStageClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

IdeBuildStage         *ide_build_stage_launcher_new                    (IdeContext            *context,
                                                                        IdeSubprocessLauncher *launcher);
IdeSubprocessLauncher *ide_build_stage_launcher_get_launcher           (IdeBuildStageLauncher *self);
IdeSubprocessLauncher *ide_build_stage_launcher_get_clean_launcher     (IdeBuildStageLauncher *self);
void                   ide_build_stage_launcher_set_clean_launcher     (IdeBuildStageLauncher *self,
                                                                        IdeSubprocessLauncher *clean_launcher);
gboolean               ide_build_stage_launcher_get_ignore_exit_status (IdeBuildStageLauncher *self);
void                   ide_build_stage_launcher_set_ignore_exit_status (IdeBuildStageLauncher *self,
                                                                        gboolean               ignore_exit_status);

G_END_DECLS

#endif /* IDE_BUILD_STAGE_LAUNCHER_H */
