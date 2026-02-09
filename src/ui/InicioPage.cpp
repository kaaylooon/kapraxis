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
    
    auto* title = new QLabel("📚 Kasty");
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: #2c3e50; margin-top: 40%");
    
    auto* subtitle = new QLabel("Sistema de Gestão de Estudos");
    subtitle->setStyleSheet("font-size: 16px; color: #7f8c8d;");
    
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addStretch();
}