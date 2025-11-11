#ifndef DRAWING_TOOL_BASE_H
#define DRAWING_TOOL_BASE_H

#include <QObject>
#include <QPointF>
#include <QMouseEvent>


class DrawingScene;
class DrawingView;
class DrawingShape;

/**
 * 改进的工具基类 - 参考Inkscape的ToolBase设计
 * 负责创建和编辑形状，分离工具逻辑和对象逻辑
 */
class DrawingToolBase : public QObject
{
    Q_OBJECT

public:
    explicit DrawingToolBase(QObject *parent = nullptr);
    virtual ~DrawingToolBase() = default;
    
    // 工具生命周期
    virtual void activate(DrawingScene *scene, DrawingView *view);
    virtual void deactivate();
    
    // 鼠标事件处理 - 返回true表示事件被消费
    virtual bool mousePressEvent(QMouseEvent *event, const QPointF &scenePos);
    virtual bool mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos);
    virtual bool mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos);
    
    // 获取当前创建的形状
    DrawingShape* currentShape() const { return m_currentShape; }
    
    // 工具状态
    bool isActive() const { return m_active; }

protected:
    // 创建形状的工厂方法
    virtual DrawingShape* createShape(const QPointF &pos) = 0;
    
    // 更新形状几何
    virtual void updateShape(const QPointF &startPos, const QPointF &currentPos) = 0;
    
    // 完成形状创建
    virtual void finishShape();
    
    // 取消形状创建
    virtual void cancelShape();
    
    DrawingScene *m_scene;
    DrawingView *m_view;
    DrawingShape *m_currentShape;
    QPointF m_startPos;
    bool m_active;
    bool m_drawing;
};

/**
 * 矩形工具 - 改进版
 */
class RectangleTool : public DrawingToolBase
{
    Q_OBJECT

public:
    explicit RectangleTool(QObject *parent = nullptr);

protected:
    DrawingShape* createShape(const QPointF &pos) override;
    void updateShape(const QPointF &startPos, const QPointF &currentPos) override;
};

/**
 * 椭圆工具 - 改进版
 */
class EllipseTool : public DrawingToolBase
{
    Q_OBJECT

public:
    explicit EllipseTool(QObject *parent = nullptr);

protected:
    DrawingShape* createShape(const QPointF &pos) override;
    void updateShape(const QPointF &startPos, const QPointF &currentPos) override;
};

/**
 * 选择工具 - 改进版
 */
class SelectTool : public DrawingToolBase
{
    Q_OBJECT

public:
    explicit SelectTool(QObject *parent = nullptr);
    
    void activate(DrawingScene *scene, DrawingView *view) override;
    void deactivate() override;
    
    bool mousePressEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos) override;

protected:
    DrawingShape* createShape(const QPointF &pos) override;
    void updateShape(const QPointF &startPos, const QPointF &currentPos) override;
};

#endif // DRAWING_TOOL_BASE_H