#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include <QPixmap>
#include "../Properties/version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    initializeGraphics();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::initializeGraphics()
{
    QPixmap background(":/about_background.png");
    ui->pictureBox->setPixmap(background);
    ui->versionLabel->setText(APPLICATION_VERSION);
}
