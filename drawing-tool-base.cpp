#include "drawing-tool-base.h"
#include "drawing-shape.h"
#include "drawingview.h"
#include "drawingscene.h"
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QBrush>
#include <QDebug>

// DrawingToolBase
DrawingToolBase::DrawingToolBase(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_active(false)
    , m_drawing(false)
{
}

void DrawingToolBase::activate(DrawingScene *scene, DrawingView *view)
{
    m_scene = scene;
    m_view = view;
    m_active = true;
}

void DrawingToolBase::deactivate()
{
    // 取消当前绘制
    if (m_drawing) {
        cancelShape();
    }
    
    m_scene = nullptr;
    m_view = nullptr;
    m_active = false;
}

bool DrawingToolBase::mousePressEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (event->button() == Qt::LeftButton && m_scene) {
        m_drawing = true;
        m_startPos = scenePos;
        
        // 清除之前的选择
        m_scene->clearSelection();
        
        // 创建新形状
        m_currentShape = createShape(scenePos);
        if (m_currentShape) {
            m_scene->addItem(m_currentShape);
            m_currentShape->setSelected(true);
        }
        
        return true;
    }
    return false;
}

bool DrawingToolBase::mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (m_drawing && m_currentShape) {
        updateShape(m_startPos, scenePos);
        return true;
    }
    return false;
}

bool DrawingToolBase::mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (event->button() == Qt::LeftButton && m_drawing) {
        m_drawing = false;
        finishShape();
        return true;
    }
    return false;
}

void DrawingToolBase::finishShape()
{
    if (m_currentShape) {
        // 检查形状是否太小
        QRectF bounds = m_currentShape->boundingRect();
        if (bounds.width() < 5 && bounds.height() < 5) {
            // 太小，删除
            cancelShape();
        } else {
            // 保留形状
            if (m_scene) {
                m_scene->setModified(true);
            }
            m_currentShape = nullptr;  // 形状所有权已转移给场景
        }
    }
}

void DrawingToolBase::cancelShape()
{
    if (m_currentShape && m_scene) {
        m_scene->removeItem(m_currentShape);
    }
    m_currentShape = nullptr;
}

// RectangleTool
RectangleTool::RectangleTool(QObject *parent)
    : DrawingToolBase(parent)
{
}

DrawingShape* RectangleTool::createShape(const QPointF &pos)
{
    auto rect = new DrawingRectangle();
    rect->setRectangle(QRectF(0, 0, 1, 1));  // 创建1x1的矩形
    rect->setPos(pos);  // 设置位置
    rect->setFillBrush(QBrush(Qt::yellow));
    rect->setStrokePen(QPen(Qt::black, 2));
    return rect;
}

void RectangleTool::updateShape(const QPointF &startPos, const QPointF &currentPos)
{
    if (!m_currentShape) return;
    
    auto *rect = static_cast<DrawingRectangle*>(m_currentShape);
    QPointF delta = currentPos - startPos;
    
    // 计算矩形几何
    QRectF newRect;
    QPointF newPos;
    
    if (delta.x() >= 0 && delta.y() >= 0) {
        // 右下拖动
        newRect = QRectF(0, 0, qMax(delta.x(), 1.0), qMax(delta.y(), 1.0));
        newPos = startPos;
    } else if (delta.x() < 0 && delta.y() >= 0) {
        // 左下拖动
        newRect = QRectF(0, 0, qAbs(delta.x()), qMax(delta.y(), 1.0));
        newPos = QPointF(currentPos.x(), startPos.y());
    } else if (delta.x() >= 0 && delta.y() < 0) {
        // 右上拖动
        newRect = QRectF(0, 0, qMax(delta.x(), 1.0), qAbs(delta.y()));
        newPos = QPointF(startPos.x(), currentPos.y());
    } else {
        // 左上拖动
        newRect = QRectF(0, 0, qAbs(delta.x()), qAbs(delta.y()));
        newPos = currentPos;
    }
    
    rect->setRectangle(newRect);
    rect->setPos(newPos);
}

// EllipseTool
EllipseTool::EllipseTool(QObject *parent)
    : DrawingToolBase(parent)
{
}

DrawingShape* EllipseTool::createShape(const QPointF &pos)
{
    auto ellipse = new DrawingEllipse();
    ellipse->setEllipse(QRectF(0, 0, 1, 1));  // 创建1x1的椭圆
    ellipse->setPos(pos);  // 设置位置
    ellipse->setFillBrush(QBrush(Qt::cyan));
    ellipse->setStrokePen(QPen(Qt::black, 2));
    return ellipse;
}

void EllipseTool::updateShape(const QPointF &startPos, const QPointF &currentPos)
{
    if (!m_currentShape) return;
    
    auto *ellipse = static_cast<DrawingEllipse*>(m_currentShape);
    QPointF delta = currentPos - startPos;
    
    // 计算椭圆几何（与矩形相同逻辑）
    QRectF newRect;
    QPointF newPos;
    
    if (delta.x() >= 0 && delta.y() >= 0) {
        newRect = QRectF(0, 0, qMax(delta.x(), 1.0), qMax(delta.y(), 1.0));
        newPos = startPos;
    } else if (delta.x() < 0 && delta.y() >= 0) {
        newRect = QRectF(0, 0, qAbs(delta.x()), qMax(delta.y(), 1.0));
        newPos = QPointF(currentPos.x(), startPos.y());
    } else if (delta.x() >= 0 && delta.y() < 0) {
        newRect = QRectF(0, 0, qMax(delta.x(), 1.0), qAbs(delta.y()));
        newPos = QPointF(startPos.x(), currentPos.y());
    } else {
        newRect = QRectF(0, 0, qAbs(delta.x()), qAbs(delta.y()));
        newPos = currentPos;
    }
    
    ellipse->setEllipse(newRect);
    ellipse->setPos(newPos);
}

// SelectTool
SelectTool::SelectTool(QObject *parent)
    : DrawingToolBase(parent)
{
}

void SelectTool::activate(DrawingScene *scene, DrawingView *view)
{
    DrawingToolBase::activate(scene, view);
    if (view) {
        view->setDragMode(QGraphicsView::RubberBandDrag);
    }
    if (scene) {
        scene->activateSelectionTool();
    }
}

void SelectTool::deactivate()
{
    if (m_view) {
        m_view->setDragMode(QGraphicsView::NoDrag);
    }
    DrawingToolBase::deactivate();
}

bool SelectTool::mousePressEvent(QMouseEvent *event, const QPointF &scenePos)
{
    // 选择工具不创建形状，让场景处理选择和拖动
    return false;
}

bool SelectTool::mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos)
{
    // 选择工具不创建形状，让场景处理移动
    return false;
}

bool SelectTool::mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos)
{
    // 选择工具不创建形状，让场景处理释放
    return false;
}

DrawingShape* SelectTool::createShape(const QPointF &pos)
{
    // 选择工具不创建形状
    return nullptr;
}

void SelectTool::updateShape(const QPointF &startPos, const QPointF &currentPos)
{
    // 选择工具不更新形状
}

#include "drawing-tool-base.moc"