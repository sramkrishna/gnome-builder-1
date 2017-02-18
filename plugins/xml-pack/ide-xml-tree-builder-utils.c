/* ide-xml-tree-builder-utils.c
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

#include "ide-xml-tree-builder-utils-private.h"

void
print_node (IdeXmlSymbolNode *node,
            guint             depth)
{
  g_autofree gchar *spacer;
  gint line;
  gint line_offset;

  g_return_if_fail (IDE_IS_XML_SYMBOL_NODE (node) || node == NULL);

  if (node == NULL)
    {
      g_warning ("Node NULL");
      return;
    }

  spacer = g_strnfill (depth, '\t');
  ide_xml_symbol_node_get_location (node, &line, &line_offset);

  printf ("%s%s (%i) at (%i,%i) %p\n",
          spacer,
          ide_symbol_node_get_name (IDE_SYMBOL_NODE (node)),
          depth,
          line,
          line_offset,
          node);
}

const gchar *
list_get_attribute (const guchar **attributes,
                    const gchar  *name)
{
  const guchar **l = attributes;

  g_return_val_if_fail (!ide_str_empty0 (name), NULL);

  if (attributes == NULL)
    return NULL;

  while (l [0] != NULL)
    {
      if (ide_str_equal0 (name, l [0]))
        return (const gchar *)l [1];

      l += 2;
    }

  return NULL;
}
