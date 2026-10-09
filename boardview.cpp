#include "boardview.h"

#include <QPainter>

void BoardView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    if (!m_image.isNull()) {
        painter.drawImage((width() - m_image.width()) / 2, 0, m_image);
    }
}
