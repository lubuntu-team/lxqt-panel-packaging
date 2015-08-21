/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2013 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "../lxqtmountplugin.h"
#include "deviceaction_info.h"

#include <LXQt/Notification>

DeviceActionInfo::DeviceActionInfo(LxQtMountPlugin *plugin, QObject *parent):
    DeviceAction(plugin, parent)
{
}

void DeviceActionInfo::doDeviceAdded(Solid::Device device)
{
    showMessage(tr("The device <b><nobr>\"%1\"</nobr></b> is connected.").arg(device.description()));
}

void DeviceActionInfo::doDeviceRemoved(Solid::Device device)
{
    showMessage(tr("The device <b><nobr>\"%1\"</nobr></b> is removed.").arg(device.description()));
}

void DeviceActionInfo::showMessage(const QString &text)
{
    LxQt::Notification::notify(tr("Removable media/devices manager"), text, mPlugin->icon().name());
}
