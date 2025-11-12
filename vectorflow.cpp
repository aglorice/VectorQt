#include "vectorflow.h"
#include "mainwindow.h"
#include <QApplication>

QDrawPro::QDrawPro()
    : m_application(nullptr)
    , m_mainWindow(nullptr)
{
}

QDrawPro::~QDrawPro()
{
    delete m_mainWindow;
    delete m_application;
}

void QDrawPro::setupApplication()
{
    // Application setup is done in run()
}

void QDrawPro::setupMainWindow()
{
    m_mainWindow = new MainWindow();
    m_mainWindow->show();
}

int QDrawPro::run(int argc, char *argv[])
{
    m_application = new QApplication(argc, argv);
    
    setupApplication();
    setupMainWindow();
    
    return m_application->exec();
}

