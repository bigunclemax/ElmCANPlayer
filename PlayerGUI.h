//
// Created by user on 15.11.2020.
//

#ifndef FOCUSIPCCTRL_PLAYERGUI_H
#define FOCUSIPCCTRL_PLAYERGUI_H

#include <QMainWindow>
#include <QThread>
#include <QCheckBox>
#include <memory>
#include <thread>
#include <CanController.h>
#include <QHBoxLayout>
#include <QtWidgets>
#include "utils.h"

using ElmPlayerUtils::Packet;

namespace Ui {
    class Form;
}

class PlayerThread : public QThread
{
    Q_OBJECT
    void run() override {
        do {
            for (const auto &packet : CAP) {
                if (isInterruptionRequested())
                    return;

                if (blackList.find(packet.ID) != blackList.end()) {
                    continue;
                }
                std::vector<uint8_t> data(reinterpret_cast<const uint8_t *>(packet.data),
                                          reinterpret_cast<const uint8_t *>(packet.data) + sizeof(Packet::data));

                emit signalLog(QString(packet.str().c_str()));
                m_controller->transaction(packet.ID, data);
            }
        } while (loopPlay);
    }

    std::vector<Packet> CAP;
    std::set<int> blackList;
    std::unique_ptr<CanController> m_controller;
    std::atomic<bool> loopPlay = false;

public:
    explicit PlayerThread(std::unique_ptr<CanController> controller) : m_controller(std::move(controller)) {};

    void play(const std::vector<Packet> &captured_data, const std::set<int> &_blackList) {
        if(!isRunning()) {
            CAP=captured_data;
            blackList=_blackList;
            start();
        }
    }

    void setProtocol(CAN_PROTO proto) {
        m_controller->set_protocol(proto);
    };

    void setLoopPlay(bool f) {
        loopPlay = f;
    };

    CanController * controller() { return m_controller.get(); }

signals:
    void signalLog(QString str);

};

class PlayerGUI : public QMainWindow {

    Q_OBJECT

private slots:
    void slotLog(const QString &str);
    void playFinished();

public:
    explicit PlayerGUI(std::unique_ptr<CanController> controller, QWidget *parent = nullptr);

private:

    enum enPeriodicCols {
        COL_CAN_ID = 0,
        COL_CAN_DATA,
        COL_EN,
        COL_COUNT
    };
    static constexpr int PERIODIC_MSG_COUNT = 10;

    void resizeEvent(QResizeEvent *event) override;

    PlayerThread player;
    Ui::Form *ui;
    std::vector<Packet> m_captured_data;
};

class CheckboxCell : public QWidget {

    Q_OBJECT

    QCheckBox *m_checkBox;
public:

    CheckboxCell() {
        m_checkBox = new QCheckBox();
        auto layoutCheckBox = new QHBoxLayout(this);
        layoutCheckBox->addWidget(m_checkBox);
        layoutCheckBox->setAlignment(Qt::AlignCenter);
        layoutCheckBox->setContentsMargins(0,0,0,0);
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::MouseButton::LeftButton) {
            m_checkBox->setChecked(!m_checkBox->isChecked());
        }
        QWidget::mousePressEvent(event);
    }

    [[nodiscard]] bool isChecked() const {
        return m_checkBox->isChecked();
    }
};

class DelegateColCanID : public QItemDelegate
{
public:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                          const QModelIndex & index) const override
    {
        auto lineEdit = new QLineEdit(parent);

        connect(lineEdit, &QLineEdit::textEdited, [lineEdit](const QString &text) {
                lineEdit->setText(text.toUpper());
        });

        auto validator = new QRegExpValidator(QRegExp("[0-9a-fA-F]{3}"), lineEdit);
        lineEdit->setValidator(validator);
        lineEdit->setPlaceholderText("XXX");
        return lineEdit;
    }
};

class DelegateColCanData : public QItemDelegate
{
public:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                          const QModelIndex & index) const override
    {
        auto lineEdit = new QLineEdit(parent);
        lineEdit->setMaxLength(23);

        connect(lineEdit, &QLineEdit::textEdited, [lineEdit](const QString &text) {
            if(!((text.size()+1) % 3)) {
                lineEdit->setText(text.toUpper() + " ");
            } else {
                lineEdit->setText(text.toUpper());
            }

        });

        lineEdit->setPlaceholderText("XX XX XX XX XX XX XX XX");
        auto validator = new QRegExpValidator(QRegExp("([0-9a-fA-F]{2} ){7}[0-9a-fA-F]{2}|"), lineEdit);
        lineEdit->setValidator(validator);

        return lineEdit;
    }
};

#endif //FOCUSIPCCTRL_PLAYERGUI_H
