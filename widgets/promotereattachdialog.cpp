#include "promotereattachdialog.h"
#include "ui_promotereattachdialog.h"
#include "../components/smidgentangleapi.h"
#include "../utils/utilsiota.h"

#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDialogButtonBox>

PromoteReattachDialog::PromoteReattachDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PromoteReattachDialog),
    m_promoteCounter(0)
{
    ui->setupUi(this);
    ui->okButton->setAutoDefault(false);
    ui->okButton->setDefault(false);
    ui->cancelButton->setAutoDefault(false);
    ui->cancelButton->setDefault(false);

    connect(ui->okButton, &QPushButton::clicked,
            this, &PromoteReattachDialog::okButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &PromoteReattachDialog::cancelButtonClicked);
    connect(ui->promoteButton, &QPushButton::clicked,
            this, &PromoteReattachDialog::promoteButtonClicked);
    connect(ui->promoteLineEdit, &QLineEdit::textChanged,
            this, &PromoteReattachDialog::promoteLineEditChanged);
    connect(ui->reattachButton, &QPushButton::clicked,
            this, &PromoteReattachDialog::reattachButtonClicked);
    connect(ui->reattachLineEdit, &QLineEdit::textChanged,
            this, &PromoteReattachDialog::reattachLineEditChanged);

    //IOTA API
    m_tangleAPI = new SmidgenTangleAPI(this);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &PromoteReattachDialog::tangleAPIRequestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &PromoteReattachDialog::tangleAPIRequestError);
}

PromoteReattachDialog::~PromoteReattachDialog()
{
    delete ui;
}

void PromoteReattachDialog::setPromoteView()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->promoteResultLabel->clear();
    ui->promoteResultLabel->hide();
    ui->promoteProgressBar->hide();
    ui->promoteStatusLabel->hide();
    ui->promoteButton->setEnabled(false);
}

void PromoteReattachDialog::setReattachView()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->reattachResultLabel->clear();
    ui->reattachResultLabel->hide();
    ui->reattachProgressBar->hide();
    ui->reattachStatusLabel->hide();
    ui->reattachButton->setEnabled(false);
}

void PromoteReattachDialog::okButtonClicked()
{
    this->accept();
}

void PromoteReattachDialog::cancelButtonClicked()
{
    m_tangleAPI->stopCurrentAPIRequest();
    this->reject();
}

void PromoteReattachDialog::promoteLineEditChanged()
{
    QString txHash = ui->promoteLineEdit->text().trimmed().toUpper();
    //check if tx is valid
    bool valid = UtilsIOTA::isValidTxHash(txHash);
    ui->promoteButton->setEnabled(valid);
}

void PromoteReattachDialog::promoteButtonClicked()
{
    ui->promoteResultLabel->clear();
    ui->promoteStatusLabel->show();
    ui->promoteProgressBar->show();
    ui->promoteButton->hide();
    ui->promoteLineEdit->hide();
    ui->tailTxHashLabelPromote->hide();
    ui->okButton->setEnabled(false);

    //promote
    m_promoteCounter = 5; // promote 5 times
    QString tailTxHash = ui->promoteLineEdit->text().trimmed();
    m_currentPromoteTxHash = tailTxHash;
    QStringList args;
    args.append(m_currentPromoteTxHash);
    m_promoteCounter--;
    ui->promoteProgressBar->setValue(1);
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::RequestType::Promote,
                                 args);
}

void PromoteReattachDialog::reattachLineEditChanged()
{
    QString txHash = ui->reattachLineEdit->text().trimmed().toUpper();
    //check if tx is valid
    bool valid = UtilsIOTA::isValidTxHash(txHash);
    ui->reattachButton->setEnabled(valid);
}

void PromoteReattachDialog::reattachButtonClicked()
{
    ui->reattachResultLabel->clear();
    ui->reattachStatusLabel->show();
    ui->reattachProgressBar->show();
    ui->reattachButton->hide();
    ui->reattachLineEdit->hide();
    ui->tailTxHashLabelReattach->hide();
    ui->okButton->setEnabled(false);

    //reattach
    QString tailTxHash = ui->reattachLineEdit->text().trimmed();
    QStringList args;
    args.append(tailTxHash);
    m_tangleAPI->startAPIRequest(AbstractTangleAPI::RequestType::Reattach,
                                 args);
}

void PromoteReattachDialog::tangleAPIRequestFinished(AbstractTangleAPI::RequestType request,
                                                     const QString &message)
{
    switch (request) {
    case AbstractTangleAPI::RequestType::Promote:
        //promoted at least 5 times
        if (m_promoteCounter > 0) {
            QStringList args;
            args.append(m_currentPromoteTxHash);
            m_promoteCounter--;
            ui->promoteProgressBar->setValue(ui->promoteProgressBar->value() + 1);
            m_tangleAPI->startAPIRequest(AbstractTangleAPI::RequestType::Promote,
                                         args);
        } else {
            ui->okButton->setEnabled(true);
            promoteFinishedViewState(message);
        }
        break;
    case AbstractTangleAPI::RequestType::Reattach:
        ui->okButton->setEnabled(true);
        reattachFinishedViewState(message);
        break;
    default:
        break;
    }
}

void PromoteReattachDialog::tangleAPIRequestError(AbstractTangleAPI::RequestType request,
                                                  const QString &message)
{
    ui->okButton->setEnabled(true);

    switch (request) {
    case AbstractTangleAPI::RequestType::Promote:
        promoteFinishedViewState(message);
        break;
    case AbstractTangleAPI::RequestType::Reattach:
        reattachFinishedViewState(message);
        break;
    default:
        break;
    }
}

void PromoteReattachDialog::promoteFinishedViewState(const QString &result)
{
    ui->promoteResultLabel->setText(result);
    ui->promoteResultLabel->show();
    ui->promoteStatusLabel->hide();
    ui->promoteProgressBar->hide();
    ui->promoteButton->show();
    ui->promoteLineEdit->show();
    ui->tailTxHashLabelPromote->show();
}

void PromoteReattachDialog::reattachFinishedViewState(const QString &result)
{
    ui->reattachResultLabel->setText(result);
    ui->reattachResultLabel->show();
    ui->reattachStatusLabel->hide();
    ui->reattachProgressBar->hide();
    ui->reattachButton->show();
    ui->reattachLineEdit->show();
    ui->tailTxHashLabelReattach->show();
}
