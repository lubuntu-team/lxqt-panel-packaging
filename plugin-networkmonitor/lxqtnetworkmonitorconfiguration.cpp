/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2011 Razor team
 * Authors:
 *   Maciej Płaza <plaza.maciej@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include "lxqtnetworkmonitorconfiguration.h"
#include "ui_lxqtnetworkmonitorconfiguration.h"

extern "C" {
#include <statgrab.h>
}

#ifdef __sg_public
// since libstatgrab 0.90 this macro is defined, so we use it for version check
#define STATGRAB_NEWER_THAN_0_90 	1
#endif

LXQtNetworkMonitorConfiguration::LXQtNetworkMonitorConfiguration(PluginSettings *settings, QWidget *parent) :
    LXQtPanelPluginConfigDialog(settings, parent),
    ui(new Ui::LXQtNetworkMonitorConfiguration)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("NetworkMonitorConfigurationWindow");
    ui->setupUi(this);

    connect(ui->buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(dialogButtonsAction(QAbstractButton*)));
    connect(ui->iconCB, SIGNAL(currentIndexChanged(int)), SLOT(saveSettings()));
    connect(ui->interfaceCB, SIGNAL(currentIndexChanged(int)), SLOT(saveSettings()));

    loadSettings();
}

LXQtNetworkMonitorConfiguration::~LXQtNetworkMonitorConfiguration()
{
    delete ui;
}

void LXQtNetworkMonitorConfiguration::saveSettings()
{
    settings().setValue("icon", ui->iconCB->currentIndex());
    settings().setValue("interface", ui->interfaceCB->currentText());
}

void LXQtNetworkMonitorConfiguration::loadSettings()
{
    ui->iconCB->setCurrentIndex(settings().value("icon", 1).toInt());

    int count;
#ifdef STATGRAB_NEWER_THAN_0_90
    size_t ret_count;
    sg_network_iface_stats* stats = sg_get_network_iface_stats(&ret_count);
    count = (int)ret_count;
#else
    sg_network_iface_stats* stats = sg_get_network_iface_stats(&count);
#endif
    for (int ix = 0; ix < count; ix++)
        ui->interfaceCB->addItem(stats[ix].interface_name);

    QString interface = settings().value("interface").toString();
    ui->interfaceCB->setCurrentIndex(qMax(qMin(0, count - 1), ui->interfaceCB->findText(interface)));
}
