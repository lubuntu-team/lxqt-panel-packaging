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
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
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


#ifndef LXQTTASKBUTTON_H
#define LXQTTASKBUTTON_H

#include <QToolButton>
#include <QProxyStyle>
#include "../panel/ilxqtpanel.h"

class QPainter;
class QPalette;
class QMimeData;
class LXQtTaskGroup;
class LXQtTaskBar;

class LeftAlignedTextStyle : public QProxyStyle
{
    using QProxyStyle::QProxyStyle;
public:

    virtual void drawItemText(QPainter * painter, const QRect & rect, int flags
            , const QPalette & pal, bool enabled, const QString & text
            , QPalette::ColorRole textRole = QPalette::NoRole) const override;
};


class LXQtTaskButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY(Qt::Corner origin READ origin WRITE setOrigin)

public:
    explicit LXQtTaskButton(const WId window, LXQtTaskBar * taskBar, QWidget *parent = 0);
    virtual ~LXQtTaskButton();

    bool isApplicationHidden() const;
    bool isApplicationActive() const;
    WId windowId() const { return mWindow; }

    bool hasUrgencyHint() const { return mUrgencyHint; }
    void setUrgencyHint(bool set);

    bool isOnDesktop(int desktop) const;
    bool isOnCurrentScreen() const;
    bool isMinimized() const;
    void updateText();
    void updateIcon();

    Qt::Corner origin() const;
    virtual void setAutoRotation(bool value, ILXQtPanel::Position position);

    LXQtTaskGroup * parentGroup(void) const {return mParentGroup;}
    void setParentGroup(LXQtTaskGroup * group) {mParentGroup = group;}
    LXQtTaskBar * parentTaskBar() const {return mParentTaskBar;}

    void refreshIconGeometry(QRect const & geom);
    static QString mimeDataFormat() { return QLatin1String("lxqt/lxqttaskbutton"); }
    /*! \return true if this buttom received DragEnter event (and no DragLeave event yet)
     * */
    bool hasDragAndDropHover() const;

public slots:
    void raiseApplication();
    void minimizeApplication();
    void maximizeApplication();
    void deMaximizeApplication();
    void shadeApplication();
    void unShadeApplication();
    void closeApplication();
    void moveApplicationToDesktop();
    void setApplicationLayer();

    void setOrigin(Qt::Corner);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent * event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *);

    void setWindowId(WId wid) {mWindow = wid;}
    virtual QMimeData * mimeData();
    static bool sDraggging;

private:
    WId mWindow;
    bool mUrgencyHint;
    QPoint mDragStartPosition;
    Qt::Corner mOrigin;
    QPixmap mPixmap;
    bool mDrawPixmap;
    LXQtTaskGroup * mParentGroup;
    LXQtTaskBar * mParentTaskBar;

    // Timer for when draggind something into a button (the button's window
    // must be activated so that the use can continue dragging to the window
    QTimer * mDNDTimer;

private slots:
    void activateWithDraggable();

signals:
    void dropped(QObject * dragSource, QPoint const & pos);
    void dragging(QObject * dragSource, QPoint const & pos);
};

typedef QHash<WId,LXQtTaskButton*> LXQtTaskButtonHash;

#endif // LXQTTASKBUTTON_H
