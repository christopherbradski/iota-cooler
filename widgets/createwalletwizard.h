/**
  * \class CreateWalletWizard
  * \brief Stacked widget with step-by-step instructions
  * for cold wallet creation, with online and offline tasks.
  * \author Oirio Joshi
  * \date 2018-03-13
  */

#ifndef CREATEWALLETWIZARD_H
#define CREATEWALLETWIZARD_H

#include <QWidget>

namespace Ui {
class CreateWalletWizard;
}

class CreateWalletWizard : public QWidget
{
    Q_OBJECT

public:
    explicit CreateWalletWizard(QWidget *parent = 0);
    ~CreateWalletWizard();

signals:
    /** Emitted when user cancels the wizard */
    void walletCreationCancelled();

private slots:
    void infoNextButtonClicked();
    void infoCancelButtonClicked();

private:
    Ui::CreateWalletWizard *ui;
};

#endif // CREATEWALLETWIZARD_H