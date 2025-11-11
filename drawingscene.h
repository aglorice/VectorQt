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
    QRectF alignToGrid(const QRectF &rect) const;
    
    // 网格对齐开关
    void setGridAlignmentEnabled(bool enabled);
    bool isGridAlignmentEnabled() const;

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
};

#endif // DRAWINGSCENE_H
