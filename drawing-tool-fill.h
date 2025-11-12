#ifndef DRAWING_TOOL_FILL_H
#define DRAWING_TOOL_FILL_H

#include "toolbase.h"
#include <QBrush>

class DrawingScene;
class DrawingView;
class DrawingShape;

/**
 * 填充工具
 * 点击封闭图形进行填充
 */
class DrawingToolFill : public ToolBase
{
    Q_OBJECT

public:
    explicit DrawingToolFill(QObject *parent = nullptr);
    
    void activate(DrawingScene *scene, DrawingView *view) override;
    void deactivate() override;
    
    bool mousePressEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos) override;

private slots:
    // 响应颜色变化
    void onFillColorChanged(const QColor &color);
    
private:
    // 检查点是否在封闭图形内
    DrawingShape* findEnclosedShape(const QPointF &scenePos);
    
    // 获取当前填充颜色
    QColor getCurrentFillColor() const;
    
    QColor m_currentFillColor;
};

#endif // DRAWING_TOOL_FILL_H