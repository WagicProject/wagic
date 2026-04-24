/****************************************************************************
** Meta object code from reading C++ file 'filedownloader.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../JGE/include/qt/filedownloader.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filedownloader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FileDownloader_t {
    QByteArrayData data[29];
    char stringdata0[358];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileDownloader_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileDownloader_t qt_meta_stringdata_FileDownloader = {
    {
QT_MOC_LITERAL(0, 0, 14), // "FileDownloader"
QT_MOC_LITERAL(1, 15, 15), // "receivedChanged"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 5), // "value"
QT_MOC_LITERAL(4, 38, 13), // "statusChanged"
QT_MOC_LITERAL(5, 52, 12), // "stateChanged"
QT_MOC_LITERAL(6, 65, 13), // "DownloadState"
QT_MOC_LITERAL(7, 79, 5), // "state"
QT_MOC_LITERAL(8, 85, 18), // "stateStringChanged"
QT_MOC_LITERAL(9, 104, 14), // "fileDownloaded"
QT_MOC_LITERAL(10, 119, 16), // "downloadProgress"
QT_MOC_LITERAL(11, 136, 13), // "bytesReceived"
QT_MOC_LITERAL(12, 150, 10), // "bytesTotal"
QT_MOC_LITERAL(13, 161, 14), // "setDownloadUrl"
QT_MOC_LITERAL(14, 176, 3), // "url"
QT_MOC_LITERAL(15, 180, 16), // "computeLocalHash"
QT_MOC_LITERAL(16, 197, 6), // "QFile&"
QT_MOC_LITERAL(17, 204, 4), // "file"
QT_MOC_LITERAL(18, 209, 11), // "requestHash"
QT_MOC_LITERAL(19, 221, 17), // "computeRemoteHash"
QT_MOC_LITERAL(20, 239, 17), // "handleStateChange"
QT_MOC_LITERAL(21, 257, 12), // "handleCancel"
QT_MOC_LITERAL(22, 270, 8), // "received"
QT_MOC_LITERAL(23, 279, 6), // "status"
QT_MOC_LITERAL(24, 286, 12), // "state_string"
QT_MOC_LITERAL(25, 299, 13), // "NETWORK_ERROR"
QT_MOC_LITERAL(26, 313, 16), // "DOWNLOADING_HASH"
QT_MOC_LITERAL(27, 330, 16), // "DOWNLOADING_FILE"
QT_MOC_LITERAL(28, 347, 10) // "DOWNLOADED"

    },
    "FileDownloader\0receivedChanged\0\0value\0"
    "statusChanged\0stateChanged\0DownloadState\0"
    "state\0stateStringChanged\0fileDownloaded\0"
    "downloadProgress\0bytesReceived\0"
    "bytesTotal\0setDownloadUrl\0url\0"
    "computeLocalHash\0QFile&\0file\0requestHash\0"
    "computeRemoteHash\0handleStateChange\0"
    "handleCancel\0received\0status\0state_string\0"
    "NETWORK_ERROR\0DOWNLOADING_HASH\0"
    "DOWNLOADING_FILE\0DOWNLOADED"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileDownloader[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       4,  102, // properties
       1,  118, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,
       4,    0,   77,    2, 0x06 /* Public */,
       5,    1,   78,    2, 0x06 /* Public */,
       8,    0,   81,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   82,    2, 0x08 /* Private */,
      10,    2,   83,    2, 0x08 /* Private */,
      13,    1,   88,    2, 0x08 /* Private */,
      15,    1,   91,    2, 0x08 /* Private */,
      18,    1,   94,    2, 0x08 /* Private */,
      19,    0,   97,    2, 0x08 /* Private */,
      20,    1,   98,    2, 0x08 /* Private */,
      21,    0,  101,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong, QMetaType::LongLong,   11,   12,
    QMetaType::Void, QMetaType::QUrl,   14,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, QMetaType::QUrl,   14,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,

 // properties: name, type, flags
      22, QMetaType::LongLong, 0x00495001,
      23, QMetaType::QString, 0x00495001,
       7, 0x80000000 | 6, 0x00495009,
      24, QMetaType::QString, 0x00495001,

 // properties: notify_signal_id
       0,
       1,
       2,
       3,

 // enums: name, alias, flags, count, data
       6,    6, 0x0,    4,  123,

 // enum data: key, value
      25, uint(FileDownloader::NETWORK_ERROR),
      26, uint(FileDownloader::DOWNLOADING_HASH),
      27, uint(FileDownloader::DOWNLOADING_FILE),
      28, uint(FileDownloader::DOWNLOADED),

       0        // eod
};

void FileDownloader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FileDownloader *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->receivedChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->statusChanged(); break;
        case 2: _t->stateChanged((*reinterpret_cast< DownloadState(*)>(_a[1]))); break;
        case 3: _t->stateStringChanged(); break;
        case 4: _t->fileDownloaded(); break;
        case 5: _t->downloadProgress((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 6: _t->setDownloadUrl((*reinterpret_cast< QUrl(*)>(_a[1]))); break;
        case 7: _t->computeLocalHash((*reinterpret_cast< QFile(*)>(_a[1]))); break;
        case 8: _t->requestHash((*reinterpret_cast< QUrl(*)>(_a[1]))); break;
        case 9: _t->computeRemoteHash(); break;
        case 10: _t->handleStateChange((*reinterpret_cast< DownloadState(*)>(_a[1]))); break;
        case 11: _t->handleCancel(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FileDownloader::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileDownloader::receivedChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FileDownloader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileDownloader::statusChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FileDownloader::*)(DownloadState );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileDownloader::stateChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (FileDownloader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileDownloader::stateStringChanged)) {
                *result = 3;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<FileDownloader *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< qint64*>(_v) = _t->received(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->getStatus(); break;
        case 2: *reinterpret_cast< DownloadState*>(_v) = _t->getState(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->getStateString(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject FileDownloader::staticMetaObject = { {
    QMetaObject::SuperData::link<QProgressDialog::staticMetaObject>(),
    qt_meta_stringdata_FileDownloader.data,
    qt_meta_data_FileDownloader,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FileDownloader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileDownloader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FileDownloader.stringdata0))
        return static_cast<void*>(this);
    return QProgressDialog::qt_metacast(_clname);
}

int FileDownloader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QProgressDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void FileDownloader::receivedChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FileDownloader::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void FileDownloader::stateChanged(DownloadState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void FileDownloader::stateStringChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
