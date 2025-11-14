#include "drawing-tool-brush.h"
#include "drawingscene.h"
#include "drawingview.h"
#include "drawing-shape.h"
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPen>
#include <QDebug>

DrawingToolBrush::DrawingToolBrush(QObject *parent)
    : ToolBase(parent)
    , m_currentPath(nullptr)
    , m_brushWidth(2.0)
    , m_smoothness(0.5)
    , m_drawing(false)
{
}

void DrawingToolBrush::activate(DrawingScene *scene, DrawingView *view)
{
    ToolBase::activate(scene, view);
    m_currentPath = nullptr;
    m_points.clear();
    m_drawing = false;
}

void DrawingToolBrush::deactivate()
{
    if (m_currentPath) {
        if (m_scene) {
            m_scene->removeItem(m_currentPath);
        }
        delete m_currentPath;
        m_currentPath = nullptr;
    }
    ToolBase::deactivate();
}

bool DrawingToolBrush::mousePressEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (event->button() == Qt::LeftButton && m_scene) {
        m_drawing = true;
        m_lastPoint = scenePos;
        m_points.clear();
        m_points.append(scenePos);
        
        // 创建新的路径
        m_currentPath = new DrawingPath();
        m_currentPath->setPos(0, 0);
        
        // 设置画笔样式
        QPen pen(Qt::black);
        pen.setWidth(m_brushWidth);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        m_currentPath->setStrokePen(pen);
        m_currentPath->setFillBrush(Qt::NoBrush);
        
        // 添加到场景
        m_scene->addItem(m_currentPath);
        m_scene->clearSelection();
        //m_currentPath->setSelected(true);
        
        return true;
    }
    
    return false;
}

bool DrawingToolBrush::mouseMoveEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (m_drawing && m_currentPath && m_scene) {
        // 计算与上一个点的距离
        qreal distance = QLineF(m_lastPoint, scenePos).length();
        
        // 只有当移动距离足够大时才添加点（避免过多的点）
        if (distance > 2.0) {
            m_points.append(scenePos);
            m_lastPoint = scenePos;
            
            // 如果点数足够，进行平滑处理
            if (m_points.size() > 2 && m_smoothness > 0) {
                QVector<QPointF> smoothedPoints = smoothPath(m_points);
                
                // 创建平滑的路径
                QPainterPath path;
                if (smoothedPoints.size() > 0) {
                    path.moveTo(smoothedPoints[0]);
                    for (int i = 1; i < smoothedPoints.size(); ++i) {
                        path.lineTo(smoothedPoints[i]);
                    }
                }
                
                m_currentPath->setPath(path);
            } else {
                // 直接使用原始点
                QPainterPath path;
                if (m_points.size() > 0) {
                    path.moveTo(m_points[0]);
                    for (int i = 1; i < m_points.size(); ++i) {
                        path.lineTo(m_points[i]);
                    }
                }
                
                m_currentPath->setPath(path);
            }
        }
        
        return true;
    }
    
    return false;
}

bool DrawingToolBrush::mouseReleaseEvent(QMouseEvent *event, const QPointF &scenePos)
{
    if (event->button() == Qt::LeftButton && m_drawing) {
        m_drawing = false;
        
        // 完成路径绘制
        if (m_currentPath && m_points.size() > 1) {
            // 最后的平滑处理
            if (m_smoothness > 0 && m_points.size() > 2) {
                QVector<QPointF> smoothedPoints = smoothPath(m_points);
                
                QPainterPath path;
                path.moveTo(smoothedPoints[0]);
                for (int i = 1; i < smoothedPoints.size(); ++i) {
                    path.lineTo(smoothedPoints[i]);
                }
                
                m_currentPath->setPath(path);
            }
            
            // 设置控制点用于后续编辑
            m_currentPath->setControlPoints(m_points);
            
            m_currentPath = nullptr; // 不删除，让场景管理
        } else if (m_currentPath) {
            // 点太少，删除路径
            if (m_scene) {
                m_scene->removeItem(m_currentPath);
            }
            delete m_currentPath;
            m_currentPath = nullptr;
        }
        
        m_points.clear();
        
        return true;
    }
    
    return false;
}

QVector<QPointF> DrawingToolBrush::smoothPath(const QVector<QPointF> &points)
{
    if (points.size() < 3) {
        return points;
    }
    
    QVector<QPointF> smoothedPoints;
    smoothedPoints.append(points[0]);
    
    // 使用简单的加权平均进行平滑
    for (int i = 1; i < points.size() - 1; ++i) {
        QPointF prev = points[i - 1];
        QPointF curr = points[i];
        QPointF next = points[i + 1];
        
        // 加权平均：当前点权重更高
        QPointF smoothed = prev * (1 - m_smoothness) / 2 + 
                          curr * m_smoothness + 
                          next * (1 - m_smoothness) / 2;
        
        smoothedPoints.append(smoothed);
    }
    
    smoothedPoints.append(points.last());
    
    return smoothedPoints;
}

#include "drawing-tool-brush.moc"