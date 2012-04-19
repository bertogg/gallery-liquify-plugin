/*
 * galleryliquifyplugin.h
 *
 * Copyright (C) 2012 Igalia, S.L.
 * Author: Alberto Garcia <agarcia@igalia.com>
 *
 * This file is part of the Gallery Liquify Plugin.
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see http://www.gnu.org/licenses/
 */

#ifndef __galleryliquifyplugin_h__
#define __galleryliquifyplugin_h__

#include <galleryeditplugin.h>
#include <QObject>
#include <QGraphicsSceneMouseEvent>
#include <MGridLayoutPolicy>
#include <MSlider>
#include <MButton>

class GalleryLiquifyPlugin: public GalleryEditPlugin
{
    Q_OBJECT
    Q_INTERFACES(GalleryEditPlugin);

public:
    GalleryLiquifyPlugin(QObject* parent = 0);
    virtual ~GalleryLiquifyPlugin();
    QString name() const;
    QString iconID() const;
    bool containsUi() const;
    bool zoomingAllowed() const;
    bool receiveMouseEvent(QGraphicsSceneMouseEvent *event);

public Q_SLOTS:
    void performEditOperation();

protected:
    QGraphicsWidget* createToolBarWidget(QGraphicsItem* parent = 0);

private:
    Q_DISABLE_COPY(GalleryLiquifyPlugin)
    QPoint pressPos;
    QPoint releasePos;
    MGridLayoutPolicy* m_landscapePolicy;
    MGridLayoutPolicy* m_portraitPolicy;
    MSlider *m_radiusSlider;
};

#endif /* !__galleryliquifyplugin_h__ */

