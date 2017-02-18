/* ide-xml-tree-builder-utils-private.h
 *
 * Copyright (C) 2017 Sebastien Lafargue <slafargue@gnome.org>
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
#ifndef IDE_XML_TREE_BUILDER_UTILS_PRIVATE_H
#define IDE_XML_TREE_BUILDER_UTILS_PRIVATE_H

#include <glib.h>
#include <ide.h>

#include "ide-xml-symbol-node.h"

G_BEGIN_DECLS

const gchar  *list_get_attribute         (const guchar      **attributes,
                                          const gchar        *name);
void          print_node                 (IdeXmlSymbolNode   *node,
                                          guint               depth);

G_END_DECLS

#endif /* IDE_XML_TREE_BUILDER_UTILS_PRIVATE_H */

