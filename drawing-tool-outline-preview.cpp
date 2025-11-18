#include "drawing-tool-outline-preview.h"
#include "drawingscene.h"
#include "drawingview.h"
#include "drawing-shape.h"
#include "transform-handle.h"
#include "drawing-transform.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QPainterPath>
#include <QtMath>
#include <QTimer>
#include <QUndoCommand>

// 数学辅助函数
namespace {
    inline qreal distance(const QPointF &a, const QPointF &b) {
        return QLineF(a, b).length();
    }

    inline qreal safeDiv(qreal a, qreal b) {
        return (qAbs(b) < 1e-6) ? 1.0 : a / b;
    }

    inline QPointF getHandlePos(const QRectF &rect, TransformHandle::HandleType type) {
        switch (type) {
            case TransformHandle::TopLeft:     return rect.topLeft();
            case TransformHandle::TopRight:    return rect.topRight();
            case TransformHandle::BottomLeft:  return rect.bottomLeft();
            case TransformHandle::BottomRight: return rect.bottomRight();
            case TransformHandle::Left:        return QPointF(rect.left(), rect.center().y());
            case TransformHandle::Right:       return QPointF(rect.right(), rect.center().y());
            case TransformHandle::Top:         return QPointF(rect.center().x(), rect.top());
            case TransformHandle::Bottom:      return QPointF(rect.center().x(), rect.bottom());
            case TransformHandle::Center:      return rect.center();
            default: return rect.center();
        }
    }
}

OutlinePreviewTransformTool::OutlinePreviewTransformTool(QObject *parent)
    : ToolBase(parent)
    , m_anchorPoint(nullptr)
    , m_dragPoint(nullptr)
    , m_outlinePreview(nullptr)
    , m_handleManager(nullptr)
{
}

OutlinePreviewTransformTool::~OutlinePreviewTransformTool()
{
    deactivate(); // 确保清理
    
    // 清理 HandleManager
    if (m_handleManager) {
        delete m_handleManager;
        m_handleManager = nullptr;
    }
}

void OutlinePreviewTransformTool::activate(DrawingScene *scene, DrawingView *view)
{
    ToolBase::activate(scene, view);
    if (view) view->setDragMode(QGraphicsView::RubberBandDrag);
    
    // 每次激活时都重新创建 HandleManager，确保场景指针正确
    if (m_handleManager) {
        delete m_handleManager;
        m_handleManager = nullptr;
    }
    
    if (scene) {
        m_handleManager = new HandleManager(scene, this);
    }
    
    // 连接选择变化信号
    if (scene) {
        connect(scene, &DrawingScene::selectionChanged, this, 
                &OutlinePreviewTransformTool::onSelectionChanged, Qt::UniqueConnection);
        connect(scene, &DrawingScene::objectStateChanged, this,
                &OutlinePreviewTransformTool::onObjectStateChanged, Qt::UniqueConnection);
        
        // 禁用所有选中图形的内部选择框
        disableInternalSelectionIndicators();
        
        // 初始显示手柄
        updateHandlePositions();
    }
}

void OutlinePreviewTransformTool::deactivate()
{
    if (m_state == STATE_GRABBED) {
        ungrab(false); // 取消变换
    }
    
    if (m_view) m_view->setDragMode(QGraphicsView::NoDrag);
    if (m_handleManager) {
        m_handleManager->hideHandles();
        delete m_handleManager;
        m_handleManager = nullptr;
    }
    
    // 恢复内部选择框
    enableInternalSelectionIndicators();
    
    ToolBase::deactivate();
}

bool OutlinePreviewTransformTool::mousePressEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (!m_scene || event->button() != Qt::LeftButton) return false;

    // 如果已在变换中，先结束
    if (m_state == STATE_GRABBED) {
        ungrab(true);
    }

    // 检查手柄
    if (m_handleManager) {
        TransformHandle::HandleType handle = m_handleManager->getHandleAtPosition(scenePos);
        if (handle != TransformHandle::None) {
            grab(handle, scenePos, event->modifiers());
            return true;
        }
    }

    // 检查是否点击了图形
    QGraphicsItem *item = m_scene->itemAt(scenePos, QTransform());
    if (item) {
        // 如果点击了图形
        if (event->modifiers() & Qt::ControlModifier) {
            // Ctrl+点击：切换选择状态
            if (item->isSelected()) {
                item->setSelected(false);
            } else {
                item->setSelected(true);
            }
        } else if (!item->isSelected()) {
            // 普通点击：如果图形未被选中，清除其他选择并选中当前图形
            m_scene->clearSelection();
            item->setSelected(true);
        }
        // 如果图形已经被选中且没有按Ctrl，不做操作（可能准备拖动）
        
        // 立即禁用内部选择框
        disableInternalSelectionIndicators();
        
        // 延迟更新手柄
        QTimer::singleShot(10, this, [this]() {
            updateHandlePositions();
        });
    } else {
        // 点击空白区域
        if (!(event->modifiers() & Qt::ControlModifier)) {
            // 没有按Ctrl：清除选择
            m_scene->clearSelection();
        }
        // 确保禁用内部选择框（即使没有选中项）
        disableInternalSelectionIndicators();
        updateHandlePositions();
    }

    // 不消费事件，让场景处理框选
    return false;
}

bool OutlinePreviewTransformTool::mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (!m_scene || !(event->buttons() & Qt::LeftButton)) {
        // 悬停光标
        if (m_handleManager && m_view && m_state == STATE_IDLE) {
            TransformHandle::HandleType handle = m_handleManager->getHandleAtPosition(scenePos);
            m_view->setCursor(handle != TransformHandle::None ? Qt::CrossCursor : Qt::ArrowCursor);
        }
        return false;
    }

    if (m_state == STATE_GRABBED) {
        transform(scenePos, event->modifiers());
        return true;
    }

    return false;
}

bool OutlinePreviewTransformTool::mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos)
{
    Q_UNUSED(scenePos)
    
    if (m_state == STATE_GRABBED) {
        ungrab(true, scenePos); // 应用变换
        return true;
    }
    
    // 框选完成后更新手柄和禁用内部选择框
    if (m_scene) {
        QTimer::singleShot(10, this, [this]() {
            disableInternalSelectionIndicators();
            updateHandlePositions();
        });
    }
    
    return false;
}

bool OutlinePreviewTransformTool::keyPressEvent(QKeyEvent *event)
{
    // ESC 取消变换
    if (event->key() == Qt::Key_Escape && m_state == STATE_GRABBED) {
        ungrab(false); // 不应用，直接取消
        return true;
    }
    return false;
}

// ==================== 核心状态机 ====================

void OutlinePreviewTransformTool::grab(TransformHandle::HandleType handleType, 
                                        const QPointF &mousePos, 
                                        Qt::KeyboardModifiers modifiers)
{
    //qDebug() << "grab() called, handleType:" << handleType << "mousePos:" << mousePos << "current state:" << m_state;
    
    // 如果已经在变换中，先结束当前变换
    if (m_state == STATE_GRABBED) {
        //qDebug() << "Already in GRABBED state, calling ungrab first";
        ungrab(true); // 结束当前变换
    }
    
    m_state = STATE_GRABBED;
    m_activeHandle = handleType;
    m_grabMousePos = mousePos;

    // 获取选中的图形
    QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
    if (selectedItems.isEmpty()) {
        resetState();
        return;
    }
    
    // 使用场景的变换记录机制
    DrawingScene::TransformType transformType = (handleType == TransformHandle::Rotate) ? 
        DrawingScene::Rotate : DrawingScene::Scale;
    m_scene->beginTransform(transformType);
    
    // 创建临时的SelectionGroup
    m_selectionGroup = new QGraphicsItemGroup();
    m_selectionGroup->setVisible(false); // 初始不可见
    m_selectionGroup->setZValue(500); // 设置较低的Z值，避免影响手柄
    m_scene->addItem(m_selectionGroup);
    
    // 将选中的图形添加到Group
    for (QGraphicsItem *item : selectedItems) {
        DrawingShape *shape = dynamic_cast<DrawingShape*>(item);
        if (!shape) continue;
        
        // 从场景移除并添加到Group
        m_scene->removeItem(shape);
        m_selectionGroup->addToGroup(shape);
    }
    
    // 保存Group的原始变换（用于取消变换时恢复）
    m_groupOriginalTransform = m_selectionGroup->transform();
    
    
    
    // 计算基准数据
    m_initialBounds = calculateInitialSelectionBounds();
    m_oppositeHandle = calculateOpposite(m_initialBounds, handleType);
    m_transformOrigin = calculateOrigin(m_initialBounds, m_oppositeHandle, modifiers);
    m_handleBounds = m_initialBounds; // 手柄始终基于初始边界
    
    // ✅ 计算并保存固定的缩放锚点
    QPointF ironPlateCenter = m_initialBounds.center();
    switch (handleType) {
        case TransformHandle::Right:
            m_scaleAnchor = QPointF(m_initialBounds.left(), ironPlateCenter.y());
            break;
        case TransformHandle::Left:
            m_scaleAnchor = QPointF(m_initialBounds.right(), ironPlateCenter.y());
            break;
        case TransformHandle::Bottom:
            m_scaleAnchor = QPointF(ironPlateCenter.x(), m_initialBounds.top());
            break;
        case TransformHandle::Top:
            m_scaleAnchor = QPointF(ironPlateCenter.x(), m_initialBounds.bottom());
            break;
        case TransformHandle::BottomRight:
            m_scaleAnchor = m_initialBounds.topLeft();
            break;
        case TransformHandle::TopLeft:
            m_scaleAnchor = m_initialBounds.bottomRight();
            break;
        case TransformHandle::TopRight:
            m_scaleAnchor = m_initialBounds.bottomLeft();
            break;
        case TransformHandle::BottomLeft:
            m_scaleAnchor = m_initialBounds.topRight();
            break;
        case TransformHandle::Center:
            m_scaleAnchor = m_initialBounds.center();
            break;
        default:
            m_scaleAnchor = m_initialBounds.center();
            break;
    }
    
    // 显示SelectionGroup（准备变换）
    m_selectionGroup->setVisible(true);
    
    
    
    // 创建视觉辅助元素（在计算完所有参数后）
    createVisualHelpers();
    
    // 隐藏手柄，避免干扰预览
    if (m_handleManager) {
        //qDebug() << "Hiding handles for handleType:" << handleType;
        m_handleManager->hideHandles();
        m_handleManager->setActiveHandle(handleType);
    } 
}

void OutlinePreviewTransformTool::transform(const QPointF &mousePos, Qt::KeyboardModifiers modifiers)
{
    if (!m_selectionGroup) {
        return;
    }

    QTransform transform;
    
    // 处理旋转
    if (m_activeHandle == TransformHandle::Rotate) {
        QPointF center = m_transformOrigin;
        qreal initialAngle = qAtan2(m_grabMousePos.y() - center.y(), 
                                      m_grabMousePos.x() - center.x());
        qreal currentAngle = qAtan2(mousePos.y() - center.y(), 
                                     mousePos.x() - center.x());
        qreal rotation = (currentAngle - initialAngle) * 180.0 / M_PI;
        
        
        
        // 场景坐标下的旋转矩阵
        transform.translate(center.x(), center.y());
        transform.rotate(rotation);
        transform.translate(-center.x(), -center.y());
    } else {
        // 处理缩放 - 基于鼠标移动的相对变化
        qreal sx = 1.0, sy = 1.0;
        qreal initialWidth = m_initialBounds.width();
        qreal initialHeight = m_initialBounds.height();
        
        switch (m_activeHandle) {
            case TransformHandle::Right:
                sx = safeDiv(mousePos.x() - m_scaleAnchor.x(), m_grabMousePos.x() - m_scaleAnchor.x());
                sy = 1.0;
                break;
            case TransformHandle::Left:
                sx = safeDiv(m_scaleAnchor.x() - mousePos.x(), m_scaleAnchor.x() - m_grabMousePos.x());
                sy = 1.0;
                break;
            case TransformHandle::Bottom:
                sx = 1.0;
                sy = safeDiv(mousePos.y() - m_scaleAnchor.y(), m_grabMousePos.y() - m_scaleAnchor.y());
                break;
            case TransformHandle::Top:
                sx = 1.0;
                sy = safeDiv(m_scaleAnchor.y() - mousePos.y(), m_scaleAnchor.y() - m_grabMousePos.y());
                break;
            case TransformHandle::BottomRight:
                sx = safeDiv(mousePos.x() - m_scaleAnchor.x(), m_grabMousePos.x() - m_scaleAnchor.x());
                sy = safeDiv(mousePos.y() - m_scaleAnchor.y(), m_grabMousePos.y() - m_scaleAnchor.y());
                break;
            case TransformHandle::TopLeft:
                sx = safeDiv(m_scaleAnchor.x() - mousePos.x(), m_scaleAnchor.x() - m_grabMousePos.x());
                sy = safeDiv(m_scaleAnchor.y() - mousePos.y(), m_scaleAnchor.y() - m_grabMousePos.y());
                break;
            case TransformHandle::TopRight:
                sx = safeDiv(mousePos.x() - m_scaleAnchor.x(), m_grabMousePos.x() - m_scaleAnchor.x());
                sy = safeDiv(m_scaleAnchor.y() - mousePos.y(), m_scaleAnchor.y() - m_grabMousePos.y());
                break;
            case TransformHandle::BottomLeft:
                sx = safeDiv(m_scaleAnchor.x() - mousePos.x(), m_scaleAnchor.x() - m_grabMousePos.x());
                sy = safeDiv(mousePos.y() - m_scaleAnchor.y(), m_grabMousePos.y() - m_scaleAnchor.y());
                break;
            default:
                return;
        }
        
        sx = qBound(-10.0, sx, 10.0);
        sy = qBound(-10.0, sy, 10.0);
        
        
        
        // 🌟 关键简化：直接对Group应用基于锚点的变换
        transform.translate(m_scaleAnchor.x(), m_scaleAnchor.y());
        transform.scale(sx, sy);
        transform.translate(-m_scaleAnchor.x(), -m_scaleAnchor.y());
    }
    
    // 应用到SelectionGroup - Qt会自动处理所有子项的变换
    m_selectionGroup->setTransform(transform);
    
    // 更新视觉辅助元素
    updateVisualHelpers(mousePos);

    if (m_scene) m_scene->update();
}

void OutlinePreviewTransformTool::ungrab(bool apply, const QPointF &finalMousePos)
{
    if (apply) {
        // 先将图形从Group中移除，这会自动将Group的变换烘焙到图形上
        if (m_selectionGroup) {
            QList<QGraphicsItem*> groupItems = m_selectionGroup->childItems();
            
            for (QGraphicsItem *item : groupItems) {
                DrawingShape *shape = dynamic_cast<DrawingShape*>(item);
                if (!shape) continue;
                
                // 从Group中移除（Qt会自动应用Group变换到图形）
                m_selectionGroup->removeFromGroup(shape);
                m_scene->addItem(shape);
                shape->setSelected(true);
            }
            
            m_scene->removeItem(m_selectionGroup);
            delete m_selectionGroup;
            m_selectionGroup = nullptr;
        }
        
    } else {
        // 取消变换
        if (m_selectionGroup) {
            m_selectionGroup->setTransform(m_groupOriginalTransform);
            
            QList<QGraphicsItem*> groupItems = m_selectionGroup->childItems();
            
            for (QGraphicsItem *item : groupItems) {
                DrawingShape *shape = dynamic_cast<DrawingShape*>(item);
                if (!shape) continue;
                
                m_selectionGroup->removeFromGroup(shape);
                m_scene->addItem(shape);
                shape->setSelected(true);
            }
            
            m_scene->removeItem(m_selectionGroup);
            delete m_selectionGroup;
            m_selectionGroup = nullptr;
        }
    }

    destroyVisualHelpers();
    
    resetState();
    
    // 清除活动手柄状态
    if (m_handleManager) {
        
      m_handleManager->setActiveHandle(TransformHandle::None);
    }
    
    // 重新显示手柄
    updateHandlePositions();

    if (m_scene) m_scene->setModified(true);
    
    // 🌟 变换完成后更新标尺显示
    if (m_scene) {
        m_scene->emitSelectionChanged();
    }
}
// ==================== 变换计算 ====================

QTransform OutlinePreviewTransformTool::calculateCurrentTransform(const QPointF &mousePos) const
{
    // 铁板中心（场景坐标）
    QPointF ironPlateCenter = m_initialBounds.center();
    
    // 旋转处理
    if (m_activeHandle == TransformHandle::Rotate) {
        QPointF center = m_transformOrigin;
        qreal initialAngle = qAtan2(m_grabMousePos.y() - center.y(), 
                                      m_grabMousePos.x() - center.x());
        qreal currentAngle = qAtan2(mousePos.y() - center.y(), 
                                     mousePos.x() - center.x());
        qreal rotation = (currentAngle - initialAngle) * 180.0 / M_PI;
        
        // ✅ 场景坐标下的旋转矩阵
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(rotation);
        transform.translate(-center.x(), -center.y());
        return transform;
    }
    
    // 缩放手柄：使用固定的锚点（场景坐标）
    qreal sx = 1.0, sy = 1.0;
    
    switch (m_activeHandle) {
        case TransformHandle::Right:
            sx = safeDiv(mousePos.x() - m_initialBounds.left(), m_initialBounds.width());
            sy = 1.0;
            break;
        case TransformHandle::Left:
            sx = safeDiv(m_initialBounds.right() - mousePos.x(), m_initialBounds.width());
            sy = 1.0;
            break;
        case TransformHandle::Bottom:
            sx = 1.0;
            sy = safeDiv(mousePos.y() - m_initialBounds.top(), m_initialBounds.height());
            break;
        case TransformHandle::Top:
            sx = 1.0;
            sy = safeDiv(m_initialBounds.bottom() - mousePos.y(), m_initialBounds.height());
            break;
        case TransformHandle::BottomRight:
            sx = safeDiv(mousePos.x() - m_initialBounds.left(), m_initialBounds.width());
            sy = safeDiv(mousePos.y() - m_initialBounds.top(), m_initialBounds.height());
            break;
        case TransformHandle::TopLeft:
            sx = safeDiv(m_initialBounds.right() - mousePos.x(), m_initialBounds.width());
            sy = safeDiv(m_initialBounds.bottom() - mousePos.y(), m_initialBounds.height());
            break;
        case TransformHandle::TopRight:
            sx = safeDiv(mousePos.x() - m_initialBounds.left(), m_initialBounds.width());
            sy = safeDiv(m_initialBounds.bottom() - mousePos.y(), m_initialBounds.height());
            break;
        case TransformHandle::BottomLeft:
            sx = safeDiv(m_initialBounds.right() - mousePos.x(), m_initialBounds.width());
            sy = safeDiv(mousePos.y() - m_initialBounds.top(), m_initialBounds.height());
            break;
        default:
            return QTransform();
    }
    
    // 🌟 允许负缩放因子以实现镜像，但限制绝对值避免极端情况
    sx = qBound(-10.0, sx, 10.0);
    sy = qBound(-10.0, sy, 10.0);
    
    // 调试信息
    //qDebug() << "Transform: handle=" << m_activeHandle << "sx=" << sx << "sy=" << sy 
//             << "mousePos=" << mousePos << "initialBounds=" << m_initialBounds;
    
    // ✅ 场景坐标下的缩放矩阵（使用固定锚点）
    QTransform transform;
    transform.translate(m_scaleAnchor.x(), m_scaleAnchor.y());
    transform.scale(sx, sy);
    transform.translate(-m_scaleAnchor.x(), -m_scaleAnchor.y());
    
    return transform;
}

void OutlinePreviewTransformTool::applyFinalTransforms(const QPointF &mousePos)
{
    // QGraphicsItemGroup 已经自动处理了子项的变换
    // 不需要手动应用变换，Group 的变换会自动应用到所有子项上
}

// ==================== 辅助方法 ====================

QPointF OutlinePreviewTransformTool::calculateOpposite(const QRectF &bounds, 
                                                       TransformHandle::HandleType type) const
{
    switch (type) {
        case TransformHandle::TopLeft:     return bounds.bottomRight();
        case TransformHandle::TopRight:    return bounds.bottomLeft();
        case TransformHandle::BottomLeft:  return bounds.topRight();
        case TransformHandle::BottomRight: return bounds.topLeft();
        case TransformHandle::Left:        return QPointF(bounds.right(), bounds.center().y());
        case TransformHandle::Right:       return QPointF(bounds.left(), bounds.center().y());
        case TransformHandle::Top:         return QPointF(bounds.center().x(), bounds.bottom());
        case TransformHandle::Bottom:      return QPointF(bounds.center().x(), bounds.top());
        case TransformHandle::Center:      return bounds.center();
        default: return bounds.center();
    }
}



QPointF OutlinePreviewTransformTool::calculateOrigin(const QRectF &bounds, 
                                                     const QPointF &opposite,
                                                     Qt::KeyboardModifiers modifiers) const
{
    // SHIFT：围绕对角点
    if (modifiers & Qt::ShiftModifier) {
        return opposite;
    }
    // 默认：边界框中心（场景坐标）
    return bounds.center();
}

QRectF OutlinePreviewTransformTool::calculateInitialSelectionBounds() const
{
    // 如果在变换中，直接返回Group的边界
    if (m_state == STATE_GRABBED && m_selectionGroup) {
        return m_selectionGroup->sceneBoundingRect();
    }
    
    // 否则从场景获取当前选中的对象
    QRectF bounds;
    bool first = true;
    
    if (m_scene) {
        QList<QGraphicsItem*> items = m_scene->selectedItems();
        for (QGraphicsItem *item : items) {
            DrawingShape *shape = dynamic_cast<DrawingShape*>(item);
            if (!shape) continue;
            QRectF sceneBounds = shape->sceneBoundingRect();
            if (first) {
                bounds = sceneBounds;
                first = false;
            } else {
                bounds = bounds.united(sceneBounds);
            }
        }
    }
    return bounds;
}

void OutlinePreviewTransformTool::onSelectionChanged()
{
    // 更新UI
    disableInternalSelectionIndicators();
    
    // 延迟更新手柄，确保选择状态完全更新
    QTimer::singleShot(0, this, [this]() {
        updateHandlePositions();
    });
}

void OutlinePreviewTransformTool::onObjectStateChanged(DrawingShape* shape)
{
    // 如果图形当前被选中，更新手柄
    if (shape && shape->isSelected()) {
        updateHandlePositions();
    }
}

void OutlinePreviewTransformTool::updateDashOffset()
{
    if (!m_outlinePreview) return;
    
    // 动态改变虚线偏移，实现蚂蚁线移动效果
    QPen pen = m_outlinePreview->pen();
    qreal dashOffset = pen.dashOffset();
    pen.setDashOffset(dashOffset + 0.5);
    m_outlinePreview->setPen(pen);
}

void OutlinePreviewTransformTool::updateHandlePositions()
{
    //qDebug() << "updateHandlePositions() called, m_handleManager:" << m_handleManager;
    
    if (!m_handleManager) {
        return;
    }
    
    QRectF bounds = calculateInitialSelectionBounds();
    //qDebug() << "updateHandlePositions() bounds:" << bounds << "isEmpty:" << bounds.isEmpty();
    
    // 如果有选中的图形，就显示手柄
    if (bounds.isEmpty()) {
        //qDebug() << "Hiding handles - no selection";
        m_handleManager->hideHandles();
    } else {
        // 手柄基于边界
        //qDebug() << "Updating handles with bounds:" << bounds;
        m_handleManager->updateHandles(bounds);
        //qDebug() << "Showing handles";
        if (m_state != STATE_GRABBED)
            m_handleManager->showHandles();
    }
}

void OutlinePreviewTransformTool::resetState()
{
    m_state = STATE_IDLE;
    m_activeHandle = TransformHandle::None;
    m_grabMousePos = QPointF();
    m_initialBounds = QRectF();
    m_oppositeHandle = QPointF();
    m_transformOrigin = QPointF();
    m_handleBounds = QRectF();
    
    
    
    // SelectionGroup的变换保持不变，不要重置
}

void OutlinePreviewTransformTool::createVisualHelpers()
{
    if (!m_scene) return;
    
    // 创建锚点（红色）- 显示在变换中心位置
    m_anchorPoint = new QGraphicsEllipseItem(-4, -4, 8, 8);
    m_anchorPoint->setBrush(QBrush(Qt::red));
    m_anchorPoint->setPen(QPen(Qt::darkRed, 1));
    m_anchorPoint->setZValue(2001);
    m_scene->addItem(m_anchorPoint);
    m_anchorPoint->setPos(m_transformOrigin);
    
    // 创建拖动点（绿色）
    m_dragPoint = new QGraphicsEllipseItem(-4, -4, 8, 8);
    m_dragPoint->setBrush(QBrush(Qt::green));
    m_dragPoint->setPen(QPen(Qt::darkGreen, 1));
    m_dragPoint->setZValue(2001);
    m_scene->addItem(m_dragPoint);
    
    // 创建整体轮廓预览
    m_outlinePreview = new QGraphicsPathItem();
    
    // 创建蚂蚁线画笔（黑白相间的虚线）
    QPen outlinePen(Qt::black, 2);
    outlinePen.setCosmetic(true); // 不受缩放影响
    outlinePen.setDashPattern({4, 2}); // 虚线模式
    m_outlinePreview->setPen(outlinePen);
    m_outlinePreview->setBrush(Qt::NoBrush);
    m_outlinePreview->setZValue(1999);
    m_scene->addItem(m_outlinePreview);
    
    
    
    // 创建动画定时器，用于实现蚂蚁线效果
    m_dashTimer = new QTimer(this);
    connect(m_dashTimer, SIGNAL(timeout()), this, SLOT(updateDashOffset()));
    m_dashTimer->start(80); // 每80ms更新一次，更快的动画
    
    // 构建整体轮廓
    updateOutlinePreview();
}

void OutlinePreviewTransformTool::destroyVisualHelpers()
{
    if (m_anchorPoint) {
        m_scene->removeItem(m_anchorPoint);
        delete m_anchorPoint;
        m_anchorPoint = nullptr;
    }
    
    if (m_dragPoint) {
        m_scene->removeItem(m_dragPoint);
        delete m_dragPoint;
        m_dragPoint = nullptr;
    }
    
    if (m_outlinePreview) {
        m_scene->removeItem(m_outlinePreview);
        delete m_outlinePreview;
        m_outlinePreview = nullptr;
    }
    
    if (m_dashTimer) {
        m_dashTimer->stop();
        delete m_dashTimer;
        m_dashTimer = nullptr;
    }
}

void OutlinePreviewTransformTool::updateVisualHelpers(const QPointF &mousePos)
{
    if (!m_scene) return;
    
    // 更新拖动点位置
    if (m_dragPoint) {
        m_dragPoint->setPos(mousePos);
    }
    
    // 确保锚点可见（特别是旋转时）
    if (m_anchorPoint) {
        m_anchorPoint->setVisible(true);
        
        m_anchorPoint->setPos(m_scaleAnchor);
    }
    
    // 更新整体轮廓
    updateOutlinePreview();
}

void OutlinePreviewTransformTool::updateOutlinePreview()
{
    if (!m_outlinePreview) {
        return;
    }
    
    // 构建轮廓
    QPainterPath combinedPath;
    
    // 如果在变换中，直接使用Group的边界
    if (m_state == STATE_GRABBED && m_selectionGroup) {
        QRectF groupBounds = m_selectionGroup->sceneBoundingRect();
        combinedPath.addRect(groupBounds);
    } else if (m_scene) {
        // 否则从场景获取选中的图形
        QList<QGraphicsItem*> items = m_scene->selectedItems();
        for (QGraphicsItem *item : items) {
            DrawingShape *shape = dynamic_cast<DrawingShape*>(item);
            if (!shape) continue;
            
            QRectF sceneBounds = shape->sceneBoundingRect();
            QPainterPath path;
            path.addRect(sceneBounds);
            
            if (combinedPath.isEmpty()) {
                combinedPath = path;
            } else {
                combinedPath = combinedPath.united(path);
            }
        }
    }
    
    m_outlinePreview->setPath(combinedPath);
}

void OutlinePreviewTransformTool::disableInternalSelectionIndicators()
{
    if (!m_scene) return;
    
    QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
    for (QGraphicsItem *item : selectedItems) {
        DrawingShape *shape = dynamic_cast<DrawingShape*>(item);
        if (shape) {
            shape->setShowSelectionIndicator(false);
        }
    }
}

void OutlinePreviewTransformTool::enableInternalSelectionIndicators()
{
    if (!m_scene) return;
    
    QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
    for (QGraphicsItem *item : selectedItems) {
        DrawingShape *shape = dynamic_cast<DrawingShape*>(item);
        if (shape) {
            shape->setShowSelectionIndicator(true);
        }
    }
}