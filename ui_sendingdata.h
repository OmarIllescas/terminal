/********************************************************************************
** Form generated from reading UI file 'sendingdata.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SENDINGDATA_H
#define UI_SENDINGDATA_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SendingData
{
public:

    void setupUi(QWidget *SendingData)
    {
        if (SendingData->objectName().isEmpty())
            SendingData->setObjectName(QStringLiteral("SendingData"));
        SendingData->resize(400, 300);

        retranslateUi(SendingData);

        QMetaObject::connectSlotsByName(SendingData);
    } // setupUi

    void retranslateUi(QWidget *SendingData)
    {
        SendingData->setWindowTitle(QApplication::translate("SendingData", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SendingData: public Ui_SendingData {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SENDINGDATA_H
