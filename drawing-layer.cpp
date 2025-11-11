#include "drawing-layer.h"
#include "drawing-shape.h"
#include <QDebug>

DrawingLayer::DrawingLayer(const QString &name, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_name(name)
    , m_visible(true)
    , m_opacity(1.0)
    , m_locked(false)
    , m_boundingRectDirty(true)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

DrawingLayer::~DrawingLayer()
{
    // 清理所有图形，将它们从图层中移除但不删除
    // 因为场景会管理它们的生命周期
    for (DrawingShape *shape : m_shapes) {
        if (shape) {
            // 禁用编辑把手，防止访问已删除的图层
            shape->setEditHandlesEnabled(false);
            shape->setParentItem(nullptr);
            
            // 如果图形被选中，取消选中
            if (shape->isSelected()) {
                shape->setSelected(false);
            }
        }
    }
    m_shapes.clear();
}

void DrawingLayer::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        setVisible(visible); // 调用基类方法
        update();
    }
}

void DrawingLayer::setOpacity(qreal opacity)
{
    if (m_opacity != opacity) {
        m_opacity = qBound(0.0, opacity, 1.0);
        update();
    }
}

void DrawingLayer::addShape(DrawingShape *shape)
{
    if (shape && !m_shapes.contains(shape)) {
        m_shapes.append(shape);
        shape->setParentItem(this); // 将图形设置为图层的子项
        m_boundingRectDirty = true;
        update();
    }
}

void DrawingLayer::removeShape(DrawingShape *shape)
{
    if (m_shapes.removeOne(shape)) {
        shape->setParentItem(nullptr); // 断开父子关系
        m_boundingRectDirty = true;
        update();
    }
}

QRectF DrawingLayer::boundingRect() const
{
    if (m_boundingRectDirty) {
        updateBoundingRect();
    }
    return m_boundingRect;
}

void DrawingLayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    if (!m_visible || qFuzzyIsNull(m_opacity)) {
        return;
    }
    
    // 保存 painter 状态
    painter->save();
    
    // 应用图层透明度
    if (m_opacity < 1.0) {
        painter->setOpacity(m_opacity);
    }
    
    // 应用图层变换
    if (!m_layerTransform.isIdentity()) {
        painter->setTransform(m_layerTransform, true);
    }
    
    // 绘制所有子图形
    for (DrawingShape *shape : m_shapes) {
        if (shape && shape->isVisible()) {
            // 子图形会自动绘制，因为它们是图层的子项
        }
    }
    
    // 恢复 painter 状态
    painter->restore();
}

void DrawingLayer::setLayerTransform(const QTransform &transform)
{
    if (m_layerTransform != transform) {
        prepareGeometryChange();
        m_layerTransform = transform;
        m_boundingRectDirty = true;
        update();
    }
}

void DrawingLayer::parseFromSvg(const QDomElement &element)
{
    // 解析图层属性
    m_name = element.attribute("id", m_name);
    
    // 解析透明度
    QString opacityStr = element.attribute("opacity");
    if (!opacityStr.isEmpty()) {
        m_opacity = opacityStr.toDouble();
    }
    
    // 解析可见性
    QString visibilityStr = element.attribute("visibility");
    if (!visibilityStr.isEmpty()) {
        m_visible = (visibilityStr != "hidden");
    }
    
    // 解析变换
    QString transformStr = element.attribute("transform");
    if (!transformStr.isEmpty()) {
        // TODO: 解析SVG变换字符串
    }
    
    qDebug() << "解析图层:" << m_name << "透明度:" << m_opacity << "可见:" << m_visible;
}

QDomElement DrawingLayer::exportToSvg(QDomDocument &doc) const
{
    QDomElement gElement = doc.createElement("g");
    
    // 设置图层属性
    if (!m_name.isEmpty()) {
        gElement.setAttribute("id", m_name);
    }
    
    if (m_opacity < 1.0) {
        gElement.setAttribute("opacity", QString::number(m_opacity));
    }
    
    if (!m_visible) {
        gElement.setAttribute("visibility", "hidden");
    }
    
    // 导出所有图形
    for (DrawingShape *shape : m_shapes) {
        if (shape) {
            // TODO: 将图形导出为SVG元素并添加到gElement中
        }
    }
    
    return gElement;
}

void DrawingLayer::updateBoundingRect() const
{
    m_boundingRect = QRectF();
    
    for (DrawingShape *shape : m_shapes) {
        if (shape && shape->isVisible()) {
            QRectF shapeBounds = shape->boundingRect();
            // DrawingShape的transform()返回DrawingTransform对象，不是指针
            DrawingTransform shapeTransform = shape->transform();
            if (!shapeTransform.transform().isIdentity()) {
                shapeBounds = shapeTransform.transformedBounds(shapeBounds);
            }
            
            if (m_boundingRect.isEmpty()) {
                m_boundingRect = shapeBounds;
            } else {
                m_boundingRect = m_boundingRect.united(shapeBounds);
            }
        }
    }
    
    // 应用图层变换到边界框
    if (!m_layerTransform.isIdentity() && !m_boundingRect.isEmpty()) {
        m_boundingRect = m_layerTransform.mapRect(m_boundingRect);
    }
    
    m_boundingRectDirty = false;
}