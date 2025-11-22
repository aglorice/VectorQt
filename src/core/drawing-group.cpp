#include "../core/drawing-group.h"
#include "../core/drawing-shape.h"

#include "../ui/drawingscene.h"
// #include "selection-layer.h" // 已移除 - 老的选择层系统
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QWidget>
#include <limits>

DrawingGroup::DrawingGroup(QGraphicsItem *parent)
    : DrawingShape(DrawingShape::Group, parent)
{
    // 设置标志，确保组合对象可以接收鼠标事件
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    // 🌟 移除ItemHasNoContents标志，避免阻止变换传播
    // setFlag(QGraphicsItem::ItemHasNoContents, true);
}

DrawingGroup::~DrawingGroup()
{
    // 先清空列表，避免在析构过程中访问
    m_items.clear();
    
    // QGraphicsItemGroup会自动清理子对象
    // 不需要手动删除子对象，它们由scene管理
}

void DrawingGroup::addItem(DrawingShape *item)
{
    if (!item) {
        return;
    }
    
    // 🌟 保存子项的初始变换（参考control-frame）
    m_initialTransforms[item] = item->transform();
    
    // 在设置父子关系之前，将子项的位置转换为相对于组的本地坐标
    // 获取子项在场景中的当前位置
    QPointF scenePos = item->scenePos();
    // 将场景位置转换为组的本地坐标
    QPointF localPos = this->mapFromScene(scenePos);
    // 设置子项在组内的本地位置
    item->setPos(localPos);
    
    // 🌟 设置父子关系，这是使组合对象能够移动的关键
    item->setParentItem(this);  // 设置父子关系
    
    // 🌟 关键修复：重置子项的变换，避免二次变换
    // 子项的位置已经转换为本地坐标，所以变换应该是单位矩阵
    item->setTransform(QTransform());
    
    // 保存到列表
    m_items.append(item);
    
    // 禁用子项的鼠标事件，让组合对象处理所有事件
    item->setFlag(QGraphicsItem::ItemIsMovable, false);
    item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    
   
    
    // 计算所有子项在组坐标系中的边界框
    QRectF combinedBounds;
    bool first = true;
    
    for (DrawingShape *item : m_items) {
        if (item) {
            // 获取子项在组坐标系中的边界框
            QRectF itemBounds = item->boundingRect();
            // 将子项的本地边界框转换到组的坐标系中
            QRectF itemBoundsInGroup = item->mapRectToParent(itemBounds);
            
            if (first) {
                combinedBounds = itemBoundsInGroup;
                first = false;
            } else {
                combinedBounds |= itemBoundsInGroup;
            }
        }
    }
    
    
    m_currentBounds = combinedBounds;
    // 更新几何
    prepareGeometryChange();
    update();
}

void DrawingGroup::removeItem(DrawingShape *item)
{
    if (!item || !m_items.contains(item)) {
        return;
    }
    
    // 🌟 解除父子关系前，恢复子项的原始变换
    if (m_initialTransforms.contains(item)) {
        item->setTransform(m_initialTransforms[item]);
        m_initialTransforms.remove(item);
    }
    
    // 🌟 解除父子关系
    item->setParentItem(nullptr);
    
    // 从列表移除
    m_items.removeOne(item);
    
    // 恢复子项的所有能力
    item->setFlag(QGraphicsItem::ItemIsMovable, true);
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    
    // 更新几何
    prepareGeometryChange();
    update();
}

QList<DrawingShape*> DrawingGroup::ungroup()
{
    QList<DrawingShape*> result;
    
    // 获取组合对象的场景位置
    QPointF groupScenePos = scenePos();
    
    // 移除所有子项
    for (DrawingShape *item : m_items) {
        if (item) {
            // 🌟 解除父子关系前，恢复子项的原始变换
            if (m_initialTransforms.contains(item)) {
                item->setTransform(m_initialTransforms[item]);
            }
            
            // 解除父子关系
            item->setParentItem(nullptr);
            
            // 恢复子项的所有能力
            item->setFlag(QGraphicsItem::ItemIsMovable, true);
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            
            // 保持子项的相对位置，而不是移动到组合位置
            // 子项的场景位置应该是组合位置加上它们在组合中的位置
            QPointF itemScenePos = mapToScene(item->pos());
            item->setPos(itemScenePos);
            
            result.append(item);
        }
    }
    
    // 清空列表和初始变换映射
    m_items.clear();
    m_initialTransforms.clear();
    
    return result;
}


QRectF DrawingGroup::localBounds() const
{
       
    return m_currentBounds;
}

void DrawingGroup::paintShape(QPainter *painter)
{
    // 不绘制任何内容，只显示子对象
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
    // qDebug() << "DrawingGroup::mousePressEvent called on" << this;
    
    // 左键自动选中
    if (event->button() == Qt::LeftButton) {
        setSelected(true);
    }
    
    // 🌟 调用QGraphicsItem的基类方法，确保拖动功能正常工作
    QGraphicsItem::mousePressEvent(event);
}

void DrawingGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // 调用QGraphicsItem的基类方法
    QGraphicsItem::mouseMoveEvent(event);
}

void DrawingGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // 调用QGraphicsItem的基类方法
    QGraphicsItem::mouseReleaseEvent(event);
}

void DrawingGroup::applyTransform(const QTransform &transform , const QPointF &anchor)
{
    // 🌟 简化变换逻辑，直接调用基类方法
    DrawingShape::applyTransform(transform,anchor);
    //QGraphicsItem::setTransform(transform);
}
QVariant DrawingGroup::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    // 🌟 在变换发生变化时，同步到所有子项
    if (change == ItemTransformHasChanged) {
        // 更新边界
        prepareGeometryChange();
        update();
    }
    
    // 位置变化也需要更新（虽然 Qt 应该自动处理）
    else if (change == ItemPositionChange || change == ItemPositionHasChanged) {
        // prepareGeometryChange();
        // update();
        
        // 老的手柄系统已移除，不再需要更新
        // if (editHandleManager()) {
        //     editHandleManager()->updateHandles();
        // }
    }
    
    // 老的手柄系统已移除，不再需要更新手柄显示
    else if (change == ItemSelectedHasChanged) {
        // if (editHandleManager()) {
        //     if (isSelected()) {
        //         editHandleManager()->showHandles();
        //     } else {
        //         editHandleManager()->hideHandles();
        //     }
        // }
    }
    
    return QGraphicsItem::itemChange(change, value);
}
