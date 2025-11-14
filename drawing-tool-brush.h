#ifndef DRAWING_TOOL_BRUSH_H
#define DRAWING_TOOL_BRUSH_H

#include "toolbase.h"
#include <QPointF>
#include <QVector>

class DrawingPath;
class QMouseEvent;

/**
 * 画笔工具 - 自由绘制
 */
class DrawingToolBrush : public ToolBase
{
    Q_OBJECT

public:
    explicit DrawingToolBrush(QObject *parent = nullptr);
    
    // 获取工具光标类型
    CursorManager::CursorType getCursorType() const override { return CursorManager::BrushCursor; }
    
    void activate(DrawingScene *scene, DrawingView *view) override;
    void deactivate() override;
    
    bool mousePressEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos) override;
    
    // 设置画笔粗细
    void setBrushWidth(qreal width) { m_brushWidth = width; }
    qreal brushWidth() const { return m_brushWidth; }
    
    // 设置平滑度
    void setSmoothness(qreal smoothness) { m_smoothness = smoothness; }
    qreal smoothness() const { return m_smoothness; }

private:
    // 平滑路径
    QVector<QPointF> smoothPath(const QVector<QPointF> &points);
    
    DrawingPath *m_currentPath;
    QVector<QPointF> m_points;
    QPointF m_lastPoint;
    qreal m_brushWidth;
    qreal m_smoothness;
    bool m_drawing;
};

#endif // DRAWING_TOOL_BRUSH_H