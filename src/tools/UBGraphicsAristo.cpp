/*
 * Copyright (C) 2026 OpenBoard contributors
 *
 * This file is part of OpenBoard and is distributed under the terms of
 * the GNU General Public License, version 3, with OpenBoard's OpenSSL
 * linking exception.
 */

#include "tools/UBGraphicsAristo.h"

#include <cmath>
#include <limits>

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsScene.h"

namespace
{
constexpr qreal kPi = 3.14159265358979323846;
constexpr qreal kRulerInset = 8.0;
constexpr qreal kProtractorRadiusRatio = 0.31;
constexpr qreal kAngularGuideInnerRatio = 0.58;
}

UBGraphicsAristo::UBGraphicsAristo()
    : UBGraphicsTriangle()
    , mDrawingGuide()
    , mStrokeWidth(1.0)
{
    // Recalculate after construction: virtual dispatch is intentionally not
    // active while UBGraphicsTriangle's constructor is running.
    setRect(QRectF(0, 0, 800, 400), BottomLeft);
}

namespace
{
QPointF nearestPointOnLine(const QPointF& point, const QLineF& line)
{
    const QPointF vector = line.p2() - line.p1();
    const qreal squaredLength = QPointF::dotProduct(vector, vector);
    if (qFuzzyIsNull(squaredLength))
        return line.p1();
    const qreal factor = QPointF::dotProduct(point - line.p1(), vector) / squaredLength;
    return line.p1() + factor * vector;
}

QPointF nearestPointOnSegment(const QPointF& point, const QLineF& line)
{
    const QPointF vector = line.p2() - line.p1();
    const qreal squaredLength = QPointF::dotProduct(vector, vector);
    if (qFuzzyIsNull(squaredLength))
        return line.p1();
    const qreal factor = qBound(0.0,
        QPointF::dotProduct(point - line.p1(), vector) / squaredLength, 1.0);
    return line.p1() + factor * vector;
}
}

void UBGraphicsAristo::StartLine(const QPointF& scenePos, qreal width)
{
    Q_UNUSED(width);
    const QPointF point = mapFromScene(scenePos);
    const QRectF bounds = bounding_Rect();
    const bool baseAtBottom = getOrientation() == BottomLeft || getOrientation() == BottomRight;
    QVector<QLineF> guides = linearDrawingGuides(bounds, baseAtBottom);
    guides += angularDrawingGuides(bounds, baseAtBottom);

    qreal shortestDistance = std::numeric_limits<qreal>::max();
    for (const QLineF& guide : guides)
    {
        // Selection is measured against the finite printed guide. Projection
        // while drawing remains infinite, so the stroke may extend naturally
        // beyond the instrument, as with OpenBoard's native ruler.
        const qreal distance = QLineF(point, nearestPointOnSegment(point, guide)).length();
        if (distance < shortestDistance)
        {
            shortestDistance = distance;
            mDrawingGuide = guide;
        }
    }

    mStrokeWidth = UBDrawingController::drawingController()->currentToolWidth();
    const QPointF constrained = constrainedPoint(scenePos);
    scene()->moveTo(constrained);
    scene()->drawLineTo(constrained, mStrokeWidth, true);
}

void UBGraphicsAristo::DrawLine(const QPointF& scenePos, qreal width)
{
    Q_UNUSED(width);
    scene()->drawLineTo(constrainedPoint(scenePos), mStrokeWidth,
        UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Marker);
}

void UBGraphicsAristo::EndLine()
{
}

QPointF UBGraphicsAristo::constrainedPoint(const QPointF& scenePos) const
{
    const QPointF point = mapFromScene(scenePos);
    return mapToScene(nearestPointOnLine(point, mDrawingGuide));
}

QVector<QLineF> UBGraphicsAristo::linearDrawingGuides(const QRectF& bounds,
                                                      bool baseAtBottom) const
{
    QVector<QLineF> guides;
    const qreal apexY = baseAtBottom ? bounds.top() : bounds.bottom();
    const qreal baseY = baseAtBottom ? bounds.bottom() : bounds.top();
    const QPointF apex(bounds.center().x(), apexY);
    const QPointF left(bounds.left(), baseY);
    const QPointF right(bounds.right(), baseY);

    // The three physical outside edges.
    guides << QLineF(left, right) << QLineF(left, apex) << QLineF(apex, right);

    // Central 90-degree guide. It is deliberately a full line: once drawing
    // starts on the printed guide, the stroke can continue beyond the tool.
    guides << QLineF(QPointF(bounds.center().x(), apexY),
                     QPointF(bounds.center().x(), baseY));

    // Horizontal guides are separated by exactly one centimetre in the same
    // scene scale as OpenBoard's native ruler. Resizing reveals more or fewer
    // guides without changing their metric spacing.
    const qreal centimeters = UBApplication::boardController->activeScene()->backgroundGridSize();
    if (centimeters <= 0.0)
        return guides;
    for (qreal offset = centimeters; offset < bounds.height() * 0.72;
         offset += centimeters)
    {
        const qreal y = baseY + (baseAtBottom ? -1.0 : 1.0) * offset;
        const qreal progress = qAbs(y - apexY) / bounds.height();
        const qreal halfWidth = bounds.width() * progress / 2.0;
        guides << QLineF(QPointF(bounds.center().x() - halfWidth, y),
                         QPointF(bounds.center().x() + halfWidth, y));
    }
    return guides;
}

QVector<QLineF> UBGraphicsAristo::angularDrawingGuides(const QRectF& bounds,
                                                       bool baseAtBottom) const
{
    QVector<QLineF> guides;
    const QPointF origin(bounds.center().x(),
                         baseAtBottom ? bounds.bottom() : bounds.top());
    const qreal radius = qMin(bounds.width() * kProtractorRadiusRatio,
                              bounds.height() * 0.72);
    const qreal verticalSign = baseAtBottom ? -1.0 : 1.0;

    // One selectable radial guide per exact degree. Only the annular part
    // printed by the protractor participates in guide selection, preventing
    // the dense angular guides from stealing strokes started near the ruler.
    for (int degree = 0; degree <= 180; ++degree)
    {
        const qreal radians = degree * kPi / 180.0;
        const qreal cosine = std::cos(radians);
        const qreal sine = std::sin(radians);
        const QPointF inner(origin.x() - radius * kAngularGuideInnerRatio * cosine,
                            origin.y() + verticalSign * radius * kAngularGuideInnerRatio * sine);
        const QPointF outer(origin.x() - radius * cosine,
                            origin.y() + verticalSign * radius * sine);
        guides << QLineF(inner, outer);
    }
    return guides;
}

UBItem* UBGraphicsAristo::deepCopy() const
{
    UBGraphicsAristo* copy = new UBGraphicsAristo();
    copyItemParameters(copy);
    return copy;
}

void UBGraphicsAristo::copyItemParameters(UBItem* copy) const
{
    UBGraphicsAristo* aristo = dynamic_cast<UBGraphicsAristo*>(copy);
    if (!aristo)
        return;

    aristo->setPos(pos());
    aristo->setRect(bounding_Rect(), getOrientation());
    aristo->setTransform(transform());
}

QPointF UBGraphicsAristo::rotationCenter() const
{
    const QRectF bounds = bounding_Rect();
    const bool baseAtBottom = getOrientation() == BottomLeft || getOrientation() == BottomRight;
    return QPointF(bounds.center().x(), baseAtBottom ? bounds.bottom() : bounds.top());
}

void UBGraphicsAristo::paintGraduations(QPainter* painter)
{
    painter->save();
    painter->setPen(drawColor());
    painter->setFont(font());
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRectF bounds = bounding_Rect();
    const bool baseAtBottom = getOrientation() == BottomLeft || getOrientation() == BottomRight;
    paintParallelAndPerpendicularGuides(painter, bounds, baseAtBottom);
    paintCentredRuler(painter, bounds, baseAtBottom);
    paintPerpendicularScale(painter, bounds, baseAtBottom);
    paintProtractor(painter, bounds, baseAtBottom);

    painter->restore();
}

void UBGraphicsAristo::paintParallelAndPerpendicularGuides(QPainter* painter,
                                                            const QRectF& bounds,
                                                            bool baseAtBottom)
{
    const QVector<QLineF> guides = linearDrawingGuides(bounds, baseAtBottom);
    painter->save();
    QPen guidePen(drawColor());
    guidePen.setWidthF(1.15);
    guidePen.setStyle(Qt::DashLine);
    guidePen.setDashPattern({6.0, 3.0});
    painter->setPen(guidePen);

    // Skip the three outside edges, already painted by UBGraphicsTriangle.
    for (int index = 3; index < guides.size(); ++index)
        painter->drawLine(guides[index]);

    // Small, unambiguous right-angle marker at the ruler's zero.
    const qreal baseY = baseAtBottom ? bounds.bottom() : bounds.top();
    const qreal direction = baseAtBottom ? -1.0 : 1.0;
    const qreal marker = qMin(18.0, bounds.height() / 12.0);
    QPainterPath rightAngle;
    rightAngle.moveTo(bounds.center().x() - marker, baseY);
    rightAngle.lineTo(bounds.center().x() - marker, baseY + direction * marker);
    rightAngle.lineTo(bounds.center().x(), baseY + direction * marker);
    painter->drawPath(rightAngle);
    painter->restore();
}

void UBGraphicsAristo::paintPerpendicularScale(QPainter* painter,
                                                const QRectF& bounds,
                                                bool baseAtBottom)
{
    const qreal pixelsPerCentimeter =
        UBApplication::boardController->activeScene()->backgroundGridSize();
    const qreal pixelsPerMillimeter = pixelsPerCentimeter / 10.0;
    if (pixelsPerMillimeter <= 0.0)
        return;

    const qreal centerX = bounds.center().x();
    const qreal baseY = baseAtBottom ? bounds.bottom() : bounds.top();
    const qreal direction = baseAtBottom ? -1.0 : 1.0;
    const int maximumMillimeters = static_cast<int>(bounds.height() * 0.68
                                                     / pixelsPerMillimeter);
    painter->save();
    painter->setPen(drawColor());
    QFont smallFont = painter->font();
    smallFont.setPointSizeF(qMax(6.0, smallFont.pointSizeF() * 0.72));
    painter->setFont(smallFont);
    const QFontMetricsF metrics(smallFont);

    for (int millimeter = 10; millimeter <= maximumMillimeters; ++millimeter)
    {
        const qreal y = baseY + direction * millimeter * pixelsPerMillimeter;
        const bool centimeter = millimeter % 10 == 0;
        const bool halfCentimeter = millimeter % 5 == 0;
        const qreal halfTick = centimeter ? 7.0 : (halfCentimeter ? 5.0 : 3.0);
        painter->drawLine(QPointF(centerX - halfTick, y),
                          QPointF(centerX + halfTick, y));
        if (centimeter)
        {
            const QString label = QString::number(millimeter / 10);
            painter->drawText(QRectF(centerX + 10.0, y - metrics.height() / 2.0,
                                     metrics.horizontalAdvance(label) + 2.0,
                                     metrics.height()), Qt::AlignLeft | Qt::AlignVCenter,
                              label);
        }
    }
    painter->restore();
}

void UBGraphicsAristo::paintCentredRuler(QPainter* painter, const QRectF& bounds,
                                         bool baseAtBottom)
{
    sPixelsPerCentimeter = UBApplication::boardController->activeScene()->backgroundGridSize();
    const qreal pixelsPerMillimeter = sPixelsPerCentimeter / 10.0;
    if (pixelsPerMillimeter <= 0.0)
        return;

    const qreal centerX = bounds.center().x();
    const qreal baseY = baseAtBottom ? bounds.bottom() : bounds.top();
    const qreal direction = baseAtBottom ? -1.0 : 1.0;
    const int maximumMillimeters = static_cast<int>((bounds.width() / 2.0 - kRulerInset)
                                                     / pixelsPerMillimeter);
    const QFontMetricsF metrics(painter->font());
    const bool showEveryCentimeter = metrics.horizontalAdvance("00")
                                     <= sPixelsPerCentimeter - 5.0;

    for (int millimeter = 0; millimeter <= maximumMillimeters; ++millimeter)
    {
        qreal height = UBGeometryUtils::millimeterGraduationHeight;
        if (millimeter % 10 == 0)
            height = UBGeometryUtils::centimeterGraduationHeight;
        else if (millimeter % 5 == 0)
            height = UBGeometryUtils::halfCentimeterGraduationHeight;

        if (showEveryCentimeter || millimeter % 5 == 0)
        {
            for (int side : {-1, 1})
            {
                if (millimeter == 0 && side == 1)
                    continue;
                const qreal x = centerX + side * millimeter * pixelsPerMillimeter;
                painter->drawLine(QPointF(x, baseY), QPointF(x, baseY + direction * height));
            }
        }

        if (millimeter % 10 != 0 || (!showEveryCentimeter && millimeter % 50 != 0))
            continue;

        const QString label = QString::number(millimeter / 10);
        const qreal textWidth = metrics.horizontalAdvance(label);
        const qreal textHeight = metrics.height();
        for (int side : {-1, 1})
        {
            if (millimeter == 0 && side == 1)
                continue;
            const qreal x = centerX + side * millimeter * pixelsPerMillimeter;
            const qreal y = baseAtBottom
                ? baseY - UBGeometryUtils::centimeterGraduationHeight - textHeight - 3.0
                : baseY + UBGeometryUtils::centimeterGraduationHeight + 3.0;
            painter->drawText(QRectF(x - textWidth / 2.0, y, textWidth, textHeight),
                              Qt::AlignCenter, label);
        }
    }
}

void UBGraphicsAristo::paintProtractor(QPainter* painter, const QRectF& bounds,
                                       bool baseAtBottom)
{
    const QPointF origin = rotationCenter();
    const qreal radius = qMin(bounds.width() * kProtractorRadiusRatio,
                              bounds.height() * 0.72);
    const qreal verticalSign = baseAtBottom ? -1.0 : 1.0;
    QFont primaryFont = painter->font();
    QFont secondaryFont = primaryFont;
    secondaryFont.setPointSizeF(qMax(6.0, primaryFont.pointSizeF() * 0.82));

    // Semicircle and radial ticks are both generated from the same exact
    // degree value. This prevents the visual/label mismatch of bitmap scales.
    QPainterPath arc;
    arc.moveTo(origin.x() - radius, origin.y());
    for (int degree = 1; degree <= 180; ++degree)
    {
        const qreal radians = degree * kPi / 180.0;
        arc.lineTo(origin.x() - radius * std::cos(radians),
                   origin.y() + verticalSign * radius * std::sin(radians));
    }
    painter->drawPath(arc);

    QPainterPath innerArc;
    const qreal innerRadius = radius * kAngularGuideInnerRatio;
    innerArc.moveTo(origin.x() - innerRadius, origin.y());
    for (int degree = 1; degree <= 180; ++degree)
    {
        const qreal radians = degree * kPi / 180.0;
        innerArc.lineTo(origin.x() - innerRadius * std::cos(radians),
                        origin.y() + verticalSign * innerRadius * std::sin(radians));
    }
    painter->drawPath(innerArc);

    for (int degree = 0; degree <= 180; ++degree)
    {
        const qreal radians = degree * kPi / 180.0;
        qreal tickLength = 4.0;
        if (degree % 10 == 0)
            tickLength = 13.0;
        else if (degree % 5 == 0)
            tickLength = 8.0;

        const qreal cosine = std::cos(radians);
        const qreal sine = std::sin(radians);
        const QPointF outer(origin.x() - radius * cosine,
                            origin.y() + verticalSign * radius * sine);
        const QPointF inner(origin.x() - (radius - tickLength) * cosine,
                            origin.y() + verticalSign * (radius - tickLength) * sine);
        painter->drawLine(inner, outer);

        if (degree % 10 != 0)
            continue;

        // Double, counter-running scale, as on a physical protractor:
        // 0..180 is read from the left and 180..0 from the right.  Both
        // scales therefore show exactly 90 degrees on the centre line.
        const QString primaryLabel = QString::number(degree);
        const QString secondaryLabel = QString::number(180 - degree);
        const qreal primaryLabelRadius = radius - 25.0;
        const qreal secondaryLabelRadius = radius - 48.0;

        painter->setFont(primaryFont);
        QFontMetricsF primaryMetrics(primaryFont);
        const QPointF primaryCenter(origin.x() - primaryLabelRadius * cosine,
                                    origin.y() + verticalSign * primaryLabelRadius * sine);
        const QRectF primaryRect(primaryCenter.x() - primaryMetrics.horizontalAdvance(primaryLabel) / 2.0,
                                 primaryCenter.y() - primaryMetrics.height() / 2.0,
                                 primaryMetrics.horizontalAdvance(primaryLabel), primaryMetrics.height());
        painter->drawText(primaryRect, Qt::AlignCenter, primaryLabel);

        // At 90 degrees both scales have the same value, so one centred label
        // is clearer than two identical, vertically adjacent labels.
        if (degree != 90)
        {
            painter->setFont(secondaryFont);
            QFontMetricsF secondaryMetrics(secondaryFont);
            const QPointF secondaryCenter(origin.x() - secondaryLabelRadius * cosine,
                                          origin.y() + verticalSign * secondaryLabelRadius * sine);
            const QRectF secondaryRect(secondaryCenter.x() - secondaryMetrics.horizontalAdvance(secondaryLabel) / 2.0,
                                       secondaryCenter.y() - secondaryMetrics.height() / 2.0,
                                       secondaryMetrics.horizontalAdvance(secondaryLabel), secondaryMetrics.height());
            painter->drawText(secondaryRect, Qt::AlignCenter, secondaryLabel);
        }
    }

    painter->setFont(primaryFont);

    // Reference rays make the most frequently constructed angles immediately
    // visible without replacing the one-degree snapping available on the arc.
    painter->save();
    QPen referencePen(drawColor());
    referencePen.setWidthF(0.8);
    referencePen.setStyle(Qt::DotLine);
    painter->setPen(referencePen);
    for (int degree : {30, 45, 60, 90, 120, 135, 150})
    {
        const qreal radians = degree * kPi / 180.0;
        painter->drawLine(origin,
            QPointF(origin.x() - innerRadius * std::cos(radians),
                    origin.y() + verticalSign * innerRadius * std::sin(radians)));
    }
    painter->restore();
}
