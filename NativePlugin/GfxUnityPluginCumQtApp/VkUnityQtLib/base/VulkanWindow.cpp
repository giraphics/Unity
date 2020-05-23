#include "VulkanWindow.h"

#include<QTimer>

#include "vulkanexamplebase.h"

Window::Window(VulkanExampleBase* vulkanApp) : m_VulkanApp(vulkanApp)
{
    assert(vulkanApp);

    VkExtent2D dimension = {1280, 720};

#ifdef USE_WINDOW
    setWidth(dimension.width);
    setHeight(dimension.height);
#else
//    setFixedWidth(dimension.width);
//    setFixedHeight(dimension.height);
    resize(dimension.width, dimension.height);

    // We need a QWindow backing this widget
    setAttribute(Qt::WA_NativeWindow);

    // And we need that to be a Metal surface
#if defined (VK_USE_PLATFORM_XCB_KHR) || defined (_WIN32)
    windowHandle()->setSurfaceType(QWindow::VulkanSurface);
#else
    windowHandle()->setSurfaceType(QWindow::MetalSurface);
#endif

    // We don't need to participate in the backingstore sync
    setAttribute(Qt::WA_PaintOnScreen);

    // We don't need any background drawn for us
    setAttribute(Qt::WA_OpaquePaintEvent);
#endif

    m_RenderTimer = new QTimer();
    m_RenderTimer->setInterval(1);

    connect(m_RenderTimer, SIGNAL(timeout()), this, SLOT(Run()));
    m_RenderTimer->start();
}

void Window::Run()
{
    m_VulkanApp->renderLoop();
}

void Window::resizeEvent(QResizeEvent* pEvent)
{
//    if (m_VulkanApp->m_ReSizeEnabled == true &&
//        isVisible() == true)
//    {
//        int newWidth = width();
//        int newHeight = height();

//        m_VulkanApp->ResizeWindow(newWidth, newHeight);
//    }

#ifdef USE_WINDOW
    QWindow::resizeEvent(pEvent);
#else
    QWidget::resizeEvent(pEvent);
#endif
}
