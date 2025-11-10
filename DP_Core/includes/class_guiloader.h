#pragma once

#include <QThread>
#include <QApplication>

class GuiLoader
{
public:

    template<typename T>
    static void setViewProperty(QObject* view, const QString& property, const T& value)
    {

        if (view)
            QMetaObject::invokeMethod(view, property.toLatin1().constData(),
                                      Qt::AutoConnection, QArgument<T>(QMetaType::typeName(qMetaTypeId<T>()), value));
    }

    template <typename Functor, typename Ret>
    static void exec(const Functor& runnable, Ret* result = nullptr)
    {
        // If this function is called from the Gui Thread, exec directly and return
        // Otherwise, invoke runnable object in Gui Thread and wait until execution finishes
        if (QThread::currentThread() == QApplication::instance()->thread())
        {
            if (result)
                *result = runnable();
            else
                runnable();
        }
        else
            QMetaObject::invokeMethod(QApplication::instance()->thread(), runnable,
                                      Qt::BlockingQueuedConnection, result);
    }

    template<typename Functor>
    static void exec(const Functor& runnable)
    {
        // If this function is called from the Gui Thread, exec directly and return
        // Otherwise, invoke runnable object in Gui Thread and wait until execution finishes
        if (QThread::currentThread() == QApplication::instance()->thread())
                runnable();
        else
            QMetaObject::invokeMethod(QApplication::instance()->thread(), runnable, Qt::BlockingQueuedConnection);
    }

    template<typename Functor>
    static void async(const Functor& runnable)
    {
        // If this function is called from the Gui Thread, exec directly and return
        // Otherwise, invoke runnable object in Gui Thread and wait until execution finishes
        if (QThread::currentThread() == QApplication::instance()->thread())
                runnable();
        else
            QMetaObject::invokeMethod(QApplication::instance()->thread(), runnable, Qt::AutoConnection);
    }

};
