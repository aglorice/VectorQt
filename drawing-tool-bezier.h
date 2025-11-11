#ifndef DRAWING_TOOL_BEZIER_H
#define DRAWING_TOOL_BEZIER_H

#include "toolbase.h"
#include <QVector>
#include <QPointF>

class DrawingScene;
class QPainterPath;
class QGraphicsSceneMouseEvent;
class QStyleOptionGraphicsItem;
class QWidget;
class QPainter;
class DrawingView;
class DrawingPath;

/**
 * 贝塞尔曲线绘制工具
 * 支持通过点击添加控制点来创建贝塞尔曲线
 */
class DrawingBezierTool : public ToolBase
{
    Q_OBJECT

public:
    explicit DrawingBezierTool(QObject *parent = nullptr);
    ~DrawingBezierTool() override;

    // 事件处理 - 重写基类方法
    bool mousePressEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos) override;
    
    // 激活/停用
    void activate(DrawingScene *scene, DrawingView *view) override;
    void deactivate() override;

private:
    // 更新当前路径
    void updatePath();
    
    // 完成绘制
    void finishDrawing();
    
    // 状态变量
    QPainterPath *m_currentPath;  // 当前正在绘制的路径
    QVector<QPointF> m_controlPoints;  // 控制点列表
    bool m_isDrawing;  // 是否正在绘制
    DrawingPath *m_currentItem;  // 当前正在创建的路径
    DrawingPath *m_previewItem;  // 预览路径
};

#endif // DRAWING_TOOL_BEZIER_H