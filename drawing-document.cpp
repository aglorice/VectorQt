#include "drawing-document.h"
#include "drawing-shape.h"
#include <QDebug>

DrawingDocument::DrawingDocument(QObject *parent)
    : QObject(parent)
{
}

DrawingDocument::~DrawingDocument()
{
    clear();
}

void DrawingDocument::addItem(DrawingShape *item)
{
    if (!item) {
        return;
    }
    
    m_items.push_back(item);
    item->setDocument(this);
    
    emit itemAdded(item);
    emit documentChanged();
}

void DrawingDocument::removeItem(DrawingShape *item)
{
    if (!item) {
        return;
    }
    
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        if (*it == item) {
            item->setDocument(nullptr);
            m_items.erase(it);
            emit itemRemoved(item);
            emit documentChanged();
            break;
        }
    }
}

std::vector<DrawingShape*> DrawingDocument::items() const
{
    return m_items;
}

void DrawingDocument::clear()
{
    for (DrawingShape *item : m_items) {
        if (item) {
            item->setDocument(nullptr);
            emit itemRemoved(item);
        }
    }
    m_items.clear();
    emit documentChanged();
}

QRectF DrawingDocument::bounds() const
{
    QRectF result;
    for (DrawingShape *item : m_items) {
        if (item) {
            if (result.isEmpty()) {
                result = item->boundingRect();
            } else {
                result |= item->boundingRect();
            }
        }
    }
    return result;
}

void DrawingDocument::executeCommand(std::unique_ptr<CommandBase> command)
{
    if (command) {
        command->redo();
        m_undoStack.push(command.release());
    }
}

// AddItemCommand
AddItemCommand::AddItemCommand(DrawingDocument *document, DrawingShape *item, QUndoCommand *parent)
    : CommandBase(document, parent)
    , m_item(item)
{
}

void AddItemCommand::undo()
{
    if (m_item && m_document) {
        m_document->removeItem(m_item);
    }
}

void AddItemCommand::redo()
{
    if (m_item && m_document) {
        m_document->addItem(m_item);
    }
}

// RemoveItemCommand
RemoveItemCommand::RemoveItemCommand(DrawingDocument *document, DrawingShape *item, QUndoCommand *parent)
    : CommandBase(document, parent)
    , m_item(item)
    , m_index(-1)
{
    if (m_document && m_item) {
        // Find the index of the item
        const auto items = m_document->items();
        m_index = -1;
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i] == m_item) {
                m_index = static_cast<int>(i);
                break;
            }
        }
    }
}

void RemoveItemCommand::undo()
{
    if (m_item && m_document) {
        // Re-add the item at its original position
        if (m_index >= 0 && m_index <= static_cast<int>(m_document->m_items.size())) {
            m_document->m_items.insert(m_document->m_items.begin() + m_index, m_item);
        } else {
            m_document->m_items.push_back(m_item);
        }
        m_item->setDocument(m_document);
        emit m_document->itemAdded(m_item);
        emit m_document->documentChanged();
    }
}

void RemoveItemCommand::redo()
{
    if (m_item && m_document) {
        // Remove the item
        m_document->removeItem(m_item);
    }
}

