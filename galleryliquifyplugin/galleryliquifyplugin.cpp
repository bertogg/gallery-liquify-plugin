/*
 * galleryliquifyplugin.cpp
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
 *
 * The code for the liquify algorithm is based on the iwarp plugin 0.1
 * of The GIMP, which is (C) 1997 Norbert Schmitz and released under
 * the GNU GPL.
 */

#include "galleryliquifyplugin.h"
#include <galleryedituiprovider.h>
#include <QuillImageFilter>
#include <MLabel>
#include <MLayout>
#include <MApplication>

GalleryLiquifyPlugin::GalleryLiquifyPlugin(QObject* parent):
    GalleryEditPlugin(parent),
    m_landscapePolicy(0),
    m_portraitPolicy(0),
    m_radiusSlider(0)
{
}


GalleryLiquifyPlugin::~GalleryLiquifyPlugin()
{
}

bool
GalleryLiquifyPlugin::containsUi() const
{
    return true;
}

bool
GalleryLiquifyPlugin::zoomingAllowed() const
{
    return true;
}

QGraphicsWidget*
GalleryLiquifyPlugin::createToolBarWidget(QGraphicsItem* parent)
{
    Q_UNUSED(parent);
    MLabel *label = new MLabel();

    MLayout* mainLayout = new MLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setAnimation(0);

    m_landscapePolicy = new MGridLayoutPolicy(mainLayout);;
    m_portraitPolicy = new MGridLayoutPolicy(mainLayout);;;

    m_radiusSlider = new MSlider();
    m_radiusSlider->setRange(0, 100);
    m_radiusSlider->setValue(50);

    m_landscapePolicy->setContentsMargins(0,0,0,0);
    m_landscapePolicy->setSpacing(0);
    m_landscapePolicy->addItem(m_radiusSlider, 0, 0, Qt::AlignCenter);

    m_portraitPolicy->setContentsMargins(0,0,0,0);
    m_portraitPolicy->setSpacing(0);
    m_portraitPolicy->addItem(m_radiusSlider, 0, 0, Qt::AlignCenter);

    mainLayout->setLandscapePolicy(m_landscapePolicy);
    mainLayout->setPortraitPolicy(m_portraitPolicy);

    label->setLayout(mainLayout);

    // Make sure initial policy is correct
    if (MApplication::activeWindow()) {
        if (MApplication::activeWindow()->orientation() == M::Portrait) {
            m_portraitPolicy->activate();
        } else {
            m_landscapePolicy->activate();
        }
    } else {
        m_landscapePolicy->activate();
    }

    return label;
}

bool
GalleryLiquifyPlugin::receiveMouseEvent(QGraphicsSceneMouseEvent *event)
{
    Qt::MouseButtons button = event->button();
    Qt::MouseButtons buttons = event->buttons();
    if (button == Qt::LeftButton) {
        if (buttons & Qt::LeftButton) {
            this->pressPos = event->pos().toPoint();
        } else {
            this->releasePos = event->pos().toPoint();
            performEditOperation();
        }
    }
    return true;
}

QString
GalleryLiquifyPlugin::name() const
{
    return QString("Liquify");
}


QString
GalleryLiquifyPlugin::iconID() const
{
    return QString("icon-m-image-edit-liquify");
}


void GalleryLiquifyPlugin::performEditOperation()
{
    GalleryEditUiProvider *provider = editUiProvider();
    if (provider) {
        const QPoint imagePressPos =
            provider->convertScreenCoordToImageCoord(this->pressPos);
        const QPoint imageReleasePos =
            provider->convertScreenCoordToImageCoord(this->releasePos);
        if (imagePressPos != QPoint(-1, -1) && imageReleasePos != QPoint(-1, -1)) {
            QHash<QuillImageFilter::QuillFilterOption, QVariant> options;
            options.insert("posFrom", QVariant(imagePressPos));
            options.insert("posTo", QVariant(imageReleasePos));
            options.insert("radius", QVariant(m_radiusSlider->value()));
            provider->runEditFilter("com.igalia.liquify", options);
        }
    }
    emit editOperationPerformed();
}

Q_EXPORT_PLUGIN2(galleryliquifyplugin, GalleryLiquifyPlugin)

