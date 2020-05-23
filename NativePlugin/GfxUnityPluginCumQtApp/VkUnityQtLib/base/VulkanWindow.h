#pragma once

#include <QWidget>
#include <QWindow>

#define USE_WINDOW
#ifdef USE_WINDOW
class VulkanExampleBase;
class Window : public QWindow
#else
class Window : public QWidget
#endif
{
    Q_OBJECT

public:
    Window(VulkanExampleBase* parent = NULL);
    ~Window() { delete m_RenderTimer; }

    public slots:
    void Run();
    void resizeEvent(QResizeEvent* pEvent);
    virtual void mousePressEvent(QMouseEvent* p_Event) { /*m_VulkanApp->mousePressEvent(p_Event);*/ }
    virtual void mouseReleaseEvent(QMouseEvent* p_Event) { /*m_VulkanApp->mouseReleaseEvent(p_Event);*/ }
    virtual void mouseMoveEvent(QMouseEvent* p_Event) { /*m_VulkanApp->mouseMoveEvent(p_Event);*/ }

private:
    QTimer* m_RenderTimer;	// Refresh timer
    VulkanExampleBase* m_VulkanApp; // Used to call run() by the timer
};
