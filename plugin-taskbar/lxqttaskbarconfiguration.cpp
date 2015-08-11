/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 * http://lxqt.org
 *
 * Copyright: 2011 Razor team
 *            2014 LXQt team
 * Authors:
 *   Maciej Płaza <plaza.maciej@gmail.com>
 *   Kuzma Shapran <kuzma.shapran@gmail.com>
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

#include "lxqttaskbarconfiguration.h"
#include "ui_lxqttaskbarconfiguration.h"

LxQtTaskbarConfiguration::LxQtTaskbarConfiguration(QSettings &settings, QWidget *parent):
    QDialog(parent),
    ui(new Ui::LxQtTaskbarConfiguration),
    mSettings(settings),
    oldSettings(settings)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("TaskbarConfigurationWindow");
    ui->setupUi(this);

    connect(ui->buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(dialogButtonsAction(QAbstractButton*)));

    ui->buttonStyleCB->addItem(tr("Icon and text"), "IconText");
    ui->buttonStyleCB->addItem(tr("Only icon"), "Icon");
    ui->buttonStyleCB->addItem(tr("Only text"), "Text");

    loadSettings();
    /* We use clicked() and activated(int) because these signals aren't emitting after programmaticaly
        change of state */
    connect(ui->fAllDesktopsCB, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(ui->fCurrentDesktopRB, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(ui->buttonStyleCB, SIGNAL(activated(int)), this, SLOT(updateControls(int)));
    connect(ui->buttonStyleCB, SIGNAL(activated(int)), this, SLOT(saveSettings()));
    connect(ui->buttonWidthSB, SIGNAL(valueChanged(int)), this, SLOT(saveSettings()));
    connect(ui->autoRotateCB, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(ui->middleClickCB, SIGNAL(clicked()), this, SLOT(saveSettings()));
}

LxQtTaskbarConfiguration::~LxQtTaskbarConfiguration()
{
    delete ui;
}

void LxQtTaskbarConfiguration::loadSettings()
{
    if (mSettings.value("showOnlyCurrentDesktopTasks", false).toBool() == true)
    {
        ui->fCurrentDesktopRB->setChecked(true);
    }
    else
    {
        ui->fAllDesktopsCB->setChecked(true);
    }

    ui->autoRotateCB->setChecked(mSettings.value("autoRotate", true).toBool());
    ui->middleClickCB->setChecked(mSettings.value("closeOnMiddleClick", true).toBool());
    ui->buttonStyleCB->setCurrentIndex(ui->buttonStyleCB->findData(mSettings.value("buttonStyle", "IconText")));
    updateControls(ui->buttonStyleCB->currentIndex());

    /* Keep buttonWidth loading at the end of this method to prevent errors */
    ui->buttonWidthSB->setValue(mSettings.value("buttonWidth", 400).toInt());
}

void LxQtTaskbarConfiguration::saveSettings()
{
    mSettings.setValue("showOnlyCurrentDesktopTasks", ui->fCurrentDesktopRB->isChecked());
    mSettings.setValue("buttonStyle", ui->buttonStyleCB->itemData(ui->buttonStyleCB->currentIndex()));
    mSettings.setValue("buttonWidth", ui->buttonWidthSB->value());
    mSettings.setValue("autoRotate", ui->autoRotateCB->isChecked());
    mSettings.setValue("closeOnMiddleClick", ui->middleClickCB->isChecked());
}

void LxQtTaskbarConfiguration::updateControls(int index)
{
    if (ui->buttonStyleCB->itemData(index) == "Icon")
    {
        ui->buttonWidthSB->setEnabled(false);
        ui->buttonWidthL->setEnabled(false);
    }
    else
    {
        ui->buttonWidthSB->setEnabled(true);
        ui->buttonWidthL->setEnabled(true);
    }
}

void LxQtTaskbarConfiguration::dialogButtonsAction(QAbstractButton *btn)
{
    if (ui->buttons->buttonRole(btn) == QDialogButtonBox::ResetRole)
    {
        /* We have to disable signals for buttonWidthSB to prevent errors. Otherwise not all data
          could be restored */
        ui->buttonWidthSB->blockSignals(true);
        oldSettings.loadToSettings();
        loadSettings();
        ui->buttonWidthSB->blockSignals(false);
    }
    else
    {
        close();
    }
}
