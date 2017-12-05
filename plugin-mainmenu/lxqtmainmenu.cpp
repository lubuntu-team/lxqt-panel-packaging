/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
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


#include "lxqtmainmenu.h"
#include "lxqtmainmenuconfiguration.h"
#include "../panel/lxqtpanel.h"
#include "actionview.h"
#include <QAction>
#include <QTimer>
#include <QMessageBox>
#include <QEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QWidgetAction>
#include <QLineEdit>
#include <lxqt-globalkeys.h>
#include <algorithm> // for find_if()
#include <KWindowSystem/KWindowSystem>
#include <QApplication>

#include <XdgMenuWidget>

#ifdef HAVE_MENU_CACHE
#include "xdgcachedmenu.h"
#endif

#define DEFAULT_SHORTCUT "Alt+F1"

LXQtMainMenu::LXQtMainMenu(const ILXQtPanelPluginStartupInfo &startupInfo):
    QObject(),
    ILXQtPanelPlugin(startupInfo),
    mMenu(0),
    mShortcut(0),
    mSearchEditAction{new QWidgetAction{this}},
    mSearchViewAction{new QWidgetAction{this}},
    mMakeDirtyAction{new QAction{this}},
    mFilterMenu(true),
    mFilterShow(true),
    mFilterClear(false),
    mFilterShowHideMenu(true),
    mHeavyMenuChanges(false)
{
#ifdef HAVE_MENU_CACHE
    mMenuCache = NULL;
    mMenuCacheNotify = 0;
#endif

    mDelayedPopup.setSingleShot(true);
    mDelayedPopup.setInterval(200);
    connect(&mDelayedPopup, &QTimer::timeout, this, &LXQtMainMenu::showHideMenu);
    mHideTimer.setSingleShot(true);
    mHideTimer.setInterval(250);

    mButton.setAutoRaise(true);
    mButton.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    //Notes:
    //1. installing event filter to parent widget to avoid infinite loop
    //   (while setting icon we also need to set the style)
    //2. delaying of installEventFilter because in c-tor mButton has no parent widget
    //   (parent is assigned in panel's logic after widget() call)
    QTimer::singleShot(0, [this] { Q_ASSERT(mButton.parentWidget()); mButton.parentWidget()->installEventFilter(this); });

    connect(&mButton, &QToolButton::clicked, this, &LXQtMainMenu::showHideMenu);

    mSearchView = new ActionView;
    mSearchView->setVisible(false);
    connect(mSearchView, &QAbstractItemView::activated, this, &LXQtMainMenu::showHideMenu);
    mSearchViewAction->setDefaultWidget(mSearchView);
    mSearchEdit = new QLineEdit;
    mSearchEdit->setClearButtonEnabled(true);
    mSearchEdit->setPlaceholderText(LXQtMainMenu::tr("Search..."));
    connect(mSearchEdit, &QLineEdit::textChanged, this, &LXQtMainMenu::searchTextChanged);
    connect(mSearchEdit, &QLineEdit::returnPressed, mSearchView, &ActionView::activateCurrent);
    mSearchEditAction->setDefaultWidget(mSearchEdit);
    QTimer::singleShot(0, [this] { settingsChanged(); });

    mShortcut = GlobalKeyShortcut::Client::instance()->addAction(QString{}, QString("/panel/%1/show_hide").arg(settings()->group()), LXQtMainMenu::tr("Show/hide main menu"), this);
    if (mShortcut)
    {
        connect(mShortcut, &GlobalKeyShortcut::Action::registrationFinished, [this] {
            if (mShortcut->shortcut().isEmpty())
                mShortcut->changeShortcut(DEFAULT_SHORTCUT);
        });
        connect(mShortcut, &GlobalKeyShortcut::Action::activated, [this] {
            if (!mHideTimer.isActive())
                // Delay this a little -- if we don't do this, search field
                // won't be able to capture focus
                // See <https://github.com/lxde/lxqt-panel/pull/131> and
                // <https://github.com/lxde/lxqt-panel/pull/312>
                mDelayedPopup.start();
        });
    }
}


/************************************************

 ************************************************/
LXQtMainMenu::~LXQtMainMenu()
{
    mButton.parentWidget()->removeEventFilter(this);
    if (mMenu)
    {
        mMenu->removeAction(mSearchEditAction);
        mMenu->removeAction(mSearchViewAction);
        delete mMenu;
    }
#ifdef HAVE_MENU_CACHE
    if(mMenuCache)
    {
        menu_cache_remove_reload_notify(mMenuCache, mMenuCacheNotify);
        menu_cache_unref(mMenuCache);
    }
#endif
}


/************************************************

 ************************************************/
void LXQtMainMenu::showHideMenu()
{
    if (mMenu && mMenu->isVisible())
        mMenu->hide();
    else
        showMenu();
}

/************************************************

 ************************************************/
void LXQtMainMenu::showMenu()
{
    if (!mMenu)
        return;

    willShowWindow(mMenu);
    // Just using Qt`s activateWindow() won't work on some WMs like Kwin.
    // Solution is to execute menu 1ms later using timer
    mMenu->popup(calculatePopupWindowPos(mMenu->sizeHint()).topLeft());
    if (mFilterMenu || mFilterShow)
    {
        if (mFilterClear && !mSearchEdit->text().isEmpty())
        {
            mSearchEdit->setText(QString{});
        }
        //Note: part of the workadound for https://bugreports.qt.io/browse/QTBUG-52021
        mSearchEdit->setReadOnly(false);
        //the setReadOnly also changes the cursor, override it back to normal
        mSearchEdit->unsetCursor();
        mSearchEdit->setFocus();
    }
}

#ifdef HAVE_MENU_CACHE
// static
void LXQtMainMenu::menuCacheReloadNotify(MenuCache* cache, gpointer user_data)
{
    reinterpret_cast<LXQtMainMenu*>(user_data)->buildMenu();
}
#endif

/************************************************

 ************************************************/
void LXQtMainMenu::settingsChanged()
{
    setButtonIcon();
    if (settings()->value("showText", false).toBool())
    {
        mButton.setText(settings()->value("text", "Start").toString());
        mButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    else
    {
        mButton.setText("");
        mButton.setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    mLogDir = settings()->value("log_dir", "").toString();

    QString menu_file = settings()->value("menu_file", "").toString();
    if (menu_file.isEmpty())
        menu_file = XdgMenu::getMenuFileName();

    if (mMenuFile != menu_file)
    {
        mMenuFile = menu_file;
#ifdef HAVE_MENU_CACHE
        menu_cache_init(0);
        if(mMenuCache)
        {
            menu_cache_remove_reload_notify(mMenuCache, mMenuCacheNotify);
            menu_cache_unref(mMenuCache);
        }
        mMenuCache = menu_cache_lookup(mMenuFile.toLocal8Bit());
        if (MenuCacheDir * root = menu_cache_dup_root_dir(mMenuCache))
        {
            menu_cache_item_unref(MENU_CACHE_ITEM(root));
            buildMenu();
        }
        mMenuCacheNotify = menu_cache_add_reload_notify(mMenuCache, (MenuCacheReloadNotify)menuCacheReloadNotify, this);
#else
        mXdgMenu.setEnvironments(QStringList() << "X-LXQT" << "LXQt");
        mXdgMenu.setLogDir(mLogDir);

        bool res = mXdgMenu.read(mMenuFile);
        connect(&mXdgMenu, SIGNAL(changed()), this, SLOT(buildMenu()));
        if (res)
        {
            QTimer::singleShot(1000, this, SLOT(buildMenu()));
        }
        else
        {
            QMessageBox::warning(0, "Parse error", mXdgMenu.errorString());
            return;
        }
#endif
    }

    setMenuFontSize();

    //clear the search to not leaving the menu in wrong state
    mSearchEdit->setText(QString{});
    mFilterMenu = settings()->value("filterMenu", true).toBool();
    mFilterShow = settings()->value("filterShow", true).toBool();
    mFilterClear = settings()->value("filterClear", false).toBool();
    mFilterShowHideMenu = settings()->value("filterShowHideMenu", true).toBool();
    if (mMenu)
    {
        mSearchEdit->setVisible(mFilterMenu || mFilterShow);
        mSearchEditAction->setVisible(mFilterMenu || mFilterShow);
    }
    mSearchView->setMaxItemsToShow(settings()->value("filterShowMaxItems", 10).toInt());
    mSearchView->setMaxItemWidth(settings()->value("filterShowMaxWidth", 300).toInt());

    realign();
}

static bool filterMenu(QMenu * menu, QString const & filter)
{
    bool has_visible = false;
    for (auto const & action : menu->actions())
    {
        if (QMenu * sub_menu = action->menu())
        {
            action->setVisible(filterMenu(sub_menu, filter)/*recursion*/);
            has_visible |= action->isVisible();
        } else if (nullptr != qobject_cast<QWidgetAction *>(action))
        {
            //our searching widget
            has_visible = true;
        } else if (!action->isSeparator())
        {
            //real menu action -> app
            action->setVisible(filter.isEmpty() || action->text().contains(filter, Qt::CaseInsensitive) || action->toolTip().contains(filter, Qt::CaseInsensitive));
            has_visible |= action->isVisible();
        }
    }
    return has_visible;
}

static void showHideMenuEntries(QMenu * menu, bool show)
{
    //show/hide the top menu entries
    for (auto const & action : menu->actions())
    {
        if (nullptr == qobject_cast<QWidgetAction *>(action))
        {
            action->setVisible(show);
        }
    }
}

static void setTranslucentMenus(QMenu * menu)
{
    menu->setAttribute(Qt::WA_TranslucentBackground);
    for (auto const & action : menu->actions())
    {
        if (QMenu * sub_menu = action->menu())
        {
            setTranslucentMenus(sub_menu);
        }
    }
}

/************************************************

 ************************************************/
void LXQtMainMenu::searchTextChanged(QString const & text)
{
    if (mFilterShow)
    {
        mHeavyMenuChanges = true;
        const bool shown = !text.isEmpty();
        if (mFilterShowHideMenu)
            showHideMenuEntries(mMenu, !shown);
        if (shown)
            mSearchView->setFilter(text);
        mSearchView->setVisible(shown);
        mSearchViewAction->setVisible(shown);
        //TODO: how to force the menu to recalculate it's size in a more elegant way?
        mMenu->addAction(mMakeDirtyAction);
        mMenu->removeAction(mMakeDirtyAction);
        mHeavyMenuChanges = false;
    }
    if (mFilterMenu && !(mFilterShow && mFilterShowHideMenu))
        filterMenu(mMenu, text);

}

/************************************************

 ************************************************/
void LXQtMainMenu::setSearchFocus(QAction *action)
{
    if (mFilterMenu || mFilterShow)
    {
        if(action == mSearchEditAction)
            mSearchEdit->setFocus();
        else
            mSearchEdit->clearFocus();
    }
}

static void menuInstallEventFilter(QMenu * menu, QObject * watcher)
{
    for (auto const & action : const_cast<QList<QAction *> const &&>(menu->actions()))
    {
        if (action->menu())
            menuInstallEventFilter(action->menu(), watcher); // recursion
    }
    menu->installEventFilter(watcher);
}

/************************************************

 ************************************************/
void LXQtMainMenu::buildMenu()
{
    if(mMenu)
    {
        mMenu->removeAction(mSearchEditAction);
        mMenu->removeAction(mSearchViewAction);
        delete mMenu;
    }
#ifdef HAVE_MENU_CACHE
    mMenu = new XdgCachedMenu(mMenuCache, &mButton);
#else
    mMenu = new XdgMenuWidget(mXdgMenu, "", &mButton);
#endif
    mMenu->setObjectName("TopLevelMainMenu");
    setTranslucentMenus(mMenu);
    // Note: the QWidget::ensurePolished() workarounds problem with transparent
    // QLineEdit (mSearchEditAction) in menu with Breeze style
    // https://bugs.kde.org/show_bug.cgi?id=368048
    mMenu->ensurePolished();
    mMenu->setStyle(&mTopMenuStyle);

    mMenu->addSeparator();

    menuInstallEventFilter(mMenu, this);
    connect(mMenu, &QMenu::aboutToHide, &mHideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(mMenu, &QMenu::aboutToShow, &mHideTimer, &QTimer::stop);

    mMenu->addSeparator();
    mMenu->addAction(mSearchViewAction);
    mMenu->addAction(mSearchEditAction);
    connect(mMenu, &QMenu::hovered, this, &LXQtMainMenu::setSearchFocus);
    //Note: setting readOnly to true to avoid wake-ups upon the Qt's internal "blink" cursor timer
    //(if the readOnly is not set, the "blink" timer is active also in case the menu is not shown ->
    //QWidgetLineControl::updateNeeded is performed w/o any need)
    //https://bugreports.qt.io/browse/QTBUG-52021
    connect(mMenu, &QMenu::aboutToHide, [this] { mSearchEdit->setReadOnly(true); });
    mSearchEdit->setVisible(mFilterMenu || mFilterShow);
    mSearchEditAction->setVisible(mFilterMenu || mFilterShow);
    mSearchView->fillActions(mMenu);

    searchTextChanged(mSearchEdit->text());
    setMenuFontSize();
}

/************************************************

 ************************************************/
void LXQtMainMenu::setMenuFontSize()
{
    if (!mMenu)
        return;

    QFont menuFont = mButton.font();
    if(settings()->value("customFont", false).toBool())
    {
        menuFont = mMenu->font();
        menuFont.setPointSize(settings()->value("customFontSize").toInt());
    }

    if (mMenu->font() != menuFont)
    {
        mMenu->setFont(menuFont);
        QList<QMenu*> subMenuList = mMenu->findChildren<QMenu*>();
        foreach (QMenu* subMenu, subMenuList)
        {
            subMenu->setFont(menuFont);
        }
        mSearchEdit->setFont(menuFont);
        mSearchView->setFont(menuFont);
    }

    //icon size the same as the font height
    const int icon_size = QFontMetrics(menuFont).height();
    mTopMenuStyle.setIconSize(icon_size);
    mSearchView->setIconSize(QSize{icon_size, icon_size});
}


/************************************************

 ************************************************/
void LXQtMainMenu::setButtonIcon()
{
    if (settings()->value("ownIcon", false).toBool())
    {
        mButton.setStyleSheet(QString("#MainMenu { qproperty-icon: url(%1); }")
                .arg(settings()->value(QLatin1String("icon"), QLatin1String(LXQT_GRAPHICS_DIR"/helix.svg")).toString()));
    } else
    {
        mButton.setStyleSheet(QString());
    }
}


/************************************************

 ************************************************/
QDialog *LXQtMainMenu::configureDialog()
{
    return new LXQtMainMenuConfiguration(settings(), mShortcut, DEFAULT_SHORTCUT);
}
/************************************************

 ************************************************/

// functor used to match a QAction by prefix
struct MatchAction
{
    MatchAction(QString key):key_(key) {}
    bool operator()(QAction* action) { return action->text().startsWith(key_, Qt::CaseInsensitive); }
    QString key_;
};

bool LXQtMainMenu::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == mButton.parentWidget())
    {
        // the application is given a new QStyle
        if(event->type() == QEvent::StyleChange)
        {
            setMenuFontSize();
            setButtonIcon();
        }
    }
    else if(QMenu* menu = qobject_cast<QMenu*>(obj))
    {
        if(event->type() == QEvent::KeyPress)
        {
            // if our shortcut key is pressed while the menu is open, close the menu
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->modifiers() & ~Qt::ShiftModifier)
            {
                mHideTimer.start();
                mMenu->hide(); // close the app menu
                return true;
            }
            else // go to the menu item which starts with the pressed key if there is an active action.
            {
                QString key = keyEvent->text();
                if(key.isEmpty())
                    return false;
                QAction* action = menu->activeAction();
                if(action !=0) {
                    QList<QAction*> actions = menu->actions();
                    QList<QAction*>::iterator it = qFind(actions.begin(), actions.end(), action);
                    it = std::find_if(it + 1, actions.end(), MatchAction(key));
                    if(it == actions.end())
                        it = std::find_if(actions.begin(), it, MatchAction(key));
                    if(it != actions.end())
                        menu->setActiveAction(*it);
                }
            }
        }

        if (obj == mMenu)
        {
            if (event->type() == QEvent::Resize)
            {
                QResizeEvent * e = dynamic_cast<QResizeEvent *>(event);
                if (e->oldSize().isValid() && e->oldSize() != e->size())
                {
                    mMenu->move(calculatePopupWindowPos(e->size()).topLeft());
                }
            } else if (event->type() == QEvent::KeyPress)
            {
                QKeyEvent * e = dynamic_cast<QKeyEvent*>(event);
                if (Qt::Key_Escape == e->key())
                {
                    if (!mSearchEdit->text().isEmpty())
                    {
                        mSearchEdit->setText(QString{});
                        //filter out this to not close the menu
                        return true;
                    }
                }
            } else if (QEvent::ActionChanged == event->type()
                    || QEvent::ActionAdded == event->type())
            {
                //filter this if we are performing heavy changes to reduce flicker
                if (mHeavyMenuChanges)
                    return true;
            }
        }
    }
    return false;
}

#undef DEFAULT_SHORTCUT
