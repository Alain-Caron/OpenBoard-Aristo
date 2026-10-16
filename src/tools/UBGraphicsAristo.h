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

    enum class InteractionMode { Free, Parallel, Perpendicular };
    bool isInteractionLocked() const { return mLocked; }
    int interactionModeValue() const { return static_cast<int>(mMode); }
    qreal symmetricLengthCm() const { return mSymmetricLengthCm; }
    void setAristoState(bool locked, int mode, qreal symmetricLengthCm);

protected:
    void paintGraduations(QPainter* painter) override;
    QPointF rotationCenter() const override;
    bool usesSymmetricGeometry() const override { return true; }
    bool allowsMoveInteraction() const override { return !mLocked; }
    bool allowsRotateInteraction() const override
        { return !mLocked && mMode == InteractionMode::Free; }
    bool allowsResizeInteraction() const override
        { return !mLocked && mMode == InteractionMode::Free; }
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

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
    void paintControls(QPainter* painter, const QRectF& bounds,
                       bool baseAtBottom);
    void paintDynamicMeasurements(QPainter* painter, const QRectF& bounds,
                                  bool baseAtBottom);
    QVector<QRectF> controlRects(const QRectF& bounds, bool baseAtBottom) const;
    qreal displayedRotation() const;
    QLineF mDrawingGuide;
    qreal mStrokeWidth;
    bool mLocked;
    InteractionMode mMode;
    int mPressedControl;
    qreal mSymmetricLengthCm;
    bool mDrawingGuideVisible;
    qreal mMeasuredAngle;
    bool mMeasuredAngleVisible;
};

#endif // UBGRAPHICSARISTO_H
