#ifndef BOARDVIEW_H
#define BOARDVIEW_H

#include <QImage>
#include <QWidget>
#include <utility>

class BoardView : public QWidget
{
public:
    explicit BoardView(QWidget *parent = nullptr)
        : QWidget(parent)
    {}

    void setImage(QImage image)
    {
        m_image = std::move(image);
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_image;
};

#endif // BOARDVIEW_H
