#include "aboutdialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication> // For qApp->applicationVersion()
#include <QPixmap> // For logo/image if needed
#include <QDesktopServices> // For opening links
#include <QUrl>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("About ItemEditor Qt"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title/Logo (Optional: Add a logo like the C# version)
    // QPixmap logo(":/images/about_background.png"); // Assuming image is in resources
    // QLabel* logoLabel = new QLabel();
    // if (!logo.isNull()) logoLabel->setPixmap(logo.scaledToWidth(400, Qt::SmoothTransformation));
    // mainLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);


    titleLabel = new QLabel(tr("ItemEditor Qt"));
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Version - using QApplication's version if set, or a placeholder
    QString appVersion = qApp->applicationVersion();
    if (appVersion.isEmpty()) {
        appVersion = tr("1.0.0 (Development)"); // Placeholder
    }
    versionLabel = new QLabel(tr("Version: %1").arg(appVersion));
    versionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(versionLabel);

    mainLayout->addSpacing(10);

    descriptionLabel = new QLabel(
        tr("This is a Qt6 port of the ItemEditor application, "
           "originally written in C# by Mignari and other contributors from the OTTools project.")
    );
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(descriptionLabel);

    mainLayout->addSpacing(5);

    sourceLink = new QLabel();
    sourceLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    sourceLink->setOpenExternalLinks(true);
    sourceLink->setText(tr("Source code available on <a href=\"https://github.com/ottools/ItemEditor\">GitHub (Original C#)</a>"));
    // Add link for this Qt port if it exists
    sourceLink->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(sourceLink);

    mainLayout->addSpacing(10);

    // Developer Info (Example - can be expanded based on C# AboutForm.Designer.cs)
    QLabel* originalDevLabel = new QLabel(tr("Original C# Developer: Mignari"));
    originalDevLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(originalDevLabel);

    QLabel* qtPortDevLabel = new QLabel(tr("Qt Port Developer: Jules (AI Agent)")); // That's me!
    qtPortDevLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(qtPortDevLabel);

    mainLayout->addSpacing(10);

    qtVersionLabel = new QLabel(tr("Built with Qt %1").arg(QT_VERSION_STR));
    qtVersionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(qtVersionLabel);

    mainLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("OK"));
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    setMinimumWidth(400);
    setLayout(mainLayout);
}

AboutDialog::~AboutDialog()
{
}
