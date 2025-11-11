#ifndef QDRAWPRO_H
#define QDRAWPRO_H

#include <QApplication>
#include <QMainWindow>

class QDrawPro
{
public:
    QDrawPro();
    ~QDrawPro();
    
    int run(int argc, char *argv[]);
    
private:
    void setupApplication();
    void setupMainWindow();
    
    QApplication *m_application;
    QMainWindow *m_mainWindow;
};

#endif // QDRAWPRO_H