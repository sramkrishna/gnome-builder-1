/* ide-environment-editor.h
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#ifndef IDE_ENVIRONMENT_EDITOR_H
#define IDE_ENVIRONMENT_EDITOR_H

#include <gtk/gtk.h>
#include <ide.h>

G_BEGIN_DECLS

#define IDE_TYPE_ENVIRONMENT_EDITOR (ide_environment_editor_get_type())

G_DECLARE_FINAL_TYPE (IdeEnvironmentEditor, ide_environment_editor, IDE, ENVIRONMENT_EDITOR, GtkListBox)

GtkWidget      *ide_environment_editor_new             (void);
IdeEnvironment *ide_environment_editor_get_environment (IdeEnvironmentEditor *self);
void            ide_environment_editor_set_environment (IdeEnvironmentEditor *self,
                                                        IdeEnvironment       *environment);

G_END_DECLS

#endif /* IDE_ENVIRONMENT_EDITOR_H */
