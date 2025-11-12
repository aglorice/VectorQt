#ifndef DRAWINGSCENE_H
#define DRAWINGSCENE_H

#include <QGraphicsScene>
#include <QUndoStack>

class DrawingShape;
class DrawingGroup;
class SelectionLayer;

class DrawingScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit DrawingScene(QObject *parent = nullptr);
    
    QUndoStack* undoStack() { return &m_undoStack; }
    
    bool isModified() const { return m_isModified; }
    void setModified(bool modified);
    
    void clearScene();
    
    // 选择层管理
    SelectionLayer* selectionLayer() const { return m_selectionLayer; }
    void updateSelection();
    
    // 激活/停用选择工具时调用
    void activateSelectionTool();
    void deactivateSelectionTool();
    
    // 网格功能
    void setGridVisible(bool visible);
    bool isGridVisible() const;
    void setGridSize(int size);
    int gridSize() const;
    void setGridColor(const QColor &color);
    QColor gridColor() const;
    
    // 网格对齐功能
    QPointF alignToGrid(const QPointF &pos) const;
    QPointF alignToGrid(const QPointF &pos, DrawingShape *excludeShape) const;
    QRectF alignToGrid(const QRectF &rect) const;
    
    // 🌟 智能吸附功能 - 只在接近网格线一定距离时才吸附
    struct SnapResult {
        QPointF snappedPos;
        bool snappedX;
        bool snappedY;
        SnapResult() : snappedX(false), snappedY(false) {}
    };
    SnapResult smartAlignToGrid(const QPointF &pos) const;
    
    // 网格对齐开关
    void setGridAlignmentEnabled(bool enabled);
    bool isGridAlignmentEnabled() const;
    
    // 🌟 智能吸附设置
    void setSnapEnabled(bool enabled);
    bool isSnapEnabled() const;
    void setSnapTolerance(int tolerance);
    int snapTolerance() const;
    
    // 🌟 参考线系统
    struct Guide {
        Qt::Orientation orientation;
        qreal position;  // 场景坐标中的位置
        QColor color;
        bool visible;
        
        Guide(Qt::Orientation orient, qreal pos, QColor col = QColor(0, 120, 255, 150))
            : orientation(orient), position(pos), color(col), visible(true) {}
    };
    
    void addGuide(Qt::Orientation orientation, qreal position);
    void removeGuide(Qt::Orientation orientation, qreal position);
    void clearGuides();
    QList<Guide> guides() const { return m_guides; }
    void setGuideVisible(Qt::Orientation orientation, qreal position, bool visible);
    
    // 🌟 参考线吸附功能
    struct GuideSnapResult {
        QPointF snappedPos;
        bool snappedToGuide;
        Qt::Orientation snapOrientation;
        qreal guidePosition;
        
        GuideSnapResult() : snappedToGuide(false), snapOrientation(Qt::Horizontal), guidePosition(0) {}
    };
    GuideSnapResult snapToGuides(const QPointF &pos) const;
    
    // 🌟 对象吸附系统
    enum ObjectSnapType {
        SnapToLeft,      // 吸附到左边
        SnapToRight,     // 吸附到右边
        SnapToTop,       // 吸附到上边
        SnapToBottom,    // 吸附到下边
        SnapToCenterX,   // 吸附到水平中心
        SnapToCenterY,   // 吸附到垂直中心
        SnapToCorner     // 吸附到角点
    };
    
    struct ObjectSnapPoint {
        QPointF position;    // 场景坐标位置
        ObjectSnapType type; // 吸附点类型
        DrawingShape* shape; // 来源图形
        
        ObjectSnapPoint(const QPointF &pos, ObjectSnapType t, DrawingShape* s)
            : position(pos), type(t), shape(s) {}
    };
    
    struct ObjectSnapResult {
        QPointF snappedPos;
        bool snappedToObject;
        ObjectSnapType snapType;
        DrawingShape* targetShape;
        QString snapDescription; // 吸附描述，用于显示
        
        ObjectSnapResult() : snappedToObject(false), snapType(SnapToLeft), targetShape(nullptr) {}
    };
    
    // 对象吸附功能
    ObjectSnapResult snapToObjects(const QPointF &pos, DrawingShape *excludeShape = nullptr);
    QList<ObjectSnapPoint> getObjectSnapPoints(DrawingShape *excludeShape = nullptr) const;
    
    // 对象吸附开关
    void setObjectSnapEnabled(bool enabled);
    bool isObjectSnapEnabled() const;
    void setObjectSnapTolerance(int tolerance);
    int objectSnapTolerance() const;
    
    // 🌟 对象吸附视觉反馈
    void showSnapIndicators(const ObjectSnapResult &snapResult);
    void clearSnapIndicators();
    void clearExpiredSnapIndicators(const QPointF &currentPos);
    void setSnapIndicatorsVisible(bool visible);
    bool areSnapIndicatorsVisible() const;

signals:
    void sceneModified(bool modified);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private slots:
    void onSelectionChanged();

private:
    void drawGrid(QPainter *painter, const QRectF &rect);
    
    QUndoStack m_undoStack;
    bool m_isModified;
    SelectionLayer *m_selectionLayer;
    
    // 网格相关
    bool m_gridVisible;
    bool m_gridAlignmentEnabled;  // 新增：网格对齐开关
    int m_gridSize;
    QColor m_gridColor;
    
    // 🌟 智能吸附相关
    bool m_snapEnabled;
    int m_snapTolerance;
    bool m_objectSnapEnabled;
    int m_objectSnapTolerance;
    bool m_snapIndicatorsVisible;
    
    // 🌟 参考线系统
    bool m_guidesEnabled;
    QList<Guide> m_guides;
};

#endif // DRAWINGSCENE_H
