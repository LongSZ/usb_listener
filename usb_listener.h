#ifndef USB_LISTENER_H
#define USB_LISTENER_H

//Qt
#include <QAbstractNativeEventFilter>
#include <QWidget>
#include <QDebug>
#include <windows.h>                    //dbt.h头文件需要它
#include <dbt.h>
#include <devguid.h>
#include <SetupAPI.h>
#include <InitGuid.h>
#include <QtSerialPort/QSerialPortInfo>
#include <QApplication>

//C++
#include <iostream>
using namespace std;

class USB_Listener : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit USB_Listener(QWidget *parent = nullptr);

    void emit_my_signal();

signals:
    void device_change_callback();
    void device_plugin();
    void device_plugout();

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

#ifdef _WINDOWS
    char first_drive_from_mask(ULONG unitmask);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif
};

#endif // USB_LISTENER_H
