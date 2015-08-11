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

class ElidedButtonStyle: public QProxyStyle
{
public:
    ElidedButtonStyle(QStyle* style=0): QProxyStyle(style) {}

    void drawItemText(QPainter* painter, const QRect& rect, int flags,
                      const QPalette & pal, bool enabled, const QString & text,
                      QPalette::ColorRole textRole = QPalette::NoRole ) const;
};


class LxQtTaskButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY(Qt::Corner origin READ origin WRITE setOrigin)

public:
    explicit LxQtTaskButton(const WId window, QWidget *parent = 0);
    virtual ~LxQtTaskButton();

    bool isAppHidden() const;
    bool isApplicationActive() const;
    WId windowId() const { return mWindow; }

    bool hasUrgencyHint() const { return mUrgencyHint; }
    void setUrgencyHint(bool set);

    int desktopNum() const;
    void updateText();
    void updateIcon();

    Qt::Corner origin() const;
    void setAutoRotation(bool value, ILxQtPanel::Position position);

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
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *);

private:
    WId mWindow;
    bool mUrgencyHint;
    const QMimeData *mDraggableMimeData;
    QPoint mDragStartPosition;
    Qt::Corner mOrigin;
    QPixmap mPixmap;
    bool mDrawPixmap;

private slots:
    void activateWithDraggable();
};

typedef QHash<WId,LxQtTaskButton*> LxQtTaskButtonHash;

#endif // LXQTTASKBUTTON_H
