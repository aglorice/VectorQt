#include "cursor-manager.h"
#include <QWidget>
#include <QApplication>
#include <QHash>
#include <QPainter>
#include <QPen>
#include <QPainterPath>

CursorManager& CursorManager::instance()
{
    static CursorManager instance;
    return instance;
}

CursorManager::CursorManager()
    : m_initialized(false)
{
    createCursors();
}

QCursor CursorManager::getCursor(CursorType type)
{
    if (!m_initialized) {
        createCursors();
    }
    
    return m_cursors.value(type, Qt::ArrowCursor);
}

void CursorManager::setCursorForView(QWidget *view, CursorType type)
{
    if (view) {
        view->setCursor(getCursor(type));
    }
}

void CursorManager::createCursors()
{
    if (m_initialized) {
        return;
    }
    
    int cursorSize = 32;
    int hotSpotX = cursorSize / 4; // 十字线中心位置（左上角）
    int hotSpotY = cursorSize / 4;
    
    // 创建各种工具的光标，设置正确的热点
    m_cursors[SelectCursor] = QCursor(Qt::ArrowCursor);
    m_cursors[RectangleCursor] = QCursor(createCrosshairWithShape(RectangleCursor), hotSpotX, hotSpotY);
    m_cursors[EllipseCursor] = QCursor(createCrosshairWithShape(EllipseCursor), hotSpotX, hotSpotY);
    m_cursors[LineCursor] = QCursor(createCrosshairWithShape(LineCursor), hotSpotX, hotSpotY);
    m_cursors[BezierCursor] = QCursor(createCrosshairWithShape(BezierCursor), hotSpotX, hotSpotY);
    m_cursors[PolygonCursor] = QCursor(createCrosshairWithShape(PolygonCursor), hotSpotX, hotSpotY);
    m_cursors[PolylineCursor] = QCursor(createCrosshairWithShape(PolylineCursor), hotSpotX, hotSpotY);
    m_cursors[BrushCursor] = QCursor(Qt::CrossCursor);
    m_cursors[FillCursor] = QCursor(Qt::PointingHandCursor);
    m_cursors[NodeEditCursor] = QCursor(Qt::CrossCursor);
    m_cursors[PathEditCursor] = QCursor(createCrosshairWithShape(PathEditCursor), hotSpotX, hotSpotY);
    m_cursors[DefaultCursor] = QCursor(Qt::ArrowCursor);
    
    m_initialized = true;
}

QPixmap CursorManager::createCrosshairCursor(int size)
{
    // 创建透明背景的图像
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 设置细十字线样式
    QPen pen(Qt::black, 1);
    pen.setCosmetic(true); // 确保线宽不受变换影响
    painter.setPen(pen);
    
    int center = size / 2;
    int crossSize = size / 3; // 十字线更小
    
    // 绘制细十字线（在左上角区域）
    painter.drawLine(center - crossSize/2, center, center + crossSize/2, center);        // 水平线
    painter.drawLine(center, center - crossSize/2, center, center + crossSize/2);        // 垂直线
    
    // 在十字中心绘制一个小点
    painter.setPen(QPen(Qt::black, 1));
    painter.drawPoint(center, center);
    
    return pixmap;
}

QPixmap CursorManager::createCrosshairWithShape(CursorType type, int size)
{
    // 创建透明背景的图像
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int crossSize = size / 4; // 十字线大小
    int shapeSize = size / 2; // 放大图形尺寸
    
    // 绘制细十字线（在左上角区域）
    QPen crossPen(Qt::black, 1);
    crossPen.setCosmetic(true);
    painter.setPen(crossPen);
    
    // 十字线在左上角区域
    int crossCenterX = crossSize;
    int crossCenterY = crossSize;
    painter.drawLine(crossCenterX - crossSize/2, crossCenterY, crossCenterX + crossSize/2, crossCenterY);
    painter.drawLine(crossCenterX, crossCenterY - crossSize/2, crossCenterX, crossCenterY + crossSize/2);
    
    // 十字中心点
    painter.drawPoint(crossCenterX, crossCenterY);
    
    // 形状在右下角区域，使用更多空间
    int shapeCenterX = size - shapeSize/2 - 2;
    int shapeCenterY = size - shapeSize/2 - 2;
    
    // 设置形状画笔样式
    QPen pen(Qt::black, 1);
    pen.setCosmetic(true);
    painter.setPen(pen);
    
    // 根据工具类型绘制不同的形状
    switch (type) {
    case RectangleCursor:
        // 绘制矩形
        painter.drawRect(shapeCenterX - shapeSize/2, shapeCenterY - shapeSize/2, shapeSize, shapeSize);
        break;
        
    case EllipseCursor:
        // 绘制椭圆
        painter.drawEllipse(QPoint(shapeCenterX, shapeCenterY), shapeSize/2, shapeSize/2);
        break;
        
    case LineCursor:
        // 绘制线条
        painter.drawLine(shapeCenterX - shapeSize/2, shapeCenterY, shapeCenterX + shapeSize/2, shapeCenterY);
        break;
        
    case BezierCursor:
        // 绘制曲线（贝塞尔曲线）
        {
            QPainterPath path;
            path.moveTo(shapeCenterX - shapeSize/2, shapeCenterY);
            path.cubicTo(shapeCenterX - shapeSize/4, shapeCenterY - shapeSize/4,
                         shapeCenterX + shapeSize/4, shapeCenterY + shapeSize/4,
                         shapeCenterX + shapeSize/2, shapeCenterY);
            painter.drawPath(path);
        }
        break;
        
    case PolygonCursor:
    case PolylineCursor:
        // 绘制三角形（代表多边形）
        {
            QPolygonF triangle;
            triangle << QPointF(shapeCenterX, shapeCenterY - shapeSize/2)
                     << QPointF(shapeCenterX - shapeSize/2, shapeCenterY + shapeSize/2)
                     << QPointF(shapeCenterX + shapeSize/2, shapeCenterY + shapeSize/2);
            painter.drawPolygon(triangle);
        }
        break;
        
    case BrushCursor:
        // 绘制画笔
        {
            // 画笔主体
            painter.drawLine(shapeCenterX, shapeCenterY + shapeSize/2, shapeCenterX, shapeCenterY - shapeSize/2);
            // 画笔尖端
            painter.drawEllipse(QPoint(shapeCenterX, shapeCenterY - shapeSize/2), 2, 2);
        }
        break;
        
    case PathEditCursor:
        // 绘制节点编辑器的方块
        painter.drawRect(shapeCenterX - 3, shapeCenterY - 3, 6, 6);
        break;
        
    case SelectCursor:
    case FillCursor:
    case NodeEditCursor:
    case DefaultCursor:
        // 这些光标使用系统默认光标，不需要绘制形状
        break;
        
    default:
        // 未知类型，绘制一个简单的十字
        break;
    }
    
    return pixmap;
}