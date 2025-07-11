#include "spritecandidatesdialog.h"
#include "widgets/clientitemview.h" // For ClientItemView
#include "otb/item.h"             // For OTB::ClientItem

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

SpriteCandidatesDialog::SpriteCandidatesDialog(const QList<const OTB::ClientItem*>& candidates, QWidget *parent)
    : QDialog(parent), m_selectedClientId(0)
{
    setWindowTitle(tr("Sprite Candidates"));
    setModal(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_viewsLayout = new QHBoxLayout();

    for (const OTB::ClientItem* candidate : candidates) {
        if (!candidate) continue;

        ClientItemView* view = new ClientItemView(this);
        view->setClientItem(candidate);
        // Make the view clickable (e.g., by overriding mousePressEvent or using an event filter)
        // For simplicity now, we'll connect a signal if ClientItemView could emit one,
        // or use a small button per view. Let's go with a simpler direct connection model.
        // We'll need a way to associate the click back to the item.
        // One way: subclass ClientItemView to emit a signal with its item, or use a map.
        // For now, let's just display them. Clicking will be handled by separate buttons if needed,
        // or we can make the views themselves interactive later.

        // A simpler way: make a small layout for each view + a button
        QVBoxLayout* itemViewLayout = new QVBoxLayout();
        itemViewLayout->addWidget(view, 0, Qt::AlignCenter);
        QPushButton* selectButton = new QPushButton(tr("Select ID: %1").arg(candidate->id), this);
        connect(selectButton, &QPushButton::clicked, this, [this, candidate]() {
            onCandidateClicked(candidate);
        });
        itemViewLayout->addWidget(selectButton);
        m_viewsLayout->addLayout(itemViewLayout);
        m_candidateViews.append(view);
    }

    mainLayout->addLayout(m_viewsLayout);

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

SpriteCandidatesDialog::~SpriteCandidatesDialog()
{
    // Views are parented to this dialog, Qt will clean them up.
}

quint16 SpriteCandidatesDialog::getSelectedClientId() const
{
    return m_selectedClientId;
}

void SpriteCandidatesDialog::onCandidateClicked(const OTB::ClientItem* candidateItem)
{
    if (candidateItem) {
        m_selectedClientId = candidateItem->id;
        accept(); // Close the dialog and signal OK
    } else {
        reject(); // Should not happen if button is associated correctly
    }
}
