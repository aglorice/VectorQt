#include "drawing-group.h"
#include "drawing-shape.h"
#include "../ui/drawingscene.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

DrawingGroup::DrawingGroup(QGraphicsItem *parent)
    : DrawingShape(DrawingShape::Group, parent)
{
    // 🌟 设置标准标志，让 Qt 处理变换
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    // 🌟 关键：不设置 ItemHasNoContents，让变换正常传播
}

DrawingGroup::~DrawingGroup()
{
    // 清空列表，Qt 会自动管理子对象的生命周期
    m_items.clear();
}

void DrawingGroup::addItem(DrawingShape *item)
{
    if (!item) {
        return;
    }
    
    qDebug() << "Adding item to group - handling coordinate conversion properly";
    
    // 🌟 关键修复：正确处理坐标转换
    // 1. 保存对象在场景中的当前位置
    QPointF itemScenePos = item->scenePos();
    QTransform itemTransform = item->transform();
    
    // 2. 设置父子关系（Qt 会自动转换坐标）
    item->setParentItem(this);
    
    // 3. 手动设置正确的本地位置，避免位置跳跃
    // 将场景位置转换为相对于组的本地坐标
    QPointF itemLocalPos = this->mapFromScene(itemScenePos);
    item->setPos(itemLocalPos);
    
    // 4. 重置子对象的变换，避免二次变换
    item->setTransform(QTransform());
    
    m_items.append(item);
    
    // 让子对象不再响应独立的事件，由组统一处理
    item->setFlag(QGraphicsItem::ItemIsMovable, false);
    item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    
    // 更新几何
    prepareGeometryChange();
    update();
    
    qDebug() << "Item added - scene pos:" << itemScenePos << "local pos:" << itemLocalPos;
}

void DrawingGroup::removeItem(DrawingShape *item)
{
    if (!item || !m_items.contains(item)) {
        return;
    }
    
    qDebug() << "Removing item from group - restoring coordinates properly";
    
    // 🌟 关键修复：正确处理坐标恢复
    // 1. 保存对象当前的本地位置
    QPointF itemLocalPos = item->pos();
    
    // 2. 计算对象应该恢复到的场景位置
    QPointF itemScenePos = this->mapToScene(itemLocalPos);
    
    // 3. 解除父子关系
    item->setParentItem(nullptr);
    
    // 4. 恢复对象的场景位置
    item->setPos(itemScenePos);
    
    // 5. 恢复单位变换
    item->setTransform(QTransform());
    
    m_items.removeOne(item);
    
    // 恢复子对象的能力
    item->setFlag(QGraphicsItem::ItemIsMovable, true);
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    
    // 更新几何
    prepareGeometryChange();
    update();
    
    qDebug() << "Item removed - local pos:" << itemLocalPos << "scene pos:" << itemScenePos;
}

QList<DrawingShape*> DrawingGroup::ungroup()
{
    QList<DrawingShape*> result;
    
    // 🌟 批量解除父子关系，Qt 自动处理所有坐标转换
    for (DrawingShape *item : m_items) {
        if (item) {
            item->setParentItem(nullptr);
            item->setFlag(QGraphicsItem::ItemIsMovable, true);
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            result.append(item);
        }
    }
    
    // 清空列表
    m_items.clear();
    
    // 更新几何
    prepareGeometryChange();
    update();
    
    return result;
}

QRectF DrawingGroup::boundingRect() const
{
    // 🌟 使用 Qt 的标准方法，自动计算所有子对象的组合边界
    QRectF bounds = childrenBoundingRect();
    
    // 添加调试信息
    qDebug() << "Group boundingRect:" << bounds << "item count:" << m_items.size();
    
    if (bounds.isEmpty()) {
        // 如果没有子对象，返回最小边界
        if (m_items.isEmpty()) {
            return QRectF(0, 0, 1, 1);
        } else {
            // 如果有子对象但边界为空，手动计算
            QRectF manualBounds;
            for (DrawingShape *item : m_items) {
                if (item) {
                    QRectF itemBounds = item->boundingRect();
                    QRectF itemBoundsInGroup = item->mapRectToParent(itemBounds);
                    if (manualBounds.isEmpty()) {
                        manualBounds = itemBoundsInGroup;
                    } else {
                        manualBounds |= itemBoundsInGroup;
                    }
                }
            }
            qDebug() << "Manual boundingRect:" << manualBounds;
            return manualBounds;
        }
    }
    
    return bounds;
}

QRectF DrawingGroup::localBounds() const
{
    // 🌟 对于组对象，本地边界就是边界框
    return boundingRect();
}

void DrawingGroup::paintShape(QPainter *painter)
{
    // 🌟 组对象本身不需要绘制，只显示子对象
    Q_UNUSED(painter);
}

QPainterPath DrawingGroup::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void DrawingGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 🌟 简化的鼠标事件处理，让 Qt 处理标准交互
    if (event->button() == Qt::LeftButton) {
        setSelected(true);
    }
    
    QGraphicsItem::mousePressEvent(event);
}

void DrawingGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // 🌟 让 Qt 处理拖动，变换会自动传播到子对象
    QGraphicsItem::mouseMoveEvent(event);
}

void DrawingGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // 🌟 让 Qt 处理释放事件
    QGraphicsItem::mouseReleaseEvent(event);
}

QVariant DrawingGroup::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    // 🌟 简化的变化处理，Qt 自动处理变换传播
    if (change == ItemTransformHasChanged || change == ItemPositionHasChanged) {
        // 变换发生变化时，Qt 会自动更新所有子对象
        // 我们只需要通知视图更新
        prepareGeometryChange();
        update();
    }
    
    return QGraphicsItem::itemChange(change, value);
}