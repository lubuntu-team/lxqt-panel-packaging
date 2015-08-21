/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2011 Razor team
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

#include <QButtonGroup>
#include <QToolButton>
#include <QWheelEvent>
#include <QtDebug>
#include <QSignalMapper>
#include <QTimer>
#include <lxqt-globalkeys.h>
#include <LXQt/GridLayout>
#include <KWindowSystem/KWindowSystem>
#include <QX11Info>
#include <cmath>

#include "desktopswitch.h"
#include "desktopswitchbutton.h"
#include "desktopswitchconfiguration.h"

#define DEFAULT_SHORTCUT_TEMPLATE QStringLiteral("Control+F%1")

DesktopSwitch::DesktopSwitch(const ILxQtPanelPluginStartupInfo &startupInfo) :
    QObject(),
    ILxQtPanelPlugin(startupInfo),
    m_pSignalMapper(new QSignalMapper(this)),
    m_desktopCount(KWindowSystem::numberOfDesktops()),
    mRows(-1),
    mDesktops(new NETRootInfo(QX11Info::connection(), NET::NumberOfDesktops | NET::CurrentDesktop | NET::DesktopNames, NET::WM2DesktopLayout)),
    mLabelType(static_cast<DesktopSwitchButton::LabelType>(-1))
{
    m_buttons = new QButtonGroup(this);
    connect (m_pSignalMapper, SIGNAL(mapped(int)), this, SLOT(setDesktop(int)));

    mLayout = new LxQt::GridLayout(&mWidget);
    mWidget.setLayout(mLayout);

    settingsChanged();

    onCurrentDesktopChanged(KWindowSystem::currentDesktop());
    QTimer::singleShot(0, this, SLOT(registerShortcuts()));

    connect(m_buttons, SIGNAL(buttonClicked(int)), this, SLOT(setDesktop(int)));

    connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)), SLOT(onNumberOfDesktopsChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), SLOT(onCurrentDesktopChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(desktopNamesChanged()), SLOT(onDesktopNamesChanged()));

    connect(KWindowSystem::self(), static_cast<void (KWindowSystem::*)(WId, NET::Properties, NET::Properties2)>(&KWindowSystem::windowChanged),
            this, &DesktopSwitch::onWindowChanged);
}

void DesktopSwitch::registerShortcuts()
{
    // Register shortcuts to change desktop
    GlobalKeyShortcut::Action * gshortcut;
    QString path;
    QString description;
    for (int i = 0; i < 12; ++i)
    {
        path = QString("/panel/%1/desktop_%2").arg(settings()->group()).arg(i + 1);
        description = tr("Switch to desktop %1").arg(i + 1);

        gshortcut = GlobalKeyShortcut::Client::instance()->addAction(QStringLiteral(), path, description, this);
        if (nullptr != gshortcut)
        {
            m_keys << gshortcut;
            connect(gshortcut, &GlobalKeyShortcut::Action::registrationFinished, this, &DesktopSwitch::shortcutRegistered);
            connect(gshortcut, SIGNAL(activated()), m_pSignalMapper, SLOT(map()));
            m_pSignalMapper->setMapping(gshortcut, i);
        }
    }
}

void DesktopSwitch::shortcutRegistered()
{
    GlobalKeyShortcut::Action * const shortcut = qobject_cast<GlobalKeyShortcut::Action*>(sender());

    disconnect(shortcut, &GlobalKeyShortcut::Action::registrationFinished, this, &DesktopSwitch::shortcutRegistered);

    const int i = m_keys.indexOf(shortcut);
    Q_ASSERT(-1 != i);

    if (shortcut->shortcut().isEmpty())
    {
        shortcut->changeShortcut(DEFAULT_SHORTCUT_TEMPLATE.arg(i + 1));
    }
}

void DesktopSwitch::onWindowChanged(WId id, NET::Properties properties, NET::Properties2 properties2)
{
    if (properties.testFlag(NET::WMState))
    {
        KWindowInfo info = KWindowInfo(id, NET::WMDesktop | NET::WMState);
        int desktop = info.desktop();
        if (!info.valid() || info.onAllDesktops())
            return;
        else
        {
            DesktopSwitchButton *button = static_cast<DesktopSwitchButton *>(m_buttons->button(desktop - 1));
            if(button)
                button->setUrgencyHint(id, info.hasState(NET::DemandsAttention));
        }
    }
}

void DesktopSwitch::refresh()
{
    QList<QAbstractButton*> btns = m_buttons->buttons();

    int i = 0;
    const int current_cnt = btns.count();
    const int border = qMin(btns.count(), m_desktopCount);
    //update existing buttons
    for ( ; i < border; ++i)
    {
        ((DesktopSwitchButton*)m_buttons->button(i))->update(i, mLabelType,
                       KWindowSystem::desktopName(i + 1).isEmpty() ?
                       tr("Desktop %1").arg(i + 1) :
                       KWindowSystem::desktopName(i + 1));
    }

    //create new buttons (if neccessary)
    QAbstractButton *b;
    for ( ; i < m_desktopCount; ++i)
    {
        b = new DesktopSwitchButton(&mWidget, i, mLabelType,
                KWindowSystem::desktopName(i+1).isEmpty() ?
                tr("Desktop %1").arg(i+1) :
                KWindowSystem::desktopName(i+1));
        mWidget.layout()->addWidget(b);
        m_buttons->addButton(b, i);
    }

    //delete unneeded buttons (if neccessary)
    for ( ; i < current_cnt; ++i)
    {
        b = m_buttons->buttons().last();
        m_buttons->removeButton(b);
        mWidget.layout()->removeWidget(b);
        delete b;
    }
}

DesktopSwitch::~DesktopSwitch()
{
}

void DesktopSwitch::setDesktop(int desktop)
{
    KWindowSystem::setCurrentDesktop(desktop + 1);
}

void DesktopSwitch::onNumberOfDesktopsChanged(int count)
{
    qDebug() << "Desktop count changed from" << m_desktopCount << "to" << count;
    m_desktopCount = count;
    refresh();
}

void DesktopSwitch::onCurrentDesktopChanged(int current)
{
    QAbstractButton *button = m_buttons->button(current - 1);
    if (button)
        button->setChecked(true);
}

void DesktopSwitch::onDesktopNamesChanged()
{
    refresh();
}

void DesktopSwitch::settingsChanged()
{
    int value = settings()->value("rows", 1).toInt();
    if (mRows != value)
    {
        mRows = value;
        realign();
    }
    value = settings()->value("labelType", DesktopSwitchButton::LABEL_TYPE_NUMBER).toInt();
    if (mLabelType != static_cast<DesktopSwitchButton::LabelType>(value))
    {
        mLabelType = static_cast<DesktopSwitchButton::LabelType>(value);
        refresh();
    }
}

void DesktopSwitch::realign()
{
    int columns = static_cast<int>(ceil(static_cast<float>(m_desktopCount) / mRows));
    mLayout->setEnabled(false);
    if (panel()->isHorizontal())
    {
        mLayout->setRowCount(mRows);
        mLayout->setColumnCount(0);
        mDesktops->setDesktopLayout(NET::OrientationHorizontal, columns, mRows, NET::DesktopLayoutCornerTopLeft);
    }
    else
    {
        mLayout->setColumnCount(mRows);
        mLayout->setRowCount(0);
        mDesktops->setDesktopLayout(NET::OrientationHorizontal, mRows, columns, NET::DesktopLayoutCornerTopLeft);
    }
    mLayout->setEnabled(true);
}

QDialog *DesktopSwitch::configureDialog()
{
    return new DesktopSwitchConfiguration(settings());
}

DesktopSwitchWidget::DesktopSwitchWidget():
    QFrame(),
    m_mouseWheelThresholdCounter(0)
{
}

void DesktopSwitchWidget::wheelEvent(QWheelEvent *e)
{
    // Without some sort of threshold which has to be passed, scrolling is too sensitive
    m_mouseWheelThresholdCounter -= e->delta();
    // If the user hasn't scrolled far enough in one direction (positive or negative): do nothing
    if(abs(m_mouseWheelThresholdCounter) < 100)
        return;

    int max = KWindowSystem::numberOfDesktops();
    int delta = e->delta() < 0 ? 1 : -1;
    int current = KWindowSystem::currentDesktop() + delta;

    if (current > max){
        current = 1;
    }
    else if (current < 1)
        current = max;

    m_mouseWheelThresholdCounter = 0;
    KWindowSystem::setCurrentDesktop(current);
}
