#include "selection-layer.h"
#include "drawing-shape.h"
#include "drawing-transform.h"
#include "drawing-edit-handles.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

SelectionLayer::SelectionLayer(QObject *parent)
    : QObject(parent)
    , m_mouseDown(false)
    , m_dragHandle(SelectionLayer::None)
    , m_handleManager(nullptr)
    , m_grabbed(false)
    , m_transformState(STATE_NONE)
{
    // 不再是QGraphicsItem，移除所有标志设置
    
    // 初始化变换矩阵
    m_currentRelativeAffine = QTransform();
    m_accumulatedTransform = QTransform();
    m_transformCenter = QPointF(0, 0);
    
    // 初始化预览模式
    m_previewMode = SHOW_CONTENT;
    m_showPreview = false;
    
    // 创建定时器用于定期更新选择边界
    // 不再使用定时器，改为事件驱动更新
    // m_updateTimer = new QTimer(this);
    // connect(m_updateTimer, &QTimer::timeout, this, &SelectionLayer::updateSelectionPeriodically);
    // m_updateTimer->start(50); // 每50ms更新一次
    
    // 不再是QGraphicsItem，不需要设置可见性
}

SelectionLayer::~SelectionLayer()
{
    // 清空图形列表，但不改变父项关系
    // Qt会自动管理对象生命周期
    m_selectedShapes.clear();
    // 不删除m_handleManager，因为它由父对象自动管理
    m_handleManager = nullptr;
}

void SelectionLayer::addShape(DrawingShape *shape)
{
    if (!shape || m_selectedShapes.contains(shape)) {
        return;
    }
    
    // 简化逻辑：只添加到列表，不改变父项关系
    m_selectedShapes.append(shape);
    shape->setSelected(true);
    
    // QGraphicsItem不能使用信号，改为事件驱动更新
    
    // 更新边界框
    updateSelectionBounds();
    
    // 显示编辑把手
    if (m_handleManager) {
        m_handleManager->showHandles();
    }
}

void SelectionLayer::removeShape(DrawingShape *shape)
{
    if (!shape || !m_selectedShapes.contains(shape)) {
        return;
    }
    
    // QGraphicsItem不能使用信号，不需要断开连接
    
    m_selectedShapes.removeOne(shape);
    shape->setSelected(false);
    
    // 更新选择边界
    updateSelectionBounds();
}

void SelectionLayer::clearShapes()
{
    for (DrawingShape *shape : m_selectedShapes) {
        if (shape) {
            shape->setSelected(false);
            // 不改变父项关系，让Qt自动管理
        }
    }
    m_selectedShapes.clear();
    
    // 更新边界框
    updateSelectionBounds();
    
    // 不再是QGraphicsItem，不需要设置可见性
}

QRectF SelectionLayer::selectionBounds() const
{
    return m_selectionBounds;
}

void SelectionLayer::translate(const QPointF &delta)
{
    if (delta.isNull()) return;
    
    // 简单的平移逻辑
    for (DrawingShape *shape : m_selectedShapes) {
        if (shape) {
            shape->setPos(shape->pos() + delta);
        }
    }
    
    // 更新选择边界
    updateSelectionBounds();
    updateHandles();
}

void SelectionLayer::applyTransform(const QTransform &relAffine, const QPointF &norm)
{
    if (m_selectedShapes.isEmpty()) {
        return;
    }
    
    // 计算最终变换矩阵：Translate(-norm) * relAffine * Translate(norm)
    // 这是Inkscape中实现相对于特定点变换的标准方法
    QTransform finalTransform = QTransform::fromTranslate(-norm.x(), -norm.y()) * relAffine * QTransform::fromTranslate(norm.x(), norm.y());
    
    // 更新累积变换
    m_accumulatedTransform = finalTransform * m_accumulatedTransform;
    m_currentRelativeAffine = finalTransform;
    
    // 根据预览模式应用变换
    if (m_previewMode == SHOW_CONTENT) {
        // 内容预览模式：直接应用变换到图形
        for (DrawingShape *shape : m_selectedShapes) {
            if (!shape) continue;
            
            // 获取图形的当前变换
            DrawingTransform shapeTransform = shape->transform();
            QTransform currentTransform = shapeTransform.transform();
            
            // 应用新的变换：prev_transform * affine
            QTransform newTransform = currentTransform * finalTransform;
            shapeTransform.setTransform(newTransform);
            shape->setTransform(shapeTransform);
        }
    } else if (m_previewMode == SHOW_OUTLINE) {
        // 轮廓预览模式：只更新预览轮廓，不改变实际图形
        updatePreview();
    }
    
    // 更新选择边界和手柄
    updateSelectionBounds();
    updateHandles();
}

void SelectionLayer::grabTransform()
{
    // 保存所有选中图形的初始变换状态（参考Inkscape的grab方法）
    m_initialTransforms.clear();
    for (DrawingShape *shape : m_selectedShapes) {
        if (shape) {
            m_initialTransforms[shape] = shape->transform().transform();
        }
    }
    
    // 保存当前边界框
    m_initialBounds = m_selectionBounds;
    
    // 重置当前相对变换
    m_currentRelativeAffine = QTransform();
    
    // 设置抓取状态
    m_grabbed = true;
    
    // 启用预览
    showPreview(true);
}

void SelectionLayer::ungrabTransform()
{
    // 释放变换状态
    m_grabbed = false;
    m_currentRelativeAffine = QTransform();
    
    // 禁用预览
    showPreview(false);
    
    // 如果是轮廓预览模式，应用最终的变换到图形
    if (m_previewMode == SHOW_OUTLINE) {
        applyFinalTransform();
    }
}

void SelectionLayer::applyFinalTransform()
{
    // 应用累积变换到所有选中的图形
    for (DrawingShape *shape : m_selectedShapes) {
        if (!shape) continue;
        
        // 获取图形的当前变换
        DrawingTransform shapeTransform = shape->transform();
        QTransform currentTransform = m_initialTransforms.value(shape, QTransform());
        
        // 应用累积变换
        QTransform newTransform = currentTransform * m_accumulatedTransform;
        shapeTransform.setTransform(newTransform);
        shape->setTransform(shapeTransform);
    }
}

void SelectionLayer::rotate(double angle, const QPointF &center)
{
    // 使用变换中心或提供的中心点
    QPointF rotateCenter = center.isNull() ? getTransformCenter() : center;
    
    // 计算旋转变换矩阵
    QTransform rotateTransform = calculateRotateTransform(angle, rotateCenter);
    
    // 应用变换
    applyTransform(rotateTransform, rotateCenter);
}

QTransform SelectionLayer::calculateRotateTransform(double angle, const QPointF &center)
{
    // 创建旋转变换矩阵
    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotateRadians(angle);
    transform.translate(-center.x(), -center.y());
    
    return transform;
}

void SelectionLayer::scale(double sx, double sy, const QPointF &center)
{
    // 使用变换中心或提供的中心点
    QPointF scaleCenter = center.isNull() ? getTransformCenter() : center;
    
    // 计算缩放变换矩阵
    QTransform scaleTransform = calculateScaleTransform(sx, sy, scaleCenter);
    
    // 应用变换
    applyTransform(scaleTransform, scaleCenter);
}

QTransform SelectionLayer::calculateScaleTransform(double sx, double sy, const QPointF &center)
{
    // 创建缩放变换矩阵
    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.scale(sx, sy);
    transform.translate(-center.x(), -center.y());
    
    return transform;
}

QPointF SelectionLayer::getTransformCenter() const
{
    if (!m_selectedShapes.isEmpty()) {
        // 计算所有选中图形的联合边界框中心
        return m_selectionBounds.center();
    }
    return QPointF(0, 0);
}

void SelectionLayer::skew(double skewX, double skewY, const QPointF &center)
{
    // 使用变换中心或提供的中心点
    QPointF skewCenter = center.isNull() ? getTransformCenter() : center;
    
    // 计算倾斜变换矩阵
    QTransform skewTransform = calculateSkewTransform(skewX, skewY, skewCenter);
    
    // 应用变换
    applyTransform(skewTransform, skewCenter);
}

QTransform SelectionLayer::calculateSkewTransform(double skewX, double skewY, const QPointF &center)
{
    // 创建倾斜变换矩阵
    QTransform transform;
    transform.translate(center.x(), center.y());
    
    // 应用倾斜
    QTransform skewMatrix;
    skewMatrix.setMatrix(1, skewY, 0, skewX, 1, 0, 0, 0, 1);
    transform = transform * skewMatrix;
    
    transform.translate(-center.x(), -center.y());
    
    return transform;
}

void SelectionLayer::scaleAroundAnchor(double sx, double sy, int handleIndex, const QPointF &anchorPoint)
{
    if (m_selectedShapes.isEmpty()) {
        return;
    }
    
    // 计算锚点（如果未提供）
    QPointF actualAnchor = anchorPoint;
    if (anchorPoint.isNull()) {
        actualAnchor = getAnchorPoint(handleIndex);
    }
    
    // 创建缩放变换矩阵
    QTransform scaleTransform;
    scaleTransform.translate(actualAnchor.x(), actualAnchor.y());
    scaleTransform.scale(sx, sy);
    scaleTransform.translate(-actualAnchor.x(), -actualAnchor.y());
    
    // 应用变换
    applyTransform(scaleTransform, actualAnchor);
}

void SelectionLayer::handleDrag(int handleIndex, const QPointF &scenePos)
{
    if (!m_grabbed || m_initialBounds.isEmpty()) {
        return;
    }
    
    // 计算拖动向量
    QPointF delta = scenePos - m_startScenePos;
    
    // 根据手柄类型执行不同的变换
    switch (handleIndex) {
        case TopLeft:
        case TopRight:
        case BottomLeft:
        case BottomRight:
            // 缩放手柄 - 需要转换为相对于场景中心的坐标
            handleScaleDrag(handleIndex, scenePos);
            break;
            
        case Top:
        case Left:
        case Right:
        case Bottom:
            // 拉伸手柄
            handleStretchDrag(handleIndex, scenePos);
            break;
            
        case Rotate:
            // 旋转手柄
            handleRotateDrag(scenePos);
            break;
    }
}

void SelectionLayer::handleScaleDrag(int handleIndex, const QPointF &scenePos)
{
    if (m_selectedShapes.isEmpty() || m_initialBounds.isEmpty()) {
        return;
    }
    
    // 简化的缩放逻辑
    QPointF center = m_initialBounds.center();
    QPointF delta = scenePos - center;
    QPointF initialDelta = m_startScenePos - center;
    
    // 计算缩放因子
    double sx = 1.0, sy = 1.0;
    
    if (qAbs(initialDelta.x()) > 0.001) {
        sx = delta.x() / initialDelta.x();
    }
    if (qAbs(initialDelta.y()) > 0.001) {
        sy = delta.y() / initialDelta.y();
    }
    
    // 限制缩放因子
    sx = qBound(0.1, sx, 10.0);
    sy = qBound(0.1, sy, 10.0);
    
    // 对每个选中的图形应用简单的缩放
    for (DrawingShape *shape : m_selectedShapes) {
        if (!shape) continue;
        
        // 获取图形的原始边界
        QRectF originalBounds = shape->localBounds();
        QPointF shapeCenter = originalBounds.center();
        
        // 计算新的边界
        QRectF newBounds = QRectF(
            originalBounds.x() * sx,
            originalBounds.y() * sy,
            originalBounds.width() * sx,
            originalBounds.height() * sy
        );
        
        // 更新图形的边界
        if (shape->shapeType() == DrawingShape::Rectangle) {
            static_cast<DrawingRectangle*>(shape)->setRectangle(newBounds);
        } else if (shape->shapeType() == DrawingShape::Ellipse) {
            static_cast<DrawingEllipse*>(shape)->setEllipse(newBounds);
        }
        
        // 保持图形位置不变
        shape->setPos(shape->pos());
    }
    
    // 更新选择边界
    updateSelectionBounds();
    updateHandles();
}

void SelectionLayer::handleStretchDrag(int handleIndex, const QPointF &scenePos)
{
    // 拉伸变换（单方向缩放）
    double sx = 1.0, sy = 1.0;
    QPointF anchorPoint = m_initialBounds.center();
    
    switch (handleIndex) {
        case Top:
        case Bottom:
            // 垂直拉伸
            sy = (scenePos.y() - anchorPoint.y()) / (m_startScenePos.y() - anchorPoint.y());
            break;
        case Left:
        case Right:
            // 水平拉伸
            sx = (scenePos.x() - anchorPoint.x()) / (m_startScenePos.x() - anchorPoint.x());
            break;
    }
    
    // 限制拉伸因子
    sx = qBound(0.1, sx, 10.0);
    sy = qBound(0.1, sy, 10.0);
    
    // 应用拉伸
    scaleAroundAnchor(sx, sy, handleIndex, anchorPoint);
}

void SelectionLayer::handleRotateDrag(const QPointF &scenePos)
{
    // 计算旋转角度
    QPointF center = m_initialBounds.center();
    
    // 计算初始角度和当前角度
    double initialAngle = qAtan2(m_startScenePos.y() - center.y(), 
                                 m_startScenePos.x() - center.x());
    double currentAngle = qAtan2(scenePos.y() - center.y(), 
                                scenePos.x() - center.x());
    
    // 计算旋转增量
    double deltaAngle = currentAngle - initialAngle;
    
    // 应用旋转
    rotate(deltaAngle, center);
}

int SelectionLayer::getOppositeHandle(int handleIndex) const
{
    // 返回相对的手柄索引（用于固定点计算）
    switch (handleIndex) {
        case TopLeft: return BottomRight;
        case TopRight: return BottomLeft;
        case BottomLeft: return TopRight;
        case BottomRight: return TopLeft;
        case Top: return Bottom;
        case Bottom: return Top;
        case Left: return Right;
        case Right: return Left;
        default: return None;
    }
}

void SelectionLayer::rotateAroundAnchor(double angle, int handleIndex)
{
    // 设置变换状态为旋转
    m_transformState = STATE_ROTATE;
    
    // 简单的旋转逻辑：直接旋转选择层
    // 不再是QGraphicsItem，不能使用setRotation()
    
    // 更新手柄位置
    updateHandPositions();
    
    // 准备几何变换
    // 不再是QGraphicsItem，不需要prepareGeometryChange()
    // 不再是QGraphicsItem，不需要update()
    
    // 强制更新手柄位置
    if (m_handleManager) {
        m_handleManager->updateHandles();
    }
}

QPointF SelectionLayer::getAnchorPoint(int handleIndex) const
{
    QPointF localPoint;
    switch (handleIndex) {
        case SelectionLayer::TopLeft: localPoint = m_selectionBounds.topLeft(); break;
        case SelectionLayer::Top: localPoint = QPointF(m_selectionBounds.center().x(), m_selectionBounds.top()); break;
        case SelectionLayer::TopRight: localPoint = m_selectionBounds.topRight(); break;
        case SelectionLayer::Left: localPoint = QPointF(m_selectionBounds.left(), m_selectionBounds.center().y()); break;
        case SelectionLayer::Right: localPoint = QPointF(m_selectionBounds.right(), m_selectionBounds.center().y()); break;
        case SelectionLayer::BottomLeft: localPoint = m_selectionBounds.bottomLeft(); break;
        case SelectionLayer::Bottom: localPoint = QPointF(m_selectionBounds.center().x(), m_selectionBounds.bottom()); break;
        case SelectionLayer::BottomRight: localPoint = m_selectionBounds.bottomRight(); break;
        case SelectionLayer::Rotate: localPoint = QPointF(m_selectionBounds.center().x(), m_selectionBounds.top() - 20); break;
        default: localPoint = m_selectionBounds.center(); break;
    }
    
    // 将本地坐标转换为场景坐标
    return localPoint; // 简化返回本地坐标
}





// QGraphicsItem 方法已移除 - SelectionLayer 不再是场景项

void SelectionLayer::updateHandPositions()
{
    if (m_selectedShapes.isEmpty()) {
        m_handles.clear();
        return;
    }
    
    QRectF bounds = m_selectionBounds;
    QPointF center = bounds.center();
    
    // 更新手柄位置（轴对齐）
    m_handles.resize(9);
    m_handles[TopLeft - 1] = bounds.topLeft();
    m_handles[Top - 1] = QPointF(center.x(), bounds.top());
    m_handles[TopRight - 1] = bounds.topRight();
    m_handles[Left - 1] = QPointF(bounds.left(), center.y());
    m_handles[Right - 1] = QPointF(bounds.right(), center.y());
    m_handles[BottomLeft - 1] = bounds.bottomLeft();
    m_handles[Bottom - 1] = QPointF(center.x(), bounds.bottom());
    m_handles[BottomRight - 1] = bounds.bottomRight();
    m_handles[Rotate - 1] = QPointF(center.x(), bounds.top() - 20); // 旋转手柄
}

void SelectionLayer::updateHandles()
{
    // 更新手柄位置（参考Inkscape的_updateHandles）
    updateHandPositions();
    
    // 通知手柄管理器更新
    if (m_handleManager) {
        m_handleManager->updateHandles();
    }
}

QVector<QPointF> SelectionLayer::getSceneHandlePositions() const
{
    QVector<QPointF> sceneHandles;
    if (m_selectedShapes.isEmpty() || m_sceneSelectionBounds.isEmpty()) {
        return sceneHandles;
    }
    
    QRectF bounds = m_sceneSelectionBounds;
    QPointF center = bounds.center();
    
    // 计算场景坐标中的手柄位置
    sceneHandles.resize(9);
    sceneHandles[TopLeft - 1] = bounds.topLeft();
    sceneHandles[Top - 1] = QPointF(center.x(), bounds.top());
    sceneHandles[TopRight - 1] = bounds.topRight();
    sceneHandles[Left - 1] = QPointF(bounds.left(), center.y());
    sceneHandles[Right - 1] = QPointF(bounds.right(), center.y());
    sceneHandles[BottomLeft - 1] = bounds.bottomLeft();
    sceneHandles[Bottom - 1] = QPointF(center.x(), bounds.bottom());
    sceneHandles[BottomRight - 1] = bounds.bottomRight();
    sceneHandles[Rotate - 1] = QPointF(center.x(), bounds.top() - 20); // 旋转手柄
    
    return sceneHandles;
}

// void SelectionLayer::updateSelectionPeriodically()
// {
//     // 定期更新选择边界和手柄位置 - 已移除定时器
//     if (!m_selectedShapes.isEmpty()) {
//         updateSelectionBounds();
//         updateHandles();
//     }
// }

QGraphicsItem* SelectionLayer::itemAt(const QPointF &pos) const
{
    // 检查是否点击了选择框内的图形
    for (DrawingShape *shape : m_selectedShapes) {
        if (shape && shape->contains(shape->mapFromScene(pos))) {
            return shape;
        }
    }
    return nullptr;
}

void SelectionLayer::applyTransformToShapes()
{
    // 应用选择层的变换到所有选中的形状
    for (DrawingShape *shape : m_selectedShapes) {
        if (!shape) continue;
        
        // 这里可以根据需要实现具体的变换应用逻辑
        // 目前主要通过scale和rotate方法直接应用
    }
}

void SelectionLayer::updateSelectionBounds()
{
    if (m_selectedShapes.isEmpty()) {
        m_selectionBounds = QRectF();
        m_sceneSelectionBounds = QRectF();
        return;
    }
    
    // 计算所有选中图形的联合边界框
    QRectF sceneBounds;
    for (DrawingShape *shape : m_selectedShapes) {
        if (!shape) continue;
        
        // 获取图形在场景中的边界框
        QRectF shapeBounds = shape->boundingRect();
        shapeBounds.translate(shape->pos());
        
        if (sceneBounds.isEmpty()) {
            sceneBounds = shapeBounds;
        } else {
            sceneBounds |= shapeBounds;
        }
    }
    
    // 设置场景边界
    m_sceneSelectionBounds = sceneBounds;
    
    // 更新手柄
    updateHandles();
}

void SelectionLayer::updateInitialTransforms()
{
    // 保存所有选中图形的初始变换（与Inkscape的grab方法相同）
    m_initialTransforms.clear();
    for (DrawingShape *shape : m_selectedShapes) {
        if (shape) {
            m_initialTransforms[shape] = shape->transform().transform();
        }
    }
}

void SelectionLayer::setAnchorAndHandlePositions(int handleIndex)
{
    // 保存当前的选择边界框作为初始AABB
    m_initialBounds = m_selectionBounds;
    
    // 保存初始变换状态（参考Inkscape的grab方法）
    grabTransform();
    
    // 更新手柄位置
    updateHandPositions();
    
    // 设置变换中心
    m_transformCenter = getTransformCenter();
    
    // 根据手柄类型设置变换状态
    if (handleIndex == Rotate) {
        m_transformState = STATE_ROTATE;
    } else {
        m_transformState = STATE_SCALE;
    }
}

// QGraphicsItem 鼠标事件方法已移除 - SelectionLayer 不再是场景项

void SelectionLayer::updatePreview()
{
    if (m_selectedShapes.isEmpty()) {
        m_previewOutlines.clear();
        return;
    }
    
    m_previewOutlines.clear();
    
    // 计算每个图形的预览轮廓
    for (DrawingShape *shape : m_selectedShapes) {
        if (!shape) continue;
        
        // 获取图形的原始边界框
        QRectF shapeBounds = shape->boundingRect();
        
        // 应用累积变换
        QTransform originalTransform = m_initialTransforms.value(shape, QTransform());
        QTransform previewTransform = originalTransform * m_accumulatedTransform;
        
        // 计算变换后的边界框
        QRectF previewBounds = previewTransform.mapRect(shapeBounds);
        
        // 转换为选择层的本地坐标系
        QPointF shapeScenePos = shape->scenePos();
        previewBounds.translate(shapeScenePos);
        QPointF localPos = previewBounds.topLeft(); // 简化坐标转换
        previewBounds.moveTopLeft(localPos);
        
        m_previewOutlines.append(previewBounds);
    }
}

void SelectionLayer::showPreview(bool show)
{
    if (m_showPreview != show) {
        m_showPreview = show;
        if (show) {
            updatePreview();
        }
        // 不再是QGraphicsItem，不需要update()
    }
}

void SelectionLayer::drawPreviewOutlines(QPainter *painter)
{
    if (m_previewOutlines.isEmpty()) {
        return;
    }
    
    // 绘制预览轮廓
    painter->setPen(QPen(QColor(255, 0, 0, 128), 1, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    
    for (const QRectF &outline : m_previewOutlines) {
        painter->drawRect(outline);
    }
}

void SelectionLayer::drawPreviewContent(QPainter *painter)
{
    // 内容预览模式下，图形已经被直接变换，这里可以添加额外的预览效果
    // 例如：半透明覆盖、变换中心指示等
    
    if (!m_grabbed) {
        return;
    }
    
    // 绘制变换中心
    painter->setPen(QPen(QColor(0, 255, 0), 2));
    painter->setBrush(QBrush(QColor(0, 255, 0)));
    QPointF centerLocal = m_transformCenter; // 简化坐标转换
    painter->drawEllipse(QRectF(centerLocal.x() - 3, centerLocal.y() - 3, 6, 6));
    
    // 绘制从中心到各个角的辅助线
    painter->setPen(QPen(QColor(0, 255, 0, 64), 1, Qt::DotLine));
    QRectF bounds = m_selectionBounds;
    painter->drawLine(centerLocal, bounds.topLeft());
    painter->drawLine(centerLocal, bounds.topRight());
    painter->drawLine(centerLocal, bounds.bottomLeft());
    painter->drawLine(centerLocal, bounds.bottomRight());
}


