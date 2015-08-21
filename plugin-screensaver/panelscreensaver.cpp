/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
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

#include <QToolButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <LXQt/ScreenSaver>
#include <LXQt/Notification>
#include <lxqt-globalkeys.h>

#include "panelscreensaver.h"

#define DEFAULT_SHORTCUT "Control+Alt+L"


PanelScreenSaver::PanelScreenSaver(const ILxQtPanelPluginStartupInfo &startupInfo) :
    QObject(),
    ILxQtPanelPlugin(startupInfo)
{
    mSaver = new LxQt::ScreenSaver(this);

    QList<QAction*> actions = mSaver->availableActions();
    if (!actions.empty())
        mButton.setDefaultAction(actions.first());
    //mButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    mShortcutKey = GlobalKeyShortcut::Client::instance()->addAction(QString(), QString("/panel/%1/lock").arg(settings()->group()), tr("Lock Screen"), this);
    if (mShortcutKey)
    {
        connect(mShortcutKey, &GlobalKeyShortcut::Action::registrationFinished, this, &PanelScreenSaver::shortcutRegistered);
        connect(mShortcutKey, SIGNAL(activated()), mSaver, SLOT(lockScreen()));
    }
}

void PanelScreenSaver::shortcutRegistered()
{
    if (mShortcutKey->shortcut().isEmpty())
    {
        mShortcutKey->changeShortcut(DEFAULT_SHORTCUT);
        if (mShortcutKey->shortcut().isEmpty())
        {
            LxQt::Notification::notify(tr("Panel Screensaver: Global shortcut '%1' cannot be registered").arg(DEFAULT_SHORTCUT));
        }
    }
}

#undef DEFAULT_SHORTCUT
