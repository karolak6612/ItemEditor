#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    QLabel *titleLabel;
    QLabel *versionLabel;
    QLabel *descriptionLabel;
    QLabel *qtVersionLabel;
    QLabel *sourceLink;
    // Add other labels from C# AboutForm (UI Icons, App Icon, Developer)

    QPushButton *okButton;
};

#endif // ABOUTDIALOG_H
