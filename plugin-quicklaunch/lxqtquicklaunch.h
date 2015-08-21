/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2012 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
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


#ifndef LXQTQUICKLAUNCH_H
#define LXQTQUICKLAUNCH_H

#include "../panel/lxqtpanel.h"
#include <QHash>
#include <QString>


class XdgDesktopFile;
class QuickLaunchAction;
class QDragEnterEvent;
class QuickLaunchButton;
class QSettings;
class QLabel;

namespace LxQt {
class GridLayout;
}


/*! \brief Loader for "quick launcher" icons in the panel.
\author Petr Vanek <petr@scribus.info>
*/
class LxQtQuickLaunch : public QFrame
{
    Q_OBJECT

public:
    LxQtQuickLaunch(ILxQtPanelPlugin *plugin, QWidget* parent = 0);
    ~LxQtQuickLaunch();

    int indexOfButton(QuickLaunchButton* button) const;
    int countOfButtons() const;

    void realign();

private:
    LxQt::GridLayout *mLayout;
    ILxQtPanelPlugin *mPlugin;
    QLabel *mPlaceHolder;

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

    void saveSettings();
    void showPlaceHolder();

private slots:
    void addButton(QuickLaunchAction* action);
    void switchButtons(QuickLaunchButton *button1, QuickLaunchButton *button2);
    void buttonDeleted();
    void buttonMoveLeft();
    void buttonMoveRight();
};

#endif
