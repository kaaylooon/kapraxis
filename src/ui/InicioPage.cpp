// InicioPage.cpp
#include "InicioPage.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

InicioPage::InicioPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    
    auto* title = new QLabel("Kapraxis");
    title->setStyleSheet("font-size: 32px; font-weight: bold; margin-top: 50%");
    
    auto* subtitle = new QLabel("Sistema de Gestão de Estudos");
    subtitle->setStyleSheet("font-size: 16px;");
    
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addStretch();
}
