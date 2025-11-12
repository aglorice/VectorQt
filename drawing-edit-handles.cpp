#include "drawing-edit-handles.h"
#include "drawing-shape.h"
#include "drawing-group.h"
#include "drawingscene.h"
#include "selection-layer.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QCursor>
#include <QtMath>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>

// 静态成员定义
SelectionLayer *EditHandle::s_globalSelectionLayer = nullptr;

// EditHandle implementation
EditHandle::EditHandle(HandleType type, DrawingShape *parent)
    : QGraphicsItem(parent)
    , m_type(type)
    , m_shape(parent)
    , m_rect(-s_handleSize.width()/2, -s_handleSize.height()/2, 
             s_handleSize.width(), s_handleSize.height())
    , m_highlighted(false)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton); // 接受鼠标左键事件
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    // 关键：设置这个标志，使把手不受父项变换的影响
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setZValue(1e10); // 确保在所有元素之上
    // 显式禁用graphics effect，避免滤镜影响
    setGraphicsEffect(nullptr);
}

EditHandle::EditHandle(HandleType type, SelectionLayer *parent)
    : QGraphicsItem(nullptr) // SelectionLayer 不再是 QGraphicsItem
    , m_type(type)
    , m_shape(nullptr)
    , m_rect(-s_handleSize.width()/2, -s_handleSize.height()/2, 
             s_handleSize.width(), s_handleSize.height())
    , m_highlighted(false)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton); // 接受鼠标左键事件
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    // 关键：设置这个标志，使把手不受父项变换的影响
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setZValue(1e10); // 确保在所有元素之上
    // 显式禁用graphics effect，避免滤镜影响
    setGraphicsEffect(nullptr);
}

EditHandle::EditHandle(HandleType type, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_type(type)
    , m_shape(nullptr)
    , m_rect(-s_handleSize.width()/2, -s_handleSize.height()/2, 
             s_handleSize.width(), s_handleSize.height())
    , m_highlighted(false)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton); // 接受鼠标左键事件
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    // 关键：设置这个标志，使把手不受父项变换的影响
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setZValue(1e10); // 确保在所有元素之上
    // 显式禁用graphics effect，避免滤镜影响
    setGraphicsEffect(nullptr);
}

EditHandle::~EditHandle()
{
    // QGraphicsScene会自动管理item的生命周期
    // 确保在析构时清理任何可能引用父对象的状态
    m_dragging = false;
}

QRectF EditHandle::boundingRect() const
{
    // 对于旋转手柄，需要特别考虑其位置可能在图形边界之外
    if (m_type == Rotation) {
        return m_rect.adjusted(-10, -25, 10, 10); // 增加上方边距以容纳旋转手柄
    }
    return m_rect.adjusted(-5, -5, 5, 5); // 扩大边界，确保鼠标事件能被正确捕获
}

QPainterPath EditHandle::shape() const
{
    QPainterPath path;
    path.addRect(m_rect);
    return path;
}

void EditHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    // 根据把手类型设置颜色
    QColor fillColor = m_highlighted ? QColor(255, 200, 0) : QColor(255, 255, 255);
    QColor strokeColor = QColor(0, 0, 0);
    
    // 🌟 检测手柄是否与任何对象相交，如果相交则设置为半透明
    if (intersectsAnyObject()) {
        fillColor.setAlpha(150); // 设置为半透明
        strokeColor.setAlpha(200); // 边框也稍微透明
    }
    
    painter->setPen(QPen(strokeColor, 1)); // 减小边框宽度，从2改为1
    painter->setBrush(QBrush(fillColor));
    
    // 旋转把手特殊绘制
    if (handleType() == Rotation) {
        painter->setBrush(QBrush(QColor(0, 255, 0)));
        painter->drawEllipse(m_rect);
    } 
    // 圆角控制手柄（圆形）
    else if (handleType() == CornerRadius || handleType() == ArcControl) {
        painter->drawEllipse(m_rect);
    }
    // 尺寸控制手柄（方形）
    else if (handleType() == SizeControl) {
        painter->drawRect(m_rect);
    }
    // 其他手柄（默认方形）
    else {
        // 绘制方形把手
        painter->drawRect(m_rect);
    }
}

void EditHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        qDebug() << "EditHandle::mousePressEvent called, handleType:" << handleType();
        
        // 获取父对象（可能是DrawingShape或SelectionLayer）
        auto parent = parentItem();
        
        // 对于自定义节点手柄（没有父对象），直接开始拖动
        // 包括所有节点编辑手柄类型
        if (!parent && (handleType() >= Custom || 
                       handleType() == CornerRadius || 
                       handleType() == SizeControl || 
                       handleType() == ArcControl)) {
            qDebug() << "Starting drag for custom node handle, type:" << handleType();
            startDrag(event->scenePos());
            event->accept();
            return;
        }
        
        if (!parent) {
            qDebug() << "No parent item!";
            QGraphicsItem::mousePressEvent(event);
            return;
        }
        
        SelectionLayer *selectionLayer = nullptr; // 直接设置为 nullptr，因为 parent 不再是 SelectionLayer
        DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(parent);
        
        qDebug() << "Parent type - shape:" << (shape ? "yes" : "no") << "selectionLayer:" << (selectionLayer ? "yes" : "no");
        
        // 只处理 DrawingShape
        if (shape) {
            qDebug() << "Calling startDrag for DrawingShape at position:" << event->scenePos();
            startDrag(event->scenePos());
            event->accept();
            return;
        }
    }
    QGraphicsItem::mousePressEvent(event);
}

void EditHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_dragging) {
        // 对于自定义节点手柄（没有父对象），处理拖动
        // 包括新的节点编辑手柄类型
        if (!parentItem() && (handleType() >= Custom || 
                             handleType() == CornerRadius || 
                             handleType() == SizeControl || 
                             handleType() == ArcControl)) {
            //qDebug() << "EditHandle::mouseMoveEvent dragging custom handle, handleType:" << handleType() << "pos:" << event->scenePos();
            updateDrag(event->scenePos());
            event->accept();
            return;
        }
        
        // 检查父对象是否仍然存在
        if (!parentItem()) {
            qDebug() << "Parent item deleted during drag!";
            m_dragging = false;
            event->accept();
            return;
        }
        
        //qDebug() << "EditHandle::mouseMoveEvent dragging, handleType:" << handleType() << "pos:" << event->scenePos();
        updateDrag(event->scenePos());
        
        // 更新所有手柄位置
        auto parent = parentItem();
        if (parent) {
            DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(parent);
            if (shape && shape->editHandleManager()) {
                shape->editHandleManager()->updateHandles();
            }
        }
        
        event->accept();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void EditHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        endDrag();
        
        // 对于自定义节点手柄（没有父对象），不需要特殊处理
        // 包括新的节点编辑手柄类型
        if (!parentItem() && (handleType() >= Custom || 
                             handleType() == CornerRadius || 
                             handleType() == SizeControl || 
                             handleType() == ArcControl)) {
            event->accept();
            return;
        }
        
        // 如果是旋转手柄，恢复到初始位置
        if (handleType() == Rotation) {
            // 触发位置更新
            if (auto manager = (m_shape ? m_shape->editHandleManager() : nullptr)) {
                manager->updateHandles();
            }
        }
        
        event->accept();
        return;
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void EditHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_highlighted = true;
    update();
    
    // 根据把手类型设置光标
    switch (handleType()) {
        case TopLeft:
        case BottomRight:
            setCursor(QCursor(Qt::SizeFDiagCursor));
            break;
        case TopRight:
        case BottomLeft:
            setCursor(QCursor(Qt::SizeBDiagCursor));
            break;
        case TopCenter:
        case BottomCenter:
            setCursor(QCursor(Qt::SizeVerCursor));
            break;
        case CenterLeft:
        case CenterRight:
            setCursor(QCursor(Qt::SizeHorCursor));
            break;
        case Rotation:
            setCursor(QCursor(Qt::CrossCursor));
            break;
        default:
            setCursor(QCursor(Qt::ArrowCursor));
            break;
    }
}

void EditHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    m_highlighted = false;
    update();
    unsetCursor();
}

void EditHandle::setPos(const QPointF &pos)
{
    QGraphicsItem::setPos(pos);
}

void EditHandle::setRect(const QRectF &rect)
{
    if (m_rect != rect) {
        prepareGeometryChange();
        m_rect = rect;
        update();
    }
}

// EditHandleManager implementation
EditHandleManager::EditHandleManager(DrawingShape *shape, QObject *parent)
    : QObject(parent)
    , m_shape(shape)
    , m_selectionLayer(nullptr)
    , m_activeHandle(nullptr)
    , m_dragging(false)
{
    // 不在此处创建手柄，让showHandles方法来创建
}

EditHandleManager::EditHandleManager(SelectionLayer *selectionLayer, QObject *parent)
    : QObject(parent)
    , m_shape(nullptr)
    , m_selectionLayer(selectionLayer)
    , m_activeHandle(nullptr)
    , m_dragging(false)
{
    // 不在此处创建手柄，让showHandles方法来创建
}

EditHandleManager::~EditHandleManager()
{
    // 隐藏所有把手
    for (EditHandle *handle : m_handles) {
        if (handle) {
            handle->hide();
        }
    }
    
    // 清理所有编辑把手
    qDeleteAll(m_handles);
    m_handles.clear();
}

void EditHandleManager::createHandles()
{
    // 先清理已存在的把手
    for (EditHandle *handle : m_handles) {
        if (handle) {
            handle->hide();
            // 不要删除把手，因为它们可能仍在场景中使用
        }
    }
    m_handles.clear();
    
    QGraphicsItem *parent = nullptr;
    QGraphicsScene *scene = nullptr;
    
    if (m_shape) {
        parent = m_shape;
        scene = m_shape->scene();
        qDebug() << "Creating handles for DrawingShape:" << m_shape;
        // 如果图形还没有添加到场景中，暂时不创建手柄
        if (!scene) {
            qDebug() << "Shape not in scene, deferring handle creation";
            return;
        }
    } else if (m_selectionLayer) {
        // SelectionLayer 不再是 QGraphicsItem，需要获取场景
        // 尝试从选中的图形获取场景
        if (!m_selectionLayer->selectedShapes().isEmpty()) {
            scene = m_selectionLayer->selectedShapes().first()->scene();
        } else {
            // 如果没有选中的图形，暂时不创建把手
            // 等到有图形被选中时再创建
            qDebug() << "No selected shapes, deferring handle creation";
            return;
        }
        qDebug() << "Creating handles for SelectionLayer, scene:" << scene;
    }
    
    // 确保场景有效
    if (!scene && m_shape) {
        qDebug() << "No valid scene for shape, cannot create handles";
        return;
    }
    
    // 创建所有把手
    qDebug() << "Creating handles, parent:" << parent << "scene:" << scene;
    m_handles.append(new EditHandle(EditHandle::TopLeft, parent));
    m_handles.append(new EditHandle(EditHandle::TopCenter, parent));
    m_handles.append(new EditHandle(EditHandle::TopRight, parent));
    m_handles.append(new EditHandle(EditHandle::CenterLeft, parent));
    m_handles.append(new EditHandle(EditHandle::CenterRight, parent));
    m_handles.append(new EditHandle(EditHandle::BottomLeft, parent));
    m_handles.append(new EditHandle(EditHandle::BottomCenter, parent));
    m_handles.append(new EditHandle(EditHandle::BottomRight, parent));
    m_handles.append(new EditHandle(EditHandle::Rotation, parent));
    qDebug() << "Created" << m_handles.count() << "handles";
    
    // 对于SelectionLayer的把手，需要手动添加到场景中
    if (m_selectionLayer && scene) {
        for (EditHandle *handle : m_handles) {
            if (handle) {
                scene->addItem(handle);
            }
        }
    }
    
    // 对于DrawingShape的把手，不需要添加到场景中，因为它们已经有父对象
    // 把手会自动跟随父对象显示在场景中
    
    updateHandlePositions();
    
    // 确保编辑把手能接收鼠标事件
    for (EditHandle *handle : m_handles) {
        if (handle) {
            handle->setAcceptedMouseButtons(Qt::LeftButton);
            handle->setAcceptHoverEvents(true);
        }
    }
}

void EditHandleManager::updateHandlePositions()
{
    if (m_shape) {
        // 对于DrawingShape，获取图形在场景中的边界框
        QRectF shapeBounds = m_shape->boundingRect();
        
        // 安全检查：确保边界框有效
        if (shapeBounds.isNull() || shapeBounds.isEmpty()) {
            qDebug() << "Invalid shape bounds for EditHandleManager::updateHandlePositions";
            return;
        }
        
        // 将图形的本地边界框转换为场景坐标
        // 使用mapRectToScene方法确保正确应用变换
        QRectF sceneBounds = m_shape->mapRectToScene(shapeBounds);
        QPointF sceneCenter = sceneBounds.center();
        
        // 更新所有把手的位置（用于单个图形的情况）
        for (EditHandle *handle : m_handles) {
            if (!handle) continue;
            
            QPointF scenePos;
            switch (handle->handleType()) {
                case EditHandle::TopLeft:
                    scenePos = sceneBounds.topLeft();
                    break;
                case EditHandle::TopCenter:
                    scenePos = QPointF(sceneCenter.x(), sceneBounds.top());
                    break;
                case EditHandle::TopRight:
                    scenePos = sceneBounds.topRight();
                    break;
                case EditHandle::CenterLeft:
                    scenePos = QPointF(sceneBounds.left(), sceneCenter.y());
                    break;
                case EditHandle::CenterRight:
                    scenePos = QPointF(sceneBounds.right(), sceneCenter.y());
                    break;
                case EditHandle::BottomLeft:
                    scenePos = sceneBounds.bottomLeft();
                    break;
                case EditHandle::BottomCenter:
                    scenePos = QPointF(sceneCenter.x(), sceneBounds.bottom());
                    break;
                case EditHandle::BottomRight:
                    scenePos = sceneBounds.bottomRight();
                    break;
                case EditHandle::Rotation:
                    // 旋转手柄位置：在轴对齐的选择框上方20像素处
                    scenePos = QPointF(sceneCenter.x(), sceneBounds.top() - 20);
                    break;
                default:
                    continue;
            }
            
            // 将场景坐标转换为图形的本地坐标
            // 使用mapFromScene确保正确应用逆变换
            QPointF localPos = m_shape->mapFromScene(scenePos);
            handle->setPos(localPos);
        }
    } else if (m_selectionLayer) {
        // 使用场景坐标中的手柄位置
        QVector<QPointF> sceneHandles = m_selectionLayer->getSceneHandlePositions();
        
        for (int i = 0; i < m_handles.size() && i < sceneHandles.size(); ++i) {
            if (m_handles[i]) {
                m_handles[i]->setPos(sceneHandles[i]);
            }
        }
        return; // 已经处理了所有把手
    }
}

void EditHandleManager::showHandles()
{
    // 如果手柄尚未创建，则创建它们
    if (m_handles.isEmpty()) {
        createHandles();
    }
    
    for (EditHandle *handle : m_handles) {
        if (handle) {
            handle->show();
        }
    }
}

void EditHandleManager::hideHandles()
{
    for (EditHandle *handle : m_handles) {
        handle->hide();
    }
}

void EditHandleManager::updateHandles()
{
    updateHandlePositions();
}

EditHandle* EditHandleManager::handleAt(EditHandle::HandleType type) const
{
    for (EditHandle *handle : m_handles) {
        if (handle->handleType() == type) {
            return handle;
        }
    }
    return nullptr;
}

void EditHandleManager::setActiveHandle(EditHandle *handle)
{
    if (m_activeHandle != handle) {
        m_activeHandle = handle;
        m_dragging = (handle != nullptr);
    }
}

// SelectionIndicator implementation
SelectionIndicator::SelectionIndicator(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setZValue(999); // 在形状上方，但在把手下方
    hide(); // 默认隐藏
}

QRectF SelectionIndicator::boundingRect() const
{
    return m_bounds.adjusted(-5, -25, 5, 5); // 为选择框留出空间，特别注意旋转手柄在上方20像素
}

QPainterPath SelectionIndicator::shape() const
{
    QPainterPath path;
    path.addRect(m_bounds);
    return path;
}

void SelectionIndicator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    // 绘制选择框
    if (!m_bounds.isEmpty()) {
        painter->setPen(QPen(QColor(0, 120, 255), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_bounds);
        
        // 如果有变换，绘制变换边界框
        if (!m_transform.isIdentity()) {
            painter->save();
            painter->setTransform(m_transform, true);
            painter->setPen(QPen(QColor(255, 0, 0), 1, Qt::DotLine));
            painter->drawRect(m_bounds);
            painter->restore();
        }
    }
}

void SelectionIndicator::updateIndicator(const QRectF &bounds, const QTransform &transform)
{
    // 始终更新边界和变换，即使它们看起来相同
    // 这可以解决在某些情况下选择框不更新的问题
    prepareGeometryChange();
    m_bounds = bounds;
    m_transform = transform;
    update();
}

// 辅助函数：获取锚点位置
static DrawingTransform::AnchorPoint getAnchorPointForHandle(EditHandle::HandleType handle, const QRectF &bounds)
{
    switch (handle) {
        case EditHandle::TopLeft: return DrawingTransform::TopLeft;
        case EditHandle::TopCenter: return DrawingTransform::TopCenter;
        case EditHandle::TopRight: return DrawingTransform::TopRight;
        case EditHandle::CenterLeft: return DrawingTransform::CenterLeft;
        case EditHandle::CenterRight: return DrawingTransform::CenterRight;
        case EditHandle::BottomLeft: return DrawingTransform::BottomLeft;
        case EditHandle::BottomCenter: return DrawingTransform::BottomCenter;
        case EditHandle::BottomRight: return DrawingTransform::BottomRight;
        default: return DrawingTransform::Center;
    }
}

// EditHandle拖动实现
void EditHandle::startDrag(const QPointF &scenePos)
{
    qDebug() << "EditHandle::startDrag called at:" << scenePos << "handleType:" << handleType();
    
    m_dragging = true;
    m_dragStartPos = scenePos;
    m_accumulatedRotation = 0.0;  // 重置累积旋转角度
    
    // 获取父对象（可能是DrawingShape或SelectionLayer）
    auto parent = parentItem();
    
    if (!parent) {
        // 对于自定义节点手柄（没有父对象），只设置拖动状态
        qDebug() << "Starting drag for custom node handle";
        return;
    }
    
    SelectionLayer *selectionLayer = nullptr; // 直接设置为 nullptr，因为 parent 不再是 SelectionLayer
    DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(parent);
    
    if (shape) {
        // 🌟 对于DrawingGroup，使用实际的边界框并调用grab方法
        if (shape->shapeType() == DrawingShape::Group) {
            DrawingGroup *group = static_cast<DrawingGroup*>(shape);
            // 使用组合的实际边界框，包含所有子项
            m_originalBounds = group->boundingRect();
            // 🌟 关键：调用grab方法保存当前变换状态
            group->grabTransform();
        } else {
            // 对于普通图形，使用本地边界框
            m_originalBounds = shape->localBounds();
        }
        
        m_originalTransform = shape->transform();
        m_originalRotation = m_originalTransform.rotation(); // 保存原始旋转角度
        
        qDebug() << "StartDrag for DrawingShape, bounds:" << m_originalBounds;
    }
}

void EditHandle::updateDrag(const QPointF &scenePos)
{
    if (!m_dragging) return;
    
    // 获取父对象（可能是DrawingShape或SelectionLayer）
    auto parent = parentItem();
    
    // 对于自定义节点手柄（没有父对象），直接更新位置
    if (!parent && (handleType() >= Custom || 
                   handleType() == CornerRadius || 
                   handleType() == SizeControl || 
                   handleType() == ArcControl)) {
        setPos(scenePos);
        return;
    }
    
    SelectionLayer *selectionLayer = s_globalSelectionLayer; // 使用全局引用
    DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(parent);
    
    if (shape) {
        // 原有的单个图形变换逻辑
        updateDragForShape(scenePos);
    } else if (selectionLayer) {
        // SelectionLayer的变换逻辑
        int handleIndex = 0;
        switch (handleType()) {
            case TopLeft: handleIndex = SelectionLayer::TopLeft; break;
            case TopCenter: handleIndex = SelectionLayer::Top; break;
            case TopRight: handleIndex = SelectionLayer::TopRight; break;
            case CenterLeft: handleIndex = SelectionLayer::Left; break;
            case CenterRight: handleIndex = SelectionLayer::Right; break;
            case BottomLeft: handleIndex = SelectionLayer::BottomLeft; break;
            case BottomCenter: handleIndex = SelectionLayer::Bottom; break;
            case BottomRight: handleIndex = SelectionLayer::BottomRight; break;
            case Rotation: handleIndex = SelectionLayer::Rotate; break;
            case CornerRadius:
            case SizeControl:
            case ArcControl:
            case Custom: 
                // 自定义手柄暂不处理 SelectionLayer 拖动
                return;
        }
        selectionLayer->handleDrag(handleIndex, scenePos);
    }
}

void EditHandle::updateDragForShape(const QPointF &scenePos)
{
    if (!m_dragging) return;
    
    // 获取父对象（可能是DrawingShape或SelectionLayer）
    auto parent = parentItem();
    SelectionLayer *selectionLayer = s_globalSelectionLayer; // 使用全局引用
    DrawingShape *shape = qgraphicsitem_cast<DrawingShape*>(parent);
    
    if (shape) {
        // 应用网格对齐
        QPointF alignedPos = scenePos;
        if (shape->scene()) {
            DrawingScene *drawingScene = qobject_cast<DrawingScene*>(shape->scene());
            if (drawingScene && drawingScene->isGridAlignmentEnabled()) {
                // 使用智能网格吸附
                DrawingScene::SnapResult gridSnap = drawingScene->smartAlignToGrid(scenePos);
                alignedPos = gridSnap.snappedPos;
                
                // 尝试对象吸附
                DrawingScene::ObjectSnapResult objectSnap = drawingScene->snapToObjects(scenePos, shape);
                if (objectSnap.snappedToObject) {
                    // 对象吸附优先级更高
                    alignedPos = objectSnap.snappedPos;
                }
            }
        }
        
        // 根据手柄类型执行相应的变换
        if (handleType() == Rotation) {
            // 旋转手柄逻辑
            QPointF boundsCenter = m_originalBounds.center();
            QPointF sceneCenter = shape->mapToScene(boundsCenter);
            QPointF startVec = m_dragStartPos - sceneCenter;
            QPointF currentVec = alignedPos - sceneCenter;  // 使用对齐后的位置
            
            double startAngle = qAtan2(startVec.y(), startVec.x());
            double currentAngle = qAtan2(currentVec.y(), currentVec.x());
            double angleDelta = currentAngle - startAngle;
            
            // 处理角度跨越问题
            if (angleDelta > M_PI) {
                angleDelta -= 2 * M_PI;
            } else if (angleDelta < -M_PI) {
                angleDelta += 2 * M_PI;
            }
            
            // 计算新的变换
            DrawingTransform newTransform = m_originalTransform;
            newTransform.rotateAroundAnchor(angleDelta, DrawingTransform::Center, m_originalBounds);
            
            // 🌟 关键：如果是DrawingGroup，使用专门的统一旋转方法
            if (shape && shape->shapeType() == DrawingShape::Group) {
                DrawingGroup *group = static_cast<DrawingGroup*>(shape);
                // 使用统一的旋转中心和角度
                QPointF boundsCenter = m_originalBounds.center();
                QPointF sceneCenter = shape->mapToScene(boundsCenter);
                group->applyRotationWithHandle(sceneCenter, angleDelta);
            } else {
                shape->setTransform(newTransform);
            }
        } else {
            // 缩放手柄逻辑
            if (handleType() < TopLeft || handleType() > BottomRight) {
                return;
            }
            
            // 🌟 简化锚点计算，使用DrawingTransform的锚点枚举
            DrawingTransform::AnchorPoint anchor = DrawingTransform::Center;
            
            // 根据手柄类型确定锚点
            switch (handleType()) {
                case EditHandle::TopLeft:
                    anchor = DrawingTransform::BottomRight;
                    break;
                case EditHandle::TopRight:
                    anchor = DrawingTransform::BottomLeft;
                    break;
                case EditHandle::BottomLeft:
                    anchor = DrawingTransform::TopRight;
                    break;
                case EditHandle::BottomRight:
                    anchor = DrawingTransform::TopLeft;
                    break;
                case EditHandle::TopCenter:
                    anchor = DrawingTransform::BottomCenter;
                    break;
                case EditHandle::BottomCenter:
                    anchor = DrawingTransform::TopCenter;
                    break;
                case EditHandle::CenterLeft:
                    anchor = DrawingTransform::CenterRight;
                    break;
                case EditHandle::CenterRight:
                    anchor = DrawingTransform::CenterLeft;
                    break;
                default:
                    return;
            }
            
            // 获取固定锚点位置（让DrawingTransform计算）
            QRectF originalBounds = m_originalBounds;
            QPointF fixedAnchor = m_originalTransform.getAnchorPoint(anchor, originalBounds);
            
            // 将固定锚点转换为场景坐标
            QPointF anchorScenePos = shape->mapToScene(fixedAnchor);
            
            // 获取当前手柄在场景中的位置（使用对齐后的位置）
            QPointF currentHandlePos = alignedPos;
            
            // 获取拖动开始时手柄在场景中的位置
            QPointF initialHandlePos = m_dragStartPos;
            
            // 计算相对于锚点的向量
            QPointF initialVec = initialHandlePos - anchorScenePos;
            QPointF currentVec = currentHandlePos - anchorScenePos;
            
            // 计算缩放因子
            double sx = 1.0, sy = 1.0;
            if (!qFuzzyIsNull(initialVec.x())) {
                sx = currentVec.x() / initialVec.x();
            }
            if (!qFuzzyIsNull(initialVec.y())) {
                sy = currentVec.y() / initialVec.y();
            }
            
            // 根据手柄类型限制缩放方向
            switch (handleType()) {
                case EditHandle::TopCenter:
                case EditHandle::BottomCenter:
                    sx = 1.0; // 只垂直缩放
                    break;
                case EditHandle::CenterLeft:
                case EditHandle::CenterRight:
                    sy = 1.0; // 只水平缩放
                    break;
                case EditHandle::TopLeft:
                case EditHandle::TopRight:
                case EditHandle::BottomLeft:
                case EditHandle::BottomRight:
                    // 角手柄：两个方向都缩放
                    break;
                default:
                    break;
            }
            
            // 限制缩放范围
            sx = qBound(0.01, sx, 100.0);
            sy = qBound(0.01, sy, 100.0);
            
            // 🌟 使用scaleAroundAnchor方法，让DrawingTransform处理锚点
            DrawingTransform newTransform = m_originalTransform;
            newTransform.scaleAroundAnchor(sx, sy, anchor, m_originalBounds);
            
            // 🌟 关键：如果是DrawingGroup，使用更精确的手柄感知方法
            if (shape && shape->shapeType() == DrawingShape::Group) {
                DrawingGroup *group = static_cast<DrawingGroup*>(shape);
                // 使用更精确的手柄感知变换方法，处理旋转后的缩放
                group->applyTransformWithHandle(handleType(), m_dragStartPos, alignedPos);
            } else {
                shape->setTransform(newTransform);
            }
        }
        
        // 更新手柄位置
        if (auto manager = shape->editHandleManager()) {
            manager->updateHandles();
        }
    } else if (selectionLayer) {
        // SelectionLayer的变换逻辑
        int handleIndex = 0;
        switch (handleType()) {
            case TopLeft: handleIndex = SelectionLayer::TopLeft; break;
            case TopCenter: handleIndex = SelectionLayer::Top; break;
            case TopRight: handleIndex = SelectionLayer::TopRight; break;
            case CenterLeft: handleIndex = SelectionLayer::Left; break;
            case CenterRight: handleIndex = SelectionLayer::Right; break;
            case BottomLeft: handleIndex = SelectionLayer::BottomLeft; break;
            case BottomCenter: handleIndex = SelectionLayer::Bottom; break;
            case BottomRight: handleIndex = SelectionLayer::BottomRight; break;
            case Rotation: handleIndex = SelectionLayer::Rotate; break;
            case CornerRadius:
            case SizeControl:
            case ArcControl:
            case Custom: 
                // 自定义手柄暂不处理 SelectionLayer 拖动
                return;
        }
        selectionLayer->handleDrag(handleIndex, scenePos);
    }
}

void EditHandle::updateDragForSelectionLayer(const QPointF &scenePos)
{
    qDebug() << "updateDragForSelectionLayer called, handleType:" << handleType() << "scenePos:" << scenePos;
    
    auto parent = parentItem();
    if (!parent) {
        qDebug() << "No parent item found!";
        return;
    }
    
    SelectionLayer *selectionLayer = nullptr; // 直接设置为 nullptr，因为 parent 不再是 SelectionLayer
    if (!selectionLayer) {
        qDebug() << "No selection layer found!";
        return;
    }
    
    // 处理旋转（使用新的变换系统）
    if (handleType() == Rotation) {
        // 使用选择边界中心作为旋转中心
        QPointF boundsCenter = m_originalBounds.center();
        QPointF sceneCenter = boundsCenter; // 移除 mapToScene 调用
        QPointF startVec = m_dragStartPos - sceneCenter;
        QPointF currentVec = scenePos - sceneCenter;
        
        double startAngle = qAtan2(startVec.y(), startVec.x());
        double currentAngle = qAtan2(currentVec.y(), currentVec.x());
        double angleDelta = currentAngle - startAngle;
        
        // 处理角度跨越问题
        if (angleDelta > M_PI) {
            angleDelta -= 2 * M_PI;
        } else if (angleDelta < -M_PI) {
            angleDelta += 2 * M_PI;
        }
        
        // 降低旋转灵敏度，使其更加平缓
        angleDelta *= 0.5;
        
        // 更新累积旋转角度
        m_accumulatedRotation += angleDelta;
        
        // 应用旋转到选择层（使用新的变换系统）
        selectionLayer->rotateAroundAnchor(m_accumulatedRotation, static_cast<int>(Rotation));
        
        // 更新拖动起始位置，以便下次计算增量
        m_dragStartPos = scenePos;
        return;
    }
    
    // 处理缩放 - 基于新的仿射变换系统
    int handleIndex = static_cast<int>(handleType());
    
    // 使用选择层的固定锚点
    QPointF anchorScene = m_fixedAnchor;
    
    // 计算初始手柄位置（基于原始边界，使用场景坐标）
    QPointF initialHandlePos;
    QRectF originalBounds = m_originalBounds;
    
    switch (handleIndex) {
        case SelectionLayer::TopLeft: 
            initialHandlePos = originalBounds.topLeft(); 
            break;
        case SelectionLayer::TopRight: 
            initialHandlePos = originalBounds.topRight(); 
            break;
        case SelectionLayer::BottomLeft: 
            initialHandlePos = originalBounds.bottomLeft(); 
            break;
        case SelectionLayer::BottomRight: 
            initialHandlePos = originalBounds.bottomRight(); 
            break;
        case SelectionLayer::Top:
            initialHandlePos = QPointF(originalBounds.center().x(), originalBounds.top());
            break;
        case SelectionLayer::Bottom:
            initialHandlePos = QPointF(originalBounds.center().x(), originalBounds.bottom());
            break;
        case SelectionLayer::Left:
            initialHandlePos = QPointF(originalBounds.left(), originalBounds.center().y());
            break;
        case SelectionLayer::Right:
            initialHandlePos = QPointF(originalBounds.right(), originalBounds.center().y());
            break;
        default:
            return;
    }
    
    // 计算向量
    QPointF initVec = initialHandlePos - anchorScene;
    QPointF realVec = scenePos - anchorScene;
    
    // 计算缩放因子 - 基于初始手柄位置到锚点的距离比例
    double sx = 1.0, sy = 1.0;
    switch (handleIndex) {
        case SelectionLayer::TopLeft:
        case SelectionLayer::TopRight:
        case SelectionLayer::BottomLeft:
        case SelectionLayer::BottomRight:
            // 对角手柄：两个方向都缩放
            if (!qFuzzyIsNull(initVec.x()) && !qFuzzyIsNull(initVec.y())) {
                // 计算相对于固定锚点的缩放因子
                sx = realVec.x() / initVec.x();
                sy = realVec.y() / initVec.y();
            } else {
                return; // 避免除零错误
            }
            break;
        case SelectionLayer::Top:
        case SelectionLayer::Bottom:
            // 上下边手柄：只缩放Y轴
            if (!qFuzzyIsNull(initVec.y())) {
                sy = realVec.y() / initVec.y();
                sx = 1.0;
            } else {
                return; // 避免除零错误
            }
            break;
        case SelectionLayer::Left:
        case SelectionLayer::Right:
            // 左右边手柄：只缩放X轴
            if (!qFuzzyIsNull(initVec.x())) {
                sx = realVec.x() / initVec.x();
                sy = 1.0;
            } else {
                return; // 避免除零错误
            }
            break;
        default:
            return;
    }
    
    // 限制缩放范围，使用更合理的范围
    const qreal minScale = 0.1;
    const qreal maxScale = 10.0;
    
    sx = qBound(minScale, sx, maxScale);
    sy = qBound(minScale, sy, maxScale);
    
    qDebug() << "Applying scale - sx:" << sx << "sy:" << sy << "anchor:" << anchorScene << "handleIndex:" << handleIndex;
    
    // 应用缩放到选择层（使用新的变换系统）
    selectionLayer->scaleAroundAnchor(sx, sy, handleIndex, anchorScene);
}

void EditHandle::endDrag()
{
    m_dragging = false;
}

// 辅助函数：获取手柄在本地坐标中的位置
QPointF EditHandle::getHandlePositionInLocalCoords(HandleType type, const QRectF &bounds) const
{
    QPointF center = bounds.center();
    switch (type) {
        case EditHandle::TopLeft:
            return bounds.topLeft();
        case EditHandle::TopCenter:
            return QPointF(center.x(), bounds.top());
        case EditHandle::TopRight:
            return bounds.topRight();
        case EditHandle::CenterLeft:
            return QPointF(bounds.left(), center.y());
        case EditHandle::CenterRight:
            return QPointF(bounds.right(), center.y());
        case EditHandle::BottomLeft:
            return bounds.bottomLeft();
        case EditHandle::BottomCenter:
            return QPointF(center.x(), bounds.bottom());
        case EditHandle::BottomRight:
            return bounds.bottomRight();
        case EditHandle::Rotation:
            return QPointF(center.x(), bounds.top() - 20); // 旋转手柄
        default:
            return center;
    }
}

bool EditHandle::intersectsAnyObject() const
{
    // 获取手柄的场景边界矩形
    QRectF handleSceneRect = mapRectToScene(boundingRect());
    
    // 获取父对象
    QGraphicsItem *parent = parentItem();
    if (!parent) {
        return false;
    }
    
    // 直接检查与父对象的相交
    QRectF parentSceneRect = parent->mapRectToScene(parent->boundingRect());
    if (handleSceneRect.intersects(parentSceneRect)) {
        return true;
    }
    
    // 获取场景
    QGraphicsScene *scene = this->scene();
    if (!scene) {
        return false;
    }
    
    // 检查与其他对象的相交
    QList<QGraphicsItem*> collidingItems = scene->items(handleSceneRect);
    for (QGraphicsItem *item : collidingItems) {
        if (item != this && item != parent) {
            // 排除自己和父对象
            return true;
        }
    }
    
    return false;
}

#include "drawing-edit-handles.moc"
