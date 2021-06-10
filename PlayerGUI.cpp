//
// Created by user on 15.11.2020.
//

#include "PlayerGUI.h"
#include "ui_form.h"
#include <set>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QTextStream>
#include <QItemDelegate>
#include <QLineEdit>
#include <QTimer>

#include "utils.h"

PlayerGUI::PlayerGUI(std::unique_ptr<CanController> controller, QWidget *parent):
        QMainWindow(parent),
        ui(new Ui::Form),
        player(std::move(controller))
{
    ui->setupUi(this);
    ui->tab_FromFile->setEnabled(false);

    ui->comboBox_canType->addItem("CAN-MS");
    ui->comboBox_canType->addItem("CAN-HS");
    // TODO: ui->comboBox_canType->addItem("CAN-MM");

    connect(&player, &QThread::finished, this, &PlayerGUI::playFinished, Qt::QueuedConnection);

    connect(ui->actionExit, &QAction::triggered, this, []{QApplication::quit();});

    connect(ui->comboBox_canType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {

                if (ui->comboBox_canType->currentText() == "CAN-MS") {
                    player.setProtocol(CAN_MS);
                } else if (ui->comboBox_canType->currentText() == "CAN-HS") {
                    player.setProtocol(CAN_HS);
                }
            });

    ui->comboBox_canType->setCurrentIndex(1);

    /* Log show */
    ui->plainTextEdit_log->setFont(QFont("monospace"));

    /* Log enable */
    connect(ui->pushButton_logEnable, QOverload<bool>::of(&QPushButton::toggled),[this](bool enableLog)
    {
        if(enableLog) {
            connect(&player, &PlayerThread::signalLog, this, &PlayerGUI::slotLog, Qt::QueuedConnection);
        } else {
            player.disconnect(this);
        }
    });

    /* Log clear */
    connect(ui->pushButton_logClear, QOverload<bool>::of(&QPushButton::clicked),[this](bool clearLog)
    {
        ui->plainTextEdit_log->clear();
    });

    ui->plainTextEdit_log->setFont(QFont("monospace"));

    /* Open file to play */
    connect(ui->actionOpen, QOverload<bool>::of(&QAction::triggered),[this](bool i)
    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open CAN dump"), "",
                                                        tr("Dump (*.txt);;All Files (*)"));

        if (fileName.isEmpty())
            return;
        else {

            ui->listWidget->clear();

            m_captured_data = ElmPlayerUtils::convert(fileName.toStdString());
            std::set<int> unique_ids;
            for(const auto &p :m_captured_data) {
                unique_ids.insert(p.ID);
            }
            for(const auto &id :unique_ids) {
                ui->listWidget->addItem(QString("0x%1").arg(id, 3, 16, QLatin1Char('0')));
            }

            if (!m_captured_data.empty()) {
                ui->tab_FromFile->setEnabled(true);
            }
        }
    });

    /* Close file */
    connect(ui->actionClose, QOverload<bool>::of(&QAction::triggered),[this](bool i)
    {
        // TODO: clear log
        ui->listWidget->clear();
        ui->tab_FromFile->setEnabled(false);
    });

    connect(ui->actionLoad_config, QOverload<bool>::of(&QAction::triggered),[this](bool showLog)
    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open config"), "",
                                                        tr("All Files (*)"));
        if (fileName.isEmpty())
            return;
        else {

            QFile file(fileName);

            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                                         file.errorString());
                return;
            }

            QTextStream in(&file);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                for(int j=0; j < ui->listWidget->count(); ++j ) {
                    auto l = ui->listWidget->item(j);
                    if(l->text() == line) {
                        l->setSelected(true);
                        break;
                    }
                }
            }
        }
    });

    connect(ui->actionSave_config, QOverload<bool>::of(&QAction::triggered),[this](bool showLog)
    {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save config"), "",
                                                        tr("All Files (*)"));
        if (fileName.isEmpty())
            return;
        else {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                                         file.errorString());
                return;
            }
            for(int j=0; j < ui->listWidget->count(); ++j ) {
                auto l = ui->listWidget->item(j);
                if(l->isSelected())
                    file.write((l->text()+'\n').toLatin1());
            }
        }
    });

    connect(ui->pushButton_play, QOverload<bool>::of(&QPushButton::toggled),[this](bool i)
    {
        if(i) {
            /* lock corresponding gui elements */
            ui->tabWidget->tabBar()->setEnabled(false);
            ui->listWidget->setEnabled(false);
            ui->pushButton_selectAll->setEnabled(false);
            ui->pushButton_resetAll->setEnabled(false);
            ui->tab_PeriodicMsg->setEnabled(false);
            ui->comboBox_canType->setEnabled(false);

            /* selected ID's will be ignored */
            std::set<int> blackList;
            for(int j=0; j < ui->listWidget->count(); ++j ) {
                auto l = ui->listWidget->item(j);
                if(l->isSelected()) {
                    blackList.insert(l->text().toInt(nullptr, 16));
                }
            }

            player.play(m_captured_data, blackList);

        } else {
            player.requestInterruption();
            player.wait();

            /* unlock corresponding gui elements */
            ui->tabWidget->tabBar()->setEnabled(true);
            ui->listWidget->setEnabled(true);
            ui->pushButton_selectAll->setEnabled(true);
            ui->pushButton_resetAll->setEnabled(true);
            ui->tab_PeriodicMsg->setEnabled(true);
            ui->comboBox_canType->setEnabled(true);
        }
    });

    connect(ui->checkBox_loopPlay, &QCheckBox::toggled, this, [this](bool checked) {
        player.setLoopPlay(checked);
    });

    connect(ui->pushButton_selectAll, QOverload<bool>::of(&QPushButton::clicked),
            [this](bool i)
            {
                for(int j=0; j < ui->listWidget->count(); ++j ) {
                    auto l = ui->listWidget->item(j);
                    l->setSelected(true);
                }
            });

    connect(ui->pushButton_resetAll, QOverload<bool>::of(&QPushButton::clicked),
            [this](bool i)
            {
                for(int j=0; j < ui->listWidget->count(); ++j ) {
                    auto l = ui->listWidget->item(j);
                    l->setSelected(false);
                }
            });

    ui->tableWidget_periodic->setRowCount(PERIODIC_MSG_COUNT);
    ui->tableWidget_periodic->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_periodic->setItemDelegateForColumn(COL_CAN_ID, new DelegateColCanID);
    ui->tableWidget_periodic->setItemDelegateForColumn(COL_CAN_DATA, new DelegateColCanData);
    for (int i = 0; i < PERIODIC_MSG_COUNT; ++i) {
        ui->tableWidget_periodic->setCellWidget(i, COL_EN, new CheckboxCell());
    }

    connect(ui->pushButton_playPeriodic, QOverload<bool>::of(&QPushButton::toggled),[this](bool doPlay) {


        if(doPlay) {
            // lock UI (tabs, table)
            ui->tabWidget->tabBar()->setEnabled(false);
            ui->tableWidget_periodic->setEnabled(false);

            // check table for valid CAN packages
            std::vector<Packet> periodic_data;

            for (int i = 0; i < PERIODIC_MSG_COUNT; ++i) {

                bool ok;
                auto checkBox = qobject_cast <CheckboxCell*> (ui->tableWidget_periodic->cellWidget(i, COL_EN));
                if (!checkBox) {
                    continue;
                }
                if (!checkBox->isChecked()) {
                    continue;
                }
                auto item = ui->tableWidget_periodic->item(i, COL_CAN_ID);
                if (!item) {
                    continue;
                }
                auto can_id = item->text().toUShort(&ok, 16);
                if(!ok) {
                    continue;
                }
                item = ui->tableWidget_periodic->item(i, COL_CAN_DATA);
                if (!item) {
                    continue;
                }
                auto data = item->text().simplified().replace( " ", "" ).toULongLong(&ok, 16);
                if(!ok) {
                    continue;
                }
                data = qToBigEndian(data);
                periodic_data.push_back({can_id, data});
            }

            if (!periodic_data.empty()) {
                player.setLoopPlay(true);
                player.play(periodic_data, std::set<int>());
            } else {
                ui->pushButton_playPeriodic->setChecked(false);
            }

        } else {
            player.requestInterruption();
            player.wait();

            // unlock UI (tabs, table)
            ui->tabWidget->tabBar()->setEnabled(true);
            ui->tableWidget_periodic->setEnabled(true);
        }
    });
}

void PlayerGUI::resizeEvent(QResizeEvent *event) {

    QTimer::singleShot(0, [&] {
        ui->tableWidget_periodic->setColumnWidth(COL_CAN_ID, ui->tableWidget_periodic->width() * 0.2);
        ui->tableWidget_periodic->setColumnWidth(COL_CAN_DATA, ui->tableWidget_periodic->width() * 0.5);
    });

    QWidget::resizeEvent(event);
}

void PlayerGUI::slotLog(const QString &str) {
    ui->plainTextEdit_log->appendPlainText(str);
}

void PlayerGUI::playFinished() {
    if (ui->pushButton_play->isChecked())
        ui->pushButton_play->setChecked(false);
}