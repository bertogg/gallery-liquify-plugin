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
#include <math.h>

#define FILTER_NAME_LIQUIFY "com.igalia.liquify"

#define MAX_DEFORM_AREA_RADIUS 100

struct vector_2d {
    double x;
    double y;
};

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
    deform_area_vectors = new vector_2d[(MAX_DEFORM_AREA_RADIUS * 2 + 1) *
                                        (MAX_DEFORM_AREA_RADIUS * 2 + 1)];
    filter = new double[MAX_DEFORM_AREA_RADIUS];
    for (int i = 0; i < MAX_DEFORM_AREA_RADIUS; i++) {
        filter[i] =
            pow((cos(sqrt((double)i / MAX_DEFORM_AREA_RADIUS) * M_PI) + 1) * 0.5, 0.7);
    }
}

Liquify::~Liquify()
{
    delete[] deform_area_vectors;
    delete[] filter;
}

QRgb
Liquify::get_pixel(const QuillImage &img, int x, int y) const
{
    if (x < 0 || x >= img_width || y < 0 || y >= img_height) {
        return qRgba(0, 0, 0, 0);
    }
    return img.pixel(x, y);
}

QRgb
Liquify::get_point(const QuillImage &img, double x, double y) const
{
    int xi,yi;
    double dx,dy,m0,m1;
    QRgb p0, p1, p2, p3;
    int r, g, b, a;

    xi = (int) x;
    yi = (int) y;
    dx = x - xi;
    dy = y - yi;

    p0 = get_pixel(img, xi,     yi);
    p1 = get_pixel(img, xi + 1, yi);
    p2 = get_pixel(img, xi,     yi + 1);
    p3 = get_pixel(img, xi + 1, yi + 1);

    m0 = qRed(p0) + dx * (qRed(p1) - qRed(p0));
    m1 = qRed(p2) + dx * (qRed(p3) - qRed(p2));
    r = m0 + dy * (m1 - m0);

    m0 = qGreen(p0) + dx * (qGreen(p1) - qGreen(p0));
    m1 = qGreen(p2) + dx * (qGreen(p3) - qGreen(p2));
    g = m0 + dy * (m1 - m0);

    m0 = qBlue(p0) + dx * (qBlue(p1) - qBlue(p0));
    m1 = qBlue(p2) + dx * (qBlue(p3) - qBlue(p2));
    b = m0 + dy * (m1 - m0);

    m0 = qAlpha(p0) + dx * (qAlpha(p1) - qAlpha(p0));
    m1 = qAlpha(p2) + dx * (qAlpha(p3) - qAlpha(p2));
    a = m0 + dy * (m1 - m0);

    return qRgba(r, g, b, a);
}

void
Liquify::get_deform_vector(double x, double y, double *xv, double *yv) const
{
    if (x >= 0 && x < (img_width - 1) && y >= 0  && y < (img_height - 1)) {
        double my0, my1, mx0, mx1;
        int xi = x;
        int yi = y;
        double dx = x - xi;
        double dy = y - yi;
        int i = (yi * img_width + xi);
        mx0 = deform_vectors[i].x + (deform_vectors[i + 1].x - deform_vectors[i].x) * dx;
        mx1 = deform_vectors[i + img_width].x + (deform_vectors[i + img_width + 1].x - deform_vectors[i + img_width].x) * dx;
        my0 = deform_vectors[i].y + dx * (deform_vectors[i + 1].y - deform_vectors[i].y);
        my1 = deform_vectors[i + img_width].y + dx * (deform_vectors[i + img_width + 1].y - deform_vectors[i + img_width].y);
        *xv = mx0 + dy * (mx1 - mx0);
        *yv = my0 + dy * (my1 - my0);
    } else {
        *xv = *yv = 0.0;
    }
}

uchar
Liquify::transparent_color(int x, int y) const
{
    const int check_size  = 32;
    const int check_light = 170;
    const int check_dark  = 85;
    if ((y % check_size) > (check_size / 2)) {
        if ((x % check_size) > (check_size / 2))
            return check_dark;
        else
            return check_light;
    } else {
        if ((x % check_size) < (check_size / 2))
            return check_dark;
        else
            return check_light;
    }
}

void
Liquify::deform(const QuillImage &srcImg, QuillImage &dstImg, int x, int y, double vx, double vy) const
{
    const int r = this->radius;
    const int r2 = r * r;

    int x0 = (x - r < 0) ? -x : -r;
    int x1 = (x + r >= img_width) ? img_width - x - 1 : r;
    int y0 = (y - r < 0) ? -y : -r;
    int y1 = (y + r >= img_height) ? img_height - y - 1 : r;

    for (int yi = y0; yi <= y1; yi++)
        for (int xi = x0; xi <= x1; xi++) {
            int length2 = (xi * xi + yi * yi) * MAX_DEFORM_AREA_RADIUS / r2;
            if (length2 < MAX_DEFORM_AREA_RADIUS) {
                double nvx, nvy;
                int fptr = (yi + r) * (r * 2 + 1) + xi + r;

                {
                    double deform_value = filter[length2] * 0.3;
                    nvx = deform_value * vx;
                    nvy = deform_value * vy;
                }

                {
                    double emh, em, edge_width;
                    edge_width = 0.2 * r;
                    emh = em = 1.0;

                    if (x+xi < edge_width)
                        em = (double)(x + xi) / edge_width;

                    if (y+yi < edge_width)
                        emh = (double)(y + yi) / edge_width;

                    if (emh < em)
                        em = emh;

                    if (img_width - x - xi - 1 < edge_width)
                        emh = (double)(img_width - x - xi - 1) / edge_width;

                    if (emh < em)
                        em = emh;

                    if (img_height - y - yi - 1 < edge_width)
                        emh = (double)(img_height - y - yi - 1) / edge_width;

                    if (emh < em)
                        em = emh;

                    nvx = nvx * em;
                    nvy = nvy * em;
                }

                {
                    double xv, yv;
                    get_deform_vector(nvx + x + xi, nvy + y + yi, &xv, &yv);

                    xv = nvx + xv;

                    if (xv + x + xi < 0.0)
                        xv = -x - xi;
                    else if (xv + x + xi > (img_width - 1))
                        xv = img_width - x - xi - 1;

                    yv = nvy + yv;

                    if (yv + y + yi < 0.0)
                        yv = -y-yi;
                    else if (yv + y + yi > (img_height - 1))
                        yv = img_height - y - yi - 1;

                    deform_area_vectors[fptr].x = xv;
                    deform_area_vectors[fptr].y = yv;
                }

                double xn = deform_area_vectors[fptr].x + x + xi;
                double yn = deform_area_vectors[fptr].y + y + yi;

                QRgb rgbColor = get_point(srcImg, xn, yn);
                uchar transp  = transparent_color(x + xi, y + yi);

                double alpha = (double) qAlpha(rgbColor) / 255;
                int red      = (uchar) (alpha * qRed(rgbColor)   + (1.0 - alpha) * transp);
                int green    = (uchar) (alpha * qGreen(rgbColor) + (1.0 - alpha) * transp);
                int blue     = (uchar) (alpha * qBlue(rgbColor)  + (1.0 - alpha) * transp);
                dstImg.setPixel(x + xi, y + yi, qRgba(red, green, blue, alpha));
            }
        }

    for (int yi = y0; yi <= y1; yi++)
        for (int xi = x0; xi <= x1; xi++) {
            int length2 = (xi * xi + yi * yi) * MAX_DEFORM_AREA_RADIUS / r2;
            if (length2 < MAX_DEFORM_AREA_RADIUS) {
                int ptr  = (yi + y) * img_width   + xi + x;
                int fptr = (yi + r) * (r * 2 + 1) + xi + r;
                deform_vectors[ptr].x = deform_area_vectors[fptr].x;
                deform_vectors[ptr].y = deform_area_vectors[fptr].y;
            }
        }
}

QuillImage
Liquify::apply(const QuillImage& image) const
{
    QuillImage result(image);
    QPainter painter(&result);
    QSize origSize = image.size();
    QSize totalSize = image.fullImageSize();
    double ratiox = (double) origSize.width() / totalSize.width();
    double ratioy = (double) origSize.height() / totalSize.height();
    int x1 = this->posFrom.x() * ratiox;
    int y1 = this->posFrom.y() * ratioy;
    int x2 = this->posTo.x() * ratiox;
    int y2 = this->posTo.y() * ratioy;
    double dx = x2 - x1;
    double dy = y2 - y1;
    double l = sqrt(dx*dx + dy*dy);
    int num = (int) (l * 2 / radius) + 1;
    dx /= num;
    dy /= num;

    img_width = origSize.width();
    img_height = origSize.height();

    deform_vectors = new vector_2d[img_width * img_height];

    for (int i = 0; i < img_width * img_height; i++)
        deform_vectors[i].x = deform_vectors[i].y = 0.0;

    double xf = x1 + dx;
    double yf = y1 + dy;

    for (int i = 0; i < num; i++) {
        deform (image, result, xf, yf, -dx, -dy);
        xf += dx;
        yf += dy;
    }

    delete[] deform_vectors; deform_vectors = 0;

    return result;
}

bool
Liquify::setOption(const QString &option, const QVariant &value)
{
    bool retValue = true;

    if (option == "posFrom") {
        this->posFrom = value.toPoint();
    } else if (option == "posTo") {
        this->posTo = value.toPoint();
    } else if (option == "radius") {
        this->radius = value.toInt();
    } else {
        retValue = false;
    }

    return retValue;
}

QVariant
Liquify::option(const QString &option) const
{
    if (option == "posFrom") {
        return QVariant(this->posFrom);
    } else if (option == "posTo") {
        return QVariant(this->posTo);
    } else if (option == "radius") {
        return QVariant(this->radius);
    } else {
        return QVariant();
    }
}

const QString
Liquify::name() const
{
    return FILTER_NAME_LIQUIFY;
}
