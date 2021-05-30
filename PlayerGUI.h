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

    PlayerThread player;
    Ui::Form *ui;
    std::vector<Packet> m_captured_data;

};


#endif //FOCUSIPCCTRL_PLAYERGUI_H
