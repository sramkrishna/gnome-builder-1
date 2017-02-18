/* ide-xml-symbol-node.h
 *
 * Copyright (C) 2017 Sébastien Lafargue <slafargue@gnome.org>
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

#ifndef IDE_XML_SYMBOL_NODE_H
#define IDE_XML_SYMBOL_NODE_H

#include "ide-xml-symbol-resolver.h"

#include <ide.h>

G_BEGIN_DECLS

#define IDE_TYPE_XML_SYMBOL_NODE (ide_xml_symbol_node_get_type())

G_DECLARE_FINAL_TYPE (IdeXmlSymbolNode, ide_xml_symbol_node, IDE, XML_SYMBOL_NODE, IdeSymbolNode)

IdeXmlSymbolNode         *ide_xml_symbol_node_new                           (const gchar            *name,
                                                                             const gchar            *value,
                                                                             const gchar            *element_name,
                                                                             IdeSymbolKind           kind,
                                                                             GFile                  *file,
                                                                             gint                    line,
                                                                             gint                    line_offset);
void                      ide_xml_symbol_node_take_child                    (IdeXmlSymbolNode       *self,
                                                                             IdeXmlSymbolNode       *child);
void                      ide_xml_symbol_node_take_internal_child           (IdeXmlSymbolNode       *self,
                                                                             IdeXmlSymbolNode       *child);
const gchar              *ide_xml_symbol_node_get_element_name              (IdeXmlSymbolNode       *self);
GFile *                   ide_xml_symbol_node_get_location                  (IdeXmlSymbolNode       *self,
                                                                             gint                   *line,
                                                                             gint                   *line_offset);
guint                     ide_xml_symbol_node_get_n_children                (IdeXmlSymbolNode       *self);
guint                     ide_xml_symbol_node_get_n_internal_children       (IdeXmlSymbolNode       *self);
IdeSymbolNode            *ide_xml_symbol_node_get_nth_child                 (IdeXmlSymbolNode       *self,
                                                                             guint                   nth_child);
IdeSymbolNode            *ide_xml_symbol_node_get_nth_internal_child        (IdeXmlSymbolNode       *self,
                                                                             guint                   nth_child);
const gchar              *ide_xml_symbol_node_get_value                     (IdeXmlSymbolNode       *self);
void                      ide_xml_symbol_node_set_location                  (IdeXmlSymbolNode       *self,
                                                                             GFile                  *file,
                                                                             gint                    line,
                                                                             gint                    line_offset);
void                      ide_xml_symbol_node_set_element_name              (IdeXmlSymbolNode       *self,
                                                                             const gchar            *element_name);
void                      ide_xml_symbol_node_set_value                     (IdeXmlSymbolNode       *self,
                                                                             const gchar            *value);

G_END_DECLS

#endif /* IDE_XML_SYMBOL_NODE_H */
