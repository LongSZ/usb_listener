#include "usb_listener.h"

USB_Listener::USB_Listener(QWidget *parent) : QWidget(parent)
{
#ifdef _WINDOWS               //在VS2019中编译时使用此段代码注册USB带伤监听器, 用于处理USB挺拔事件的处理
    static const GUID GUID_DEVINTERFACE_LIST[] = {
        { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },             // GUID_DEVINTERFACE_USB_DEVICE
        { 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },             // GUID_DEVINTERFACE_DISK
        { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },             // GUID_DEVINTERFACE_HID,
        { 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }              // GUID_NDIS_LAN_CLASS
//        { 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },           // GUID_DEVINTERFACE_COMPORT
//        { 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },           // GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
//        { 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },           // GUID_DEVINTERFACE_PARALLEL
//        { 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }            // GUID_DEVINTERFACE_PARCLASS
    };

    //注册插拔事件
    HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotifacationFiler;
    ZeroMemory(&NotifacationFiler,sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    NotifacationFiler.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotifacationFiler.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    for (int i = 0; i < sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID); i++) {
        NotifacationFiler.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];                                   //GetCurrentUSBGUID();//m_usb->GetDriverGUID();
        hDevNotify = RegisterDeviceNotification((HANDLE)this->winId(), &NotifacationFiler, DEVICE_NOTIFY_WINDOW_HANDLE);
        if (!hDevNotify) {
            int Err = GetLastError();
            qDebug() << "注册失败" <<endl;
        }
    }
#endif
}

void USB_Listener::emit_my_signal()
{
    emit device_plugin();
}

bool USB_Listener::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    unsigned int msgType = msg->message;
    if(msgType == WM_DEVICECHANGE) {
        //cout << "1. msgType = " << msgType << endl;
        //qDebug() << "2. event WM_DEVICECHANGE happen" << endl;
        emit device_change_callback();               //触发信号

        if(msg->wParam == DBT_DEVICEARRIVAL) {
            //qDebug()  << "3. usb plugin" << endl;
            emit device_plugin();           //触发信号
        }
        else if(msg->wParam == DBT_DEVICEREMOVECOMPLETE) {
            //qDebug()  << "4. usb plugout" << endl;
            emit device_plugout();          //触发信号
        }
    }

    return QWidget::nativeEvent(eventType, message, result);
}

#ifdef _WINDOWS
char USB_Listener::first_drive_from_mask(ULONG unitmask)
{
    char i;
    for (i = 0; i < 26; ++i) {
        if (unitmask & 0x1) {
            break;
        }
        unitmask = unitmask >> 1;
    }

    return (i + 'A');
}

bool USB_Listener::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if (msgType == WM_DEVICECHANGE) {
        //std::cout << "5. Event DEVICECHANGE Happend" << std::endl;
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            //std::cout << "6. usb plugin" << std::endl;
            emit device_plugin();           //触发信号
//            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
//                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
//                if (lpdbv->dbcv_flags == 0) {                   //插入u盘
//                    QString USBDisk = QString(this->first_drive_from_mask(lpdbv ->dbcv_unitmask));
//                    //qDebug() << "7. USB_Arrived and The USBDisk is: " << USBDisk ;

//                }
//            }
//            if (lpdb->dbch_devicetype = DBT_DEVTYP_DEVICEINTERFACE) {
//                PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;
//                QString strname = QString::fromWCharArray(pDevInf->dbcc_name,pDevInf->dbcc_size);
//            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            //std::cout << "8. usb plugout" << std::endl;
            emit device_plugout();          //触发信号
//            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) {
//                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
//                if(lpdbv->dbcv_flags == 0) {
//                    //qDebug() << "9. USB_Removed";
//                }
//            }
//            if (lpdb->dbch_devicetype = DBT_DEVTYP_DEVICEINTERFACE) {
//                PDEV_BROADCAST_DEVICEINTERFACE pDevInf  = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;
//                //qDebug()<< "10. remove(name): " << pDevInf->dbcc_name;
//                //qDebug()<< "移除设备(guid)：" << pDevInf->dbcc_classguid;
//                //qDebug()<< "11. remove(size): " << pDevInf->dbcc_size;
//                QString strname = QString::fromWCharArray(pDevInf->dbcc_name,pDevInf->dbcc_size);
//            }
            break;
        }
    }
    return true;
}
#endif

