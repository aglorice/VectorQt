#include <QGraphicsItem>
#include "drawingscene.h"
#include "drawing-shape.h"
#include "drawing-group.h"
#include "selection-layer.h"
#include "drawing-edit-handles.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QUndoCommand>
#include <QPainter>

class AddItemCommand : public QUndoCommand
{
public:
    AddItemCommand(DrawingScene *scene, QGraphicsItem *item, QUndoCommand *parent = nullptr)
        : QUndoCommand("添加项目", parent), m_scene(scene), m_item(item) {}
    
    void undo() override {
        m_scene->removeItem(m_item);
        m_item->setVisible(false);
    }
    
    void redo() override {
        m_scene->addItem(m_item);
        m_item->setVisible(true);
    }
    
private:
    DrawingScene *m_scene;
    QGraphicsItem *m_item;
};

class RemoveItemCommand : public QUndoCommand
{
public:
    RemoveItemCommand(DrawingScene *scene, QGraphicsItem *item, QUndoCommand *parent = nullptr)
        : QUndoCommand("删除项目", parent), m_scene(scene), m_item(item) {}
    
    void undo() override {
        m_scene->addItem(m_item);
        m_item->setVisible(true);
    }
    
    void redo() override {
        m_scene->removeItem(m_item);
        m_item->setVisible(false);
    }
    
    ~RemoveItemCommand() override {
        // QGraphicsScene会自动管理item的生命周期，不需要手动删除
    }
    
private:
    DrawingScene *m_scene;
    QGraphicsItem *m_item;
};

DrawingScene::DrawingScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_isModified(false)
    , m_selectionLayer(nullptr)
    , m_gridVisible(true)
    , m_gridAlignmentEnabled(false)  // 默认关闭网格对齐
    , m_gridSize(20)
    , m_gridColor(QColor(200, 200, 200, 150))  // 更改网格颜色为浅灰色，适应亮色背景
{
    // 不在这里创建选择层，只在选择工具激活时创建
    // 暂时不连接选择变化信号，避免在初始化时触发
    // connect(this, &DrawingScene::selectionChanged, this, &DrawingScene::onSelectionChanged);
}

void DrawingScene::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit sceneModified(modified);
    }
}

void DrawingScene::clearScene()
{
    // 先清除所有选择
    clearSelection();
    
    // QGraphicsScene会自动管理item的生命周期，只需要移除它们
    QList<QGraphicsItem*> items = this->items();
    foreach (QGraphicsItem *item, items) {
        if (item) {
            removeItem(item);
            // 不需要手动删除，scene会自动处理
        }
    }
    
    m_undoStack.clear();
    setModified(false);
}

void DrawingScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 检查是否点击了空白区域
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
    bool clickedOnEmpty = (item == nullptr);
    
    // 如果点击空白区域且有选择，清除所有选择
    if (clickedOnEmpty) {
        qDebug() << "Clicked on empty area, clearing selection";
        clearSelection();
    }
    
    QGraphicsScene::mousePressEvent(event);
}

void DrawingScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
}

void DrawingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
}

void DrawingScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        QList<QGraphicsItem*> selected = selectedItems();
        if (!selected.isEmpty()) {
            // 先清除选择，避免在删除过程中出现问题
            clearSelection();
            
            foreach (QGraphicsItem *item, selected) {
                if (item) {
                    m_undoStack.push(new RemoveItemCommand(this, item));
                }
            }
            setModified(true);
        }
        event->accept();
    } else {
        QGraphicsScene::keyPressEvent(event);
    }
}

void DrawingScene::updateSelection()
{
    // 使用Qt的信号阻塞机制来避免递归调用
    bool wasBlocked = blockSignals(true);
    
    QList<QGraphicsItem*> selected = selectedItems();
    QList<DrawingShape*> selectedShapes;
    
    qDebug() << "updateSelection called, total selected items:" << selected.count();
    
    // 只收集选中的DrawingShape对象，忽略QGraphicsItemGroup和DrawingLayer
    for (QGraphicsItem *item : selected) {
        if (!item) continue; // 空指针检查
        
        // 检查是否是DrawingLayer，如果是则跳过
        if (item->type() == QGraphicsItem::UserType + 100) {
            qDebug() << "Skipping DrawingLayer item in selection";
            continue;
        }
        
        
        
        DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(item);
        if (shape) {
            // 额外检查对象是否有效且在场景中
            if (shape->scene() == this) {
                selectedShapes.append(shape);
                qDebug() << "Found selected shape:" << shape;
            }
        }
        // QGraphicsItemGroup和DrawingLayer不需要特殊的编辑手柄，所以不处理
    }
    
    qDebug() << "Total DrawingShape objects selected:" << selectedShapes.count();
    
    // 禁用所有未选中图形的编辑把手
    QList<QGraphicsItem*> allItems = items();
    for (QGraphicsItem *item : allItems) {
        if (!item) continue; // 空指针检查
        
        // 跳过DrawingLayer
        if (item->type() == QGraphicsItem::UserType + 100) {
            continue;
        }
        
        
        
        DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(item);
        if (shape && !selectedShapes.contains(shape)) {
            // 额外检查对象是否有效
            if (shape->scene() == this) {
                shape->setEditHandlesEnabled(false);
            }
        }
    }
    
    // 启用选中图形的编辑把手
    for (DrawingShape *shape : selectedShapes) {
        if (shape && shape->scene() == this) {  // 确保形状仍然在场景中
            shape->setEditHandlesEnabled(true);
        }
    }
    
    // 恢复信号状态
    blockSignals(wasBlocked);
}

void DrawingScene::activateSelectionTool()
{
    qDebug() << "activateSelectionTool called";
    // 只在选择工具激活时连接选择变化信号
    if (!signalsBlocked()) {
        // 断开已存在的连接（如果有的话）
        disconnect(this, &DrawingScene::selectionChanged, this, &DrawingScene::onSelectionChanged);
        
        qDebug() << "Connecting selectionChanged signal";
        connect(this, &DrawingScene::selectionChanged, this, &DrawingScene::onSelectionChanged);
        
        // 立即更新一次选择状态
        if (this->selectionLayer()) {
            this->selectionLayer()->updateSelectionBounds();
        }
    }
}

void DrawingScene::deactivateSelectionTool()
{
    // 断开选择变化信号
    disconnect(this, &DrawingScene::selectionChanged, this, &DrawingScene::onSelectionChanged);
}

void DrawingScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // 设置亮色背景
    painter->fillRect(rect, QColor(255, 255, 255)); // 白色背景
    
    // 绘制网格，但限制在场景矩形范围内
    if (m_gridVisible) {
        QRectF sceneRect = QGraphicsScene::sceneRect();
        QRectF limitedRect = rect.intersected(sceneRect);
        if (!limitedRect.isEmpty()) {
            drawGrid(painter, limitedRect);
        }
    }
}

void DrawingScene::drawGrid(QPainter *painter, const QRectF &rect)
{
    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    
    // 网格以场景坐标(0,0)为原点，与标尺对齐
    // 计算网格起始位置（从0开始，对齐到网格大小）
    int startX = qFloor(rect.left() / m_gridSize) * m_gridSize;
    int startY = qFloor(rect.top() / m_gridSize) * m_gridSize;
    int endX = qCeil(rect.right() / m_gridSize) * m_gridSize;
    int endY = qCeil(rect.bottom() / m_gridSize) * m_gridSize;
    
    // 确保包含0,0点
    startX = qMin(startX, 0);
    startY = qMin(startY, 0);
    
    // 绘制垂直线
    for (int x = startX; x <= endX; x += m_gridSize) {
        // 加粗原点线
        if (x == 0) {
            painter->setPen(QPen(m_gridColor.darker(150), 1, Qt::SolidLine));
        } else {
            painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
        }
        painter->drawLine(QPointF(x, startY), QPointF(x, endY));
    }
    
    // 绘制水平线
    for (int y = startY; y <= endY; y += m_gridSize) {
        // 加粗原点线
        if (y == 0) {
            painter->setPen(QPen(m_gridColor.darker(150), 1, Qt::SolidLine));
        } else {
            painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
        }
        painter->drawLine(QPointF(startX, y), QPointF(endX, y));
    }
}

// 网格功能实现
void DrawingScene::setGridVisible(bool visible)
{
    if (m_gridVisible != visible) {
        m_gridVisible = visible;
        update(); // 触发重绘
    }
}

bool DrawingScene::isGridVisible() const
{
    return m_gridVisible;
}

void DrawingScene::setGridSize(int size)
{
    if (m_gridSize != size && size > 0) {
        m_gridSize = size;
        update(); // 触发重绘
    }
}

int DrawingScene::gridSize() const
{
    return m_gridSize;
}

void DrawingScene::setGridColor(const QColor &color)
{
    if (m_gridColor != color) {
        m_gridColor = color;
        update(); // 触发重绘
    }
}

QColor DrawingScene::gridColor() const
{
    return m_gridColor;
}

QPointF DrawingScene::alignToGrid(const QPointF &pos) const
{
    if (!m_gridVisible || !m_gridAlignmentEnabled) {
        return pos; // 如果网格不可见或对齐未启用，则返回原始位置
    }
    
    qreal x = qRound(pos.x() / m_gridSize) * m_gridSize;
    qreal y = qRound(pos.y() / m_gridSize) * m_gridSize;
    
    return QPointF(x, y);
}

QRectF DrawingScene::alignToGrid(const QRectF &rect) const
{
    QPointF topLeft = alignToGrid(rect.topLeft());
    QPointF bottomRight = alignToGrid(rect.bottomRight());
    
    return QRectF(topLeft, bottomRight).normalized();
}

void DrawingScene::setGridAlignmentEnabled(bool enabled)
{
    m_gridAlignmentEnabled = enabled;
}

bool DrawingScene::isGridAlignmentEnabled() const
{
    return m_gridAlignmentEnabled;
}

void DrawingScene::onSelectionChanged()
{
    qDebug() << "onSelectionChanged called";
    // 直接更新选择
    updateSelection();
}