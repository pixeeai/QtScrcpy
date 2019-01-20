#include <QDebug>
#include <QMoveEvent>
#include <QStyle>

#include "magneticwidget.h"
#include "ui_magneticwidget.h"

MagneticWidget::MagneticWidget(QWidget* adsorbWidget, AdsorbPositions adsorbPos)
    : QWidget(Q_NULLPTR)
    , m_adsorbPos(adsorbPos)
    , m_adsorbWidget(adsorbWidget)
{
    Q_ASSERT(m_adsorbWidget);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(m_adsorbWidget, &QWidget::destroyed, this, &QWidget::close);
    m_adsorbWidget->installEventFilter(this);

}

MagneticWidget::~MagneticWidget()
{
    if (m_adsorbWidget) {
        m_adsorbWidget->removeEventFilter(this);
    }
}

bool MagneticWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_adsorbWidget || !event) {
        return false;
    }
    if (m_adsorbed && QEvent::Move == event->type()) {
        move(m_adsorbWidget->pos() - m_relativePos);
    }
    if (m_adsorbed && QEvent::Resize == event->type()) {
        QRect parentRect;
        QRect targetRect;
        getGeometry(parentRect, targetRect);
        QPoint pos(parentRect.left(), parentRect.top());
        switch (m_curAdsorbPosition) {
        case AP_OUTSIDE_LEFT:
            pos.setX(pos.x() - width());
            pos.setY(pos.y() - m_relativePos.y());
            break;
        case AP_OUTSIDE_RIGHT:
            pos.setX(pos.x() + m_adsorbWidget->width());
            pos.setY(pos.y() - m_relativePos.y());
            break;
        case AP_OUTSIDE_TOP:
            pos.setX(pos.x() - m_relativePos.x());
            pos.setY(pos.y() - targetRect.height());
            break;
        case AP_OUTSIDE_BOTTOM:
            pos.setX(pos.x() - m_relativePos.x());
            pos.setY(pos.y() + parentRect.height());
            break;
        case AP_INSIDE_LEFT:
            pos.setY(pos.y() - m_relativePos.y());
            break;
        case AP_INSIDE_RIGHT:
            pos.setX(parentRect.right() - targetRect.width());
            pos.setY(pos.y() - m_relativePos.y());
            break;
        case AP_INSIDE_TOP:
            pos.setX(pos.x() - m_relativePos.x());
            break;
        case AP_INSIDE_BOTTOM:
            pos.setX(pos.x() - m_relativePos.x());
            pos.setY(parentRect.bottom() - targetRect.height());
            break;
        default:
            break;
        }
        move(pos);
    }
    return false;
}

void MagneticWidget::moveEvent(QMoveEvent *event)
{
    if (!m_adsorbWidget) {
        return;
    }

    QRect parentRect;
    QRect targetRect;
    getGeometry(parentRect, targetRect);

    int parentLeft = parentRect.left();
    int parentRight = parentRect.right();
    int parentTop = parentRect.top();
    int parentBottom = parentRect.bottom();
    int targetLeft = targetRect.left();
    int targetRight = targetRect.right();
    int targetTop = targetRect.top();
    int targetBottom = targetRect.bottom();

    QPoint finalPosition = pos();
    int adsorbDistance = 30;

    m_adsorbed = false;

    if (m_adsorbPos & AP_INSIDE_LEFT
            && parentRect.intersects(targetRect)
            && qAbs(parentLeft - targetLeft) < adsorbDistance) {
        finalPosition.setX(parentLeft);
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_INSIDE_LEFT;
    }

    if (m_adsorbPos & AP_OUTSIDE_RIGHT
            && parentRect.intersects(targetRect.translated(-adsorbDistance, 0))
            && qAbs(parentRight - targetLeft) < adsorbDistance) {
        finalPosition.setX(parentRight);
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_OUTSIDE_RIGHT;
    }

    if (m_adsorbPos & AP_OUTSIDE_LEFT
            && parentRect.intersects(targetRect.translated(adsorbDistance, 0))
            && qAbs(parentLeft - targetRight) < adsorbDistance) {
        finalPosition.setX(parentLeft - targetRect.width());
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_OUTSIDE_LEFT;
    }

    if (m_adsorbPos & AP_INSIDE_RIGHT
            && parentRect.intersects(targetRect)
            && qAbs(parentRight - targetRight) < adsorbDistance) {
        finalPosition.setX(parentRight - targetRect.width());
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_INSIDE_RIGHT;
    }

    if (m_adsorbPos & AP_INSIDE_TOP
            && parentRect.intersects(targetRect)
            && qAbs(parentTop - targetTop) < adsorbDistance) {
        finalPosition.setY(parentTop);
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_INSIDE_TOP;
    }

    if (m_adsorbPos & AP_OUTSIDE_TOP
            && parentRect.intersects(targetRect.translated(0, adsorbDistance))
            && qAbs(parentTop - targetBottom) < adsorbDistance) {
        finalPosition.setY(parentTop - targetRect.height());
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_OUTSIDE_TOP;
    }

    if (m_adsorbPos & AP_OUTSIDE_BOTTOM
            && parentRect.intersects(targetRect.translated(0, -adsorbDistance))
            && qAbs(parentBottom - targetTop) < adsorbDistance) {
        finalPosition.setY(parentBottom);
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_OUTSIDE_BOTTOM;
    }

    if (m_adsorbPos & AP_INSIDE_BOTTOM
            && parentRect.intersects(targetRect)
            && qAbs(parentBottom - targetBottom) < adsorbDistance) {
        finalPosition.setY(parentBottom - targetRect.height());
        m_adsorbed |= true;
        m_curAdsorbPosition =  AP_INSIDE_BOTTOM;
    }

    if (m_adsorbed) {
        m_relativePos = m_adsorbWidget->pos() - pos();
    }

    move(finalPosition);
}

void MagneticWidget::getGeometry(QRect &relativeWidgetRect, QRect &targetWidgetRect)
{
    QRect parentRect = m_adsorbWidget->geometry();
    QRect targetRect = geometry();

    if (!m_adsorbWidget->windowFlags().testFlag(Qt::FramelessWindowHint)) {
        // title bar height
        int titleBarHeight = m_adsorbWidget->style()->pixelMetric(QStyle::PM_TitleBarHeight);
        parentRect.translate(0, -titleBarHeight);
        parentRect.setHeight(parentRect.height() + titleBarHeight);
    }

    if (!windowFlags().testFlag(Qt::FramelessWindowHint)) {
        // title bar height
        int titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight);
        targetRect.translate(0, -titleBarHeight);
        targetRect.setHeight(targetRect.height() + titleBarHeight);
    }
    relativeWidgetRect = parentRect;
    targetWidgetRect = targetRect;
}

