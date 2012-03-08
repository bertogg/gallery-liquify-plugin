/*
 * quillliquifyplugin.cpp
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

#include "quillliquifyplugin.h"
#include <QDebug>
#include <QPainter>

#define FILTER_NAME_LIQUIFY "com.igalia.liquify"

QuillLiquifyPlugin::QuillLiquifyPlugin():
    QObject()
{
}

QuillImageFilterImplementation*
QuillLiquifyPlugin::create(const QString& name)
{
    if (name == FILTER_NAME_LIQUIFY) {
        return new Liquify;
    } else {
        return 0;
    }
}

const QStringList QuillLiquifyPlugin::name() const
{
    return QStringList() << FILTER_NAME_LIQUIFY;
}

Q_EXPORT_PLUGIN2(quillliquifyplugin, QuillLiquifyPlugin)


Liquify::Liquify()
{
    radius = 50;
}

Liquify::~Liquify()
{
}

QuillImage
Liquify::apply(const QuillImage& image) const
{
    QuillImage result(image);
    QPainter painter(&result);
    QSize origSize = image.size();
    QSize totalSize = image.fullImageSize();
    QPoint pos = QPoint(this->pos.x() * origSize.width() / totalSize.width(),
                        this->pos.y() * origSize.height() / totalSize.height());
    int x = qMax(pos.x() - radius, 0);
    int y = qMax(pos.y() - radius, 0);
    int w = pos.x() + radius - x;
    int h = pos.y() + radius - y;
    painter.fillRect(x, y, w, h, Qt::SolidPattern);
    return result;
}

bool
Liquify::setOption(const QString &option, const QVariant &value)
{
    bool retValue = true;

    if (option == "pos") {
        this->pos = value.toPoint();
    } else if (option == "radius") {
        this->radius = value.toInt();
    } else if (option == "enlarge") {
        this->enlarge = value.toBool();
    } else {
        retValue = false;
    }

    return retValue;
}

QVariant
Liquify::option(const QString &option) const
{
    if (option == "pos") {
        return QVariant(this->pos);
    } else if (option == "radius") {
        return QVariant(this->radius);
    } else if (option == "enlarge") {
        return QVariant(this->enlarge);
    } else {
        return QVariant();
    }
}

const QString
Liquify::name() const
{
    return FILTER_NAME_LIQUIFY;
}
