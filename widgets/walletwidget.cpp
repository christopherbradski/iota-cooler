#include "walletwidget.h"
#include "ui_walletwidget.h"
#include "../components/walletmanager.h"
#include "../components/smidgentangleapi.h"
#include "../utils/utilsiota.h"
#include "../components/settingsmanager.h"

#include <QtWidgets/QLabel>
#include <QtCore/QTimer>
#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include <QtWidgets/QMessageBox>

WalletWidget::WalletWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WalletWidget),
    m_BalanceRefreshTimer(0)
{
    ui->setupUi(this);
    m_currentWalletFilePath = "/invalid";
    m_walletManager = &WalletManager::getInstance();
    m_settingsManager = new SettingsManager(this);

    connect(ui->closeWalletButton, &QPushButton::clicked,
            this, &WalletWidget::closeWalletButtonClicked);
    connect(ui->copyAddressToolButton, &QToolButton::clicked,
            this, &WalletWidget::copyCurrentAddress);

    //tangle api
    m_tangleAPI = new SmidgenTangleAPI(this);
    connect(m_tangleAPI, &AbstractTangleAPI::requestFinished,
            this, &WalletWidget::requestFinished);
    connect(m_tangleAPI, &AbstractTangleAPI::requestError,
            this, &WalletWidget::requestError);
}

WalletWidget::~WalletWidget()
{
    stopBalanceRefresher();
    delete ui;
}

void WalletWidget::setCurrentWalletPath(const QString &walletFilePath)
{
    m_currentWalletFilePath = walletFilePath;

    //main wallet address
    QString address = m_walletManager->getCurrentAddress();
    QString addressWrapped = QString(address).insert(30, "<br />")
                                             .insert(66, "<br />");
    ui->addressLabel->setText(QString("<a href='%1'>%2</a>")
                              .arg("https://thetangle.org/address/" + address)
                              .arg(addressWrapped));

    //check balance
    updateBalance();
    //setup balance periodic check
    startBalanceRefresher();
}

void WalletWidget::updateBalance()
{
    if (m_currentWalletFilePath != "/invalid") {
        ui->balanceLabel->setText(tr("checking..."));
        QString a = m_walletManager->getCurrentAddress();
        if (UtilsIOTA::isValidAddress(a)) {
            QStringList args;
            args.append(a);
            m_tangleAPI->startAPIRequest(AbstractTangleAPI::GetBalance,
                                         args);
        }
    }
}

void WalletWidget::closeWalletButtonClicked()
{
    stopBalanceRefresher();
    emit walletClosed();
}

void WalletWidget::copyCurrentAddress()
{
    QGuiApplication::clipboard()->setText(
                m_walletManager->getCurrentAddress());
}

void WalletWidget::requestFinished(AbstractTangleAPI::RequestType request,
                                   const QString &responseMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
    {
        QString balanceHumanReadable = responseMessage.split(":").at(3);
        ui->balanceLabel->setText(balanceHumanReadable);

    }
        break;
    default:
        break;
    }
}

void WalletWidget::requestError(AbstractTangleAPI::RequestType request,
                                const QString &errorMessage)
{
    switch (request) {
    case AbstractTangleAPI::GetBalance:
        ui->balanceLabel->setText(tr("error..."));
        if (errorMessage.contains("--provider")) { //TODO: check actual message when offline
            emit showStatusMessage(tr("Failed to check balance! "
                                      "Please check your node and connection"));
            break;
        } else {
            //break missing
            //to show error message with box belo
            //WARNING: Do not add any switch cases below this, only above!
        }
    default:
        QMessageBox::critical(this,
                              tr("IOTA API Request Error"),
                              errorMessage);
        break;
    }
}

void WalletWidget::startBalanceRefresher()
{
    if (!m_BalanceRefreshTimer) {
        m_BalanceRefreshTimer = new QTimer(this);
    }
    connect(m_BalanceRefreshTimer, &QTimer::timeout,
            this, &WalletWidget::updateBalance);
    m_BalanceRefreshTimer->start(30000); //every 30sec
}

void WalletWidget::stopBalanceRefresher()
{
    if (m_BalanceRefreshTimer) {
        m_BalanceRefreshTimer->stop();
        disconnect(m_BalanceRefreshTimer, &QTimer::timeout,
                   this, &WalletWidget::updateBalance);
    }
}
