#ifndef DRAWING_EDIT_HANDLES_H
#define DRAWING_EDIT_HANDLES_H

#include <QGraphicsItem>
#include <QObject>
#include <QRectF>
#include <QTransform>
#include <QList>

class DrawingShape;
class SelectionLayer;
class DrawingScene;
#include "drawing-transform.h"
#include "selection-layer.h"

/**
 * 编辑把手 - 用于调整图形的大小和形状
 * 基于Inkscape的手柄系统设计
 */
class EditHandle : public QGraphicsItem
{
public:
    // 手柄类型（参考Inkscape的HANDLE_*枚举）
    enum HandleType {
        TopLeft = SelectionLayer::TopLeft,
        TopCenter = SelectionLayer::Top,
        TopRight = SelectionLayer::TopRight,
        CenterLeft = SelectionLayer::Left,
        CenterRight = SelectionLayer::Right,
        BottomLeft = SelectionLayer::BottomLeft,
        BottomCenter = SelectionLayer::Bottom,
        BottomRight = SelectionLayer::BottomRight,
        Rotation = SelectionLayer::Rotate,
        CornerRadius = 10,  // 用于矩形圆角控制的圆形手柄
        SizeControl = 11,   // 用于尺寸控制的方形手柄
        ArcControl = 12,    // 用于椭圆弧度控制的圆形手柄
        Custom = 13         // 自定义手柄
    };
    
    public:
    explicit EditHandle(HandleType type, DrawingShape *parent);
    explicit EditHandle(HandleType type, SelectionLayer *parent);
    explicit EditHandle(HandleType type, QGraphicsItem *parent);
    
    // 设置全局SelectionLayer引用
    static void setGlobalSelectionLayer(SelectionLayer *selectionLayer) {
        s_globalSelectionLayer = selectionLayer;
    }
    ~EditHandle();
    
    // 基本属性
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    // 交互
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    
    // 变换处理
    void startDrag(const QPointF &scenePos);
    void updateDrag(const QPointF &scenePos);
    void endDrag();
    
    // 属性
    HandleType handleType() const { return m_type; }
    void setHighlighted(bool highlighted) { m_highlighted = highlighted; update(); }
    bool isHighlighted() const { return m_highlighted; }
    
    // 位置和大小
    void setPos(const QPointF &pos);
    void setRect(const QRectF &rect);
    QPointF pos() const { return m_rect.center(); }
    QRectF rect() const { return m_rect; }

private:
    // 拖动处理
    void updateDragForShape(const QPointF &scenePos);
    void updateDragForSelectionLayer(const QPointF &scenePos);
    
    // 辅助函数：获取手柄在本地坐标中的位置
    QPointF getHandlePositionInLocalCoords(HandleType type, const QRectF &bounds) const;
    
    HandleType m_type;
    DrawingShape *m_shape;
    QRectF m_rect;
    bool m_highlighted;
    static constexpr QSize s_handleSize = QSize(8, 8);
    
    // 拖动状态
    bool m_dragging = false;
    QPointF m_dragStartPos;
    QRectF m_originalBounds;
    DrawingTransform m_originalTransform;
    double m_originalRotation = 0.0;  // 原始旋转角度
    double m_accumulatedRotation = 0.0;  // 累积旋转角度
    QPointF m_fixedAnchor;  // 固定的锚点位置（场景坐标）
    
    // 全局SelectionLayer引用
    static SelectionLayer *s_globalSelectionLayer;
    
    // 检测手柄是否与对象相交
    bool intersectsAnyObject() const;
};

// EditHandleManager
class EditHandleManager : public QObject
{
    Q_OBJECT
    
public:
    EditHandleManager(DrawingShape *shape, QObject *parent = nullptr);
    EditHandleManager(SelectionLayer *selectionLayer, QObject *parent = nullptr);
    ~EditHandleManager();
    
    // 创建和更新把手
    void createHandles();
    void updateHandlePositions();
    void showHandles();
    void hideHandles();
    void updateHandles();
    
    // 获取特定类型的把手
    EditHandle* handleAt(EditHandle::HandleType type) const;
    
    // 获取所有把手
    QList<EditHandle*> handles() const { return m_handles; }
    
    // 清空把手列表（不删除把手）
    void clearHandles() { m_handles.clear(); }
    
    // 设置活动把手
    void setActiveHandle(EditHandle *handle);
    EditHandle* activeHandle() const { return m_activeHandle; }
    
private:
    DrawingShape *m_shape;
    SelectionLayer *m_selectionLayer;
    QList<EditHandle*> m_handles;
    EditHandle *m_activeHandle;
    bool m_dragging;
};

/**
 * 选择指示器 - 显示选中和编辑状态
 */
class SelectionIndicator : public QGraphicsItem
{
public:
    explicit SelectionIndicator(QGraphicsItem *parent = nullptr);
    
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    void updateIndicator(const QRectF &bounds, const QTransform &transform);

private:
    QRectF m_bounds;
    QTransform m_transform;
};

#endif // DRAWING_EDIT_HANDLES_H