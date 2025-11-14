#ifndef DRAWING_TOOL_NODE_EDIT_H
#define DRAWING_TOOL_NODE_EDIT_H

#include "toolbase.h"
#include <QPointF>
#include <QGraphicsItem>

class DrawingScene;
class DrawingView;
class DrawingShape;
class EditHandle;

/**
 * 节点编辑工具 - 用于编辑图形的内部属性
 * 参考Inkscape的节点工具设计
 */
class DrawingNodeEditTool : public ToolBase
{
    Q_OBJECT

public:
    explicit DrawingNodeEditTool(QObject *parent = nullptr);
    ~DrawingNodeEditTool() override;

    // 事件处理 - 重写基类方法
    bool mousePressEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos) override;
    bool mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos) override;
    
    // 获取工具光标类型
    CursorManager::CursorType getCursorType() const override { return CursorManager::NodeEditCursor; }
    
    // 激活/停用
    void activate(DrawingScene *scene, DrawingView *view) override;
    void deactivate() override;

private:
    // 内部方法
    void updateNodeHandles();
    void updateOtherNodeHandles(int draggedIndex, const QPointF &draggedPos);  // 更新除拖动手柄外的其他手柄
    void clearNodeHandles();
    void onSceneSelectionChanged(); // 处理场景选择变化
    
    // 状态变量
    DrawingShape *m_selectedShape;  // 当前选中的形状
    EditHandle *m_activeHandle;     // 当前激活的编辑手柄
    bool m_dragging;                // 是否正在拖动
    QPointF m_dragStartPos;         // 拖动起始位置
    QPointF m_originalValue;        // 原始值（用于撤销）
    QList<EditHandle*> m_nodeHandles; // 节点手柄列表
};

#endif // DRAWING_TOOL_NODE_EDIT_H