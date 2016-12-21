/* ide-configuration-provider.c
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

#include "ide-configuration-manager.h"
#include "ide-configuration-provider.h"

G_DEFINE_INTERFACE (IdeConfigurationProvider, ide_configuration_provider, G_TYPE_OBJECT)

static void
ide_configuration_provider_real_load (IdeConfigurationProvider *self,
                                      IdeConfigurationManager  *manager)
{
}

static void
ide_configuration_provider_real_unload (IdeConfigurationProvider *self,
                                        IdeConfigurationManager  *manager)
{
}

static void
ide_configuration_provider_default_init (IdeConfigurationProviderInterface *iface)
{
  iface->load = ide_configuration_provider_real_load;
  iface->unload = ide_configuration_provider_real_unload;
}

void
ide_configuration_provider_load (IdeConfigurationProvider *self,
                                 IdeConfigurationManager  *manager)
{
  g_return_if_fail (IDE_IS_CONFIGURATION_PROVIDER (self));
  g_return_if_fail (IDE_IS_CONFIGURATION_MANAGER (manager));

  IDE_CONFIGURATION_PROVIDER_GET_IFACE (self)->load (self, manager);
}

void
ide_configuration_provider_unload (IdeConfigurationProvider *self,
                                   IdeConfigurationManager  *manager)
{
  g_return_if_fail (IDE_IS_CONFIGURATION_PROVIDER (self));
  g_return_if_fail (IDE_IS_CONFIGURATION_MANAGER (manager));

  IDE_CONFIGURATION_PROVIDER_GET_IFACE (self)->unload (self, manager);
}
