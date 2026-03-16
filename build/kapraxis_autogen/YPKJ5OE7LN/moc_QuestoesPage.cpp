/****************************************************************************
** Meta object code from reading C++ file 'QuestoesPage.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/QuestoesPage.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QuestoesPage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_QuestoesPage_t {
    uint offsetsAndSizes[30];
    char stringdata0[13];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[15];
    char stringdata5[16];
    char stringdata6[17];
    char stringdata7[5];
    char stringdata8[15];
    char stringdata9[15];
    char stringdata10[16];
    char stringdata11[6];
    char stringdata12[15];
    char stringdata13[6];
    char stringdata14[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_QuestoesPage_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_QuestoesPage_t qt_meta_stringdata_QuestoesPage = {
    {
        QT_MOC_LITERAL(0, 12),  // "QuestoesPage"
        QT_MOC_LITERAL(13, 16),  // "adicionarQuestao"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 13),  // "editarQuestao"
        QT_MOC_LITERAL(45, 14),  // "excluirQuestao"
        QT_MOC_LITERAL(60, 15),  // "mostrarDetalhes"
        QT_MOC_LITERAL(76, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(93, 4),  // "item"
        QT_MOC_LITERAL(98, 14),  // "carregarEstilo"
        QT_MOC_LITERAL(113, 14),  // "salvarResposta"
        QT_MOC_LITERAL(128, 15),  // "filtrarQuestoes"
        QT_MOC_LITERAL(144, 5),  // "index"
        QT_MOC_LITERAL(150, 14),  // "buscarQuestoes"
        QT_MOC_LITERAL(165, 5),  // "texto"
        QT_MOC_LITERAL(171, 10)   // "focarBusca"
    },
    "QuestoesPage",
    "adicionarQuestao",
    "",
    "editarQuestao",
    "excluirQuestao",
    "mostrarDetalhes",
    "QListWidgetItem*",
    "item",
    "carregarEstilo",
    "salvarResposta",
    "filtrarQuestoes",
    "index",
    "buscarQuestoes",
    "texto",
    "focarBusca"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_QuestoesPage[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x08,    1 /* Private */,
       3,    0,   69,    2, 0x08,    2 /* Private */,
       4,    0,   70,    2, 0x08,    3 /* Private */,
       5,    1,   71,    2, 0x08,    4 /* Private */,
       8,    0,   74,    2, 0x08,    6 /* Private */,
       9,    0,   75,    2, 0x08,    7 /* Private */,
      10,    1,   76,    2, 0x08,    8 /* Private */,
      12,    1,   79,    2, 0x08,   10 /* Private */,
      14,    0,   82,    2, 0x08,   12 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject QuestoesPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_QuestoesPage.offsetsAndSizes,
    qt_meta_data_QuestoesPage,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_QuestoesPage_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<QuestoesPage, std::true_type>,
        // method 'adicionarQuestao'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'editarQuestao'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'excluirQuestao'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'mostrarDetalhes'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'carregarEstilo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'salvarResposta'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'filtrarQuestoes'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'buscarQuestoes'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'focarBusca'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void QuestoesPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QuestoesPage *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->adicionarQuestao(); break;
        case 1: _t->editarQuestao(); break;
        case 2: _t->excluirQuestao(); break;
        case 3: _t->mostrarDetalhes((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 4: _t->carregarEstilo(); break;
        case 5: _t->salvarResposta(); break;
        case 6: _t->filtrarQuestoes((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->buscarQuestoes((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->focarBusca(); break;
        default: ;
        }
    }
}

const QMetaObject *QuestoesPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QuestoesPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QuestoesPage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int QuestoesPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
