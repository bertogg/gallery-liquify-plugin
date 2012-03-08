/*
 * quillliquifyplugin.h
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

#ifndef __quillliquifyplugin_h__
#define __quillliquifyplugin_h__

#include <QuillImageFilter>
#include <QuillImageFilterInterface>
#include <QuillImageFilterImplementation>

class QuillLiquifyPlugin:
    public QObject,
    public QuillImageFilterInterface
{
    Q_OBJECT
    Q_INTERFACES(QuillImageFilterInterface)

public:
    QuillLiquifyPlugin();
    QuillImageFilterImplementation* create(const QString& name);
    const QStringList name() const;

private:
    Q_DISABLE_COPY(QuillLiquifyPlugin)
};


class Liquify:
    public QuillImageFilterImplementation
{
public:
    Liquify();
    ~Liquify();

    QuillImage apply(const QuillImage& image) const;
    virtual const QString name() const;
    bool setOption(const QString &option, const QVariant &value);
    QVariant option(const QString &option) const;

private:
    bool enlarge;
    int radius;
    QPoint pos;
};

#endif /* !__quillliquifyplugin_h__ */

