#pragma once

#include <QDialog>

namespace UI {
namespace Dialogs {

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    virtual ~AboutDialog() = default;
};

} // namespace Dialogs
} // namespace UI