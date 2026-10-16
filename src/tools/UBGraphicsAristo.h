/*
 * Copyright (C) 2026 OpenBoard contributors
 *
 * This file is part of OpenBoard and is distributed under the terms of
 * the GNU General Public License, version 3, with OpenBoard's OpenSSL
 * linking exception.
 */

#ifndef UBGRAPHICSARISTO_H
#define UBGRAPHICSARISTO_H

#include <QLineF>
#include <QVector>

#include "tools/UBGraphicsTriangle.h"

/**
 * Native Aristo-style set square combining a centred ruler, a 45-degree
 * set square and a mathematically generated 0..180 degree protractor.
 *
 * Movement, rotation, resizing, flipping and edge drawing are inherited
 * from UBGraphicsTriangle so the tool behaves like OpenBoard's instruments.
 */
class UBGraphicsAristo : public UBGraphicsTriangle
{
    Q_OBJECT

public:
    UBGraphicsAristo();

    enum { Type = UBGraphicsItemType::AristoItemType };
    int type() const override { return Type; }

    UBItem* deepCopy() const override;
    void copyItemParameters(UBItem* copy) const override;
    void StartLine(const QPointF& scenePos, qreal width) override;
    void DrawLine(const QPointF& scenePos, qreal width) override;
    void EndLine() override;

protected:
    void paintGraduations(QPainter* painter) override;
    QPointF rotationCenter() const override;
    bool usesSymmetricGeometry() const override { return true; }

private:
    QPointF constrainedPoint(const QPointF& scenePos) const;
    QVector<QLineF> linearDrawingGuides(const QRectF& bounds, bool baseAtBottom) const;
    QVector<QLineF> angularDrawingGuides(const QRectF& bounds, bool baseAtBottom) const;
    void paintParallelAndPerpendicularGuides(QPainter* painter,
                                              const QRectF& bounds,
                                              bool baseAtBottom);
    void paintPerpendicularScale(QPainter* painter, const QRectF& bounds,
                                 bool baseAtBottom);
    void paintCentredRuler(QPainter* painter, const QRectF& bounds,
                           bool baseAtBottom);
    void paintProtractor(QPainter* painter, const QRectF& bounds,
                         bool baseAtBottom);
    QLineF mDrawingGuide;
    qreal mStrokeWidth;
};

#endif // UBGRAPHICSARISTO_H
