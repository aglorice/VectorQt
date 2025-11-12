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
    , m_gridVisible(false)
    , m_gridAlignmentEnabled(true)
    , m_gridSize(20)
    , m_gridColor(QColor(200, 200, 200, 100))
    , m_snapEnabled(true)
    , m_snapTolerance(10)
    , m_objectSnapEnabled(true)
    , m_objectSnapTolerance(10)
    , m_snapIndicatorsVisible(true)
    , m_guidesEnabled(true)
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
    
    // 🌟 绘制参考线
    if (m_guidesEnabled && !m_guides.isEmpty()) {
        painter->setRenderHint(QPainter::Antialiasing, false);
        
        for (const Guide &guide : m_guides) {
            if (!guide.visible) continue;
            
            painter->setPen(QPen(guide.color, 1, Qt::SolidLine));
            
            if (guide.orientation == Qt::Vertical) {
                // 垂直参考线
                qreal lineX = guide.position;
                if (lineX >= rect.left() && lineX <= rect.right()) {
                    painter->drawLine(QPointF(lineX, rect.top()), QPointF(lineX, rect.bottom()));
                }
            } else {
                // 水平参考线
                qreal lineY = guide.position;
                if (lineY >= rect.top() && lineY <= rect.bottom()) {
                    painter->drawLine(QPointF(rect.left(), lineY), QPointF(rect.right(), lineY));
                }
            }
        }
        
        painter->setRenderHint(QPainter::Antialiasing, true);
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

// 🌟 智能吸附功能实现
DrawingScene::SnapResult DrawingScene::smartAlignToGrid(const QPointF &pos) const
{
    SnapResult result;
    result.snappedPos = pos;
    
    if (!m_snapEnabled || !m_gridAlignmentEnabled || !m_gridVisible) {
        return result;
    }
    
    const int tolerance = m_snapTolerance;
    const int gridSize = m_gridSize;
    
    // 计算最近的网格线
    int gridX = qRound(pos.x() / gridSize) * gridSize;
    int gridY = qRound(pos.y() / gridSize) * gridSize;
    
    // 检查X方向是否需要吸附
    if (qAbs(pos.x() - gridX) <= tolerance) {
        result.snappedPos.setX(gridX);
        result.snappedX = true;
    }
    
    // 检查Y方向是否需要吸附
    if (qAbs(pos.y() - gridY) <= tolerance) {
        result.snappedPos.setY(gridY);
        result.snappedY = true;
    }
    
    return result;
}

void DrawingScene::setSnapEnabled(bool enabled)
{
    m_snapEnabled = enabled;
}

bool DrawingScene::isSnapEnabled() const
{
    return m_snapEnabled;
}

void DrawingScene::setSnapTolerance(int tolerance)
{
    m_snapTolerance = qMax(1, tolerance);
}

int DrawingScene::snapTolerance() const
{
    return m_snapTolerance;
}

// 🌟 参考线系统实现
void DrawingScene::addGuide(Qt::Orientation orientation, qreal position)
{
    m_guides.append(Guide(orientation, position));
    update();
}

void DrawingScene::removeGuide(Qt::Orientation orientation, qreal position)
{
    for (int i = 0; i < m_guides.size(); ++i) {
        if (m_guides[i].orientation == orientation && qAbs(m_guides[i].position - position) < 1.0) {
            m_guides.removeAt(i);
            update();
            break;
        }
    }
}

void DrawingScene::clearGuides()
{
    m_guides.clear();
    update();
}

void DrawingScene::setGuideVisible(Qt::Orientation orientation, qreal position, bool visible)
{
    for (Guide &guide : m_guides) {
        if (guide.orientation == orientation && qAbs(guide.position - position) < 1.0) {
            guide.visible = visible;
            update();
            break;
        }
    }
}

DrawingScene::GuideSnapResult DrawingScene::snapToGuides(const QPointF &pos) const
{
    GuideSnapResult result;
    result.snappedPos = pos;
    
    if (!m_snapEnabled || m_guides.isEmpty()) {
        return result;
    }
    
    const int tolerance = m_snapTolerance;
    qreal minDistance = tolerance + 1;
    
    for (const Guide &guide : m_guides) {
        if (!guide.visible) continue;
        
        qreal distance;
        if (guide.orientation == Qt::Vertical) {
            distance = qAbs(pos.x() - guide.position);
            if (distance < minDistance) {
                minDistance = distance;
                result.snappedPos.setX(guide.position);
                result.snappedToGuide = true;
                result.snapOrientation = Qt::Vertical;
                result.guidePosition = guide.position;
            }
        } else {
            distance = qAbs(pos.y() - guide.position);
            if (distance < minDistance) {
                minDistance = distance;
                result.snappedPos.setY(guide.position);
                result.snappedToGuide = true;
                result.snapOrientation = Qt::Horizontal;
                result.guidePosition = guide.position;
            }
        }
    }
    
    return result;
}

// 🌟 对象吸附功能实现
DrawingScene::ObjectSnapResult DrawingScene::snapToObjects(const QPointF &pos, DrawingShape *excludeShape)
{
    ObjectSnapResult result;
    result.snappedPos = pos;
    
    if (!m_objectSnapEnabled) {
        return result;
    }
    
    const int tolerance = m_objectSnapTolerance;
    qreal minDistance = tolerance + 1;
    
    QList<ObjectSnapPoint> snapPoints = getObjectSnapPoints(excludeShape);
    
    for (const ObjectSnapPoint &snapPoint : snapPoints) {
        qreal distance = QLineF(pos, snapPoint.position).length();
        if (distance < minDistance) {
            minDistance = distance;
            result.snappedPos = snapPoint.position;
            result.snappedToObject = true;
            result.snapType = snapPoint.type;
            result.targetShape = snapPoint.shape;
            
            // 设置描述
            switch (snapPoint.type) {
                case SnapToLeft: result.snapDescription = "吸附到左边"; break;
                case SnapToRight: result.snapDescription = "吸附到右边"; break;
                case SnapToTop: result.snapDescription = "吸附到上边"; break;
                case SnapToBottom: result.snapDescription = "吸附到下边"; break;
                case SnapToCenterX: result.snapDescription = "吸附到水平中心"; break;
                case SnapToCenterY: result.snapDescription = "吸附到垂直中心"; break;
                case SnapToCorner: result.snapDescription = "吸附到角点"; break;
            }
        }
    }
    
    return result;
}

QList<DrawingScene::ObjectSnapPoint> DrawingScene::getObjectSnapPoints(DrawingShape *excludeShape) const
{
    QList<ObjectSnapPoint> points;
    
    for (QGraphicsItem *item : items()) {
        DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(item);
        if (!shape || shape == excludeShape || !shape->isVisible()) {
            continue;
        }
        
        QRectF bounds = shape->boundingRect();
        QPointF center = bounds.center();
        
        // 添加关键吸附点
        points.append(ObjectSnapPoint(bounds.topLeft(), SnapToCorner, shape));
        points.append(ObjectSnapPoint(bounds.topRight(), SnapToCorner, shape));
        points.append(ObjectSnapPoint(bounds.bottomLeft(), SnapToCorner, shape));
        points.append(ObjectSnapPoint(bounds.bottomRight(), SnapToCorner, shape));
        points.append(ObjectSnapPoint(center, SnapToCenterX, shape));
        points.append(ObjectSnapPoint(QPointF(bounds.left(), center.y()), SnapToLeft, shape));
        points.append(ObjectSnapPoint(QPointF(bounds.right(), center.y()), SnapToRight, shape));
        points.append(ObjectSnapPoint(QPointF(center.x(), bounds.top()), SnapToTop, shape));
        points.append(ObjectSnapPoint(QPointF(center.x(), bounds.bottom()), SnapToBottom, shape));
    }
    
    return points;
}

void DrawingScene::setObjectSnapEnabled(bool enabled)
{
    m_objectSnapEnabled = enabled;
}

bool DrawingScene::isObjectSnapEnabled() const
{
    return m_objectSnapEnabled;
}

void DrawingScene::setObjectSnapTolerance(int tolerance)
{
    m_objectSnapTolerance = qMax(1, tolerance);
}

int DrawingScene::objectSnapTolerance() const
{
    return m_objectSnapTolerance;
}

void DrawingScene::showSnapIndicators(const ObjectSnapResult &snapResult)
{
    Q_UNUSED(snapResult)
    // TODO: 实现吸附指示器的视觉显示
}

void DrawingScene::clearSnapIndicators()
{
    // TODO: 清除吸附指示器
}

void DrawingScene::clearExpiredSnapIndicators(const QPointF &currentPos)
{
    Q_UNUSED(currentPos)
    // TODO: 清除过期的吸附指示器
}

void DrawingScene::setSnapIndicatorsVisible(bool visible)
{
    m_snapIndicatorsVisible = visible;
}

bool DrawingScene::areSnapIndicatorsVisible() const
{
    return m_snapIndicatorsVisible;
}