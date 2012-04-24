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
    m_radiusSlider(0),
    m_enabled(true)
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

void GalleryLiquifyPlugin::activate()
{
    if (editUiProvider()) {
        QSize size = editUiProvider()->fullImageSize();
        m_enabled = size.height() <= 512 && size.width() <= 512;
        if (!m_enabled) {
            showMessageBox("Liquify plugin limitations",
                           "The Gallery Liquify Plugin is currently "
                           "limited to small images (up to 512x512).");
        }
    }
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
            if (m_enabled) {
                performEditOperation();
            } else {
                showInfoBanner("Plugin disabled for this image size");
            }
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
    if (provider && m_enabled) {
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

MMessageBox *
GalleryLiquifyPlugin::showMessageBox(const QString& title, const QString& text) const
{
    MMessageBox* messageBox = new MMessageBox(title, "");
    MLabel* innerLabel = new MLabel(messageBox);
    innerLabel->setWordWrap(true);
    innerLabel->setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    innerLabel->setStyleName("CommonQueryText");
    innerLabel->setText(text);
    innerLabel->setAlignment(Qt::AlignHCenter);
    messageBox->setCentralWidget(innerLabel);

    connect(innerLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(onAboutLinkActivated(QString)));
    connect(this, SIGNAL(deactivated()),
            messageBox, SLOT(disappear()));

    messageBox->appear(MSceneWindow::DestroyWhenDone);

    return messageBox;
}

MBanner *
GalleryLiquifyPlugin::showInfoBanner(const QString& title) const
{
    MBanner *infoBanner = new MBanner;
    infoBanner->setTitle(title);
    infoBanner->setStyleName("InformationBanner");
    infoBanner->model()->setDisappearTimeout(2000);
    connect(this, SIGNAL(deactivated()),
            infoBanner, SLOT(disappear()));

    infoBanner->appear(MApplication::activeWindow(), MSceneWindow::DestroyWhenDone);

    return infoBanner;
}

Q_EXPORT_PLUGIN2(galleryliquifyplugin, GalleryLiquifyPlugin)

