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


#ifndef LXQTMAINMENUCONFIGURATION_H
#define LXQTMAINMENUCONFIGURATION_H

#include <LXQt/Settings>

#include <QDialog>

class QSettings;
class QAbstractButton;

namespace Ui {
    class LXQtMainMenuConfiguration;
}

class LXQtMainMenuConfiguration : public QDialog
{
    Q_OBJECT

public:
    explicit LXQtMainMenuConfiguration(QSettings &settings, const QString &defaultShortcut, QWidget *parent = 0);
    ~LXQtMainMenuConfiguration();

private:
    Ui::LXQtMainMenuConfiguration *ui;
    QSettings &mSettings;
    LXQt::SettingsCache mOldSettings;
    QString mDefaultShortcut;

private slots:
    /*
      Saves settings in conf file.
    */
    void loadSettings();
    void dialogButtonsAction(QAbstractButton *btn);
    void textButtonChanged(const QString &value);
    void showTextChanged(bool value);
    void chooseMenuFile();
    void shortcutChanged(const QString &value);
    void shortcutReset();
    void customFontChanged(bool value);
    void customFontSizeChanged(int value);
};

#endif // LXQTMAINMENUCONFIGURATION_H
