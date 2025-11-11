#ifndef DRAWING_DOCUMENT_H
#define DRAWING_DOCUMENT_H

#include <QObject>
#include <vector>
#include <QRectF>
#include <QUndoStack>


class DrawingShape;
class CommandBase;
class RemoveItemCommand;

class DrawingDocument : public QObject
{
    Q_OBJECT

    friend class CommandBase;
    friend class RemoveItemCommand;

public:
    explicit DrawingDocument(QObject *parent = nullptr);
    ~DrawingDocument();

    void addItem(DrawingShape *item);
    void removeItem(DrawingShape *item);
    std::vector<DrawingShape*> items() const;
    
    void clear();
    QRectF bounds() const;
    
    QUndoStack* undoStack() { return &m_undoStack; }
    
    void executeCommand(std::unique_ptr<CommandBase> command);

signals:
    void itemAdded(DrawingShape *item);
    void itemRemoved(DrawingShape *item);
    void documentChanged();

private:
    std::vector<DrawingShape*> m_items;
    QUndoStack m_undoStack;
};

class CommandBase : public QUndoCommand
{
public:
    explicit CommandBase(DrawingDocument *document, QUndoCommand *parent = nullptr)
        : QUndoCommand(parent), m_document(document) {}
    
    virtual ~CommandBase() = default;

protected:
    DrawingDocument *m_document;
};

class AddItemCommand : public CommandBase
{
public:
    AddItemCommand(DrawingDocument *document, DrawingShape *item, QUndoCommand *parent = nullptr);
    
    void undo() override;
    void redo() override;

private:
    DrawingShape *m_item;
};

class RemoveItemCommand : public CommandBase
{
public:
    RemoveItemCommand(DrawingDocument *document, DrawingShape *item, QUndoCommand *parent = nullptr);
    
    void undo() override;
    void redo() override;

private:
    DrawingShape *m_item;
    DrawingShape *m_itemPtr; // Temporary pointer for constructor
    int m_index;
};

#endif // DRAWING_DOCUMENT_H