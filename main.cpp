#include <QApplication>
#include "PlayerGUI.h"
#include "dialog.h"
#include "CanController.h"
#include <QtConcurrent/QtConcurrent>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::unique_ptr<Dialog>         init_dialog = std::make_unique<Dialog>();
    std::unique_ptr<PlayerGUI>      form;
    std::unique_ptr<CanController>  can_controller;

    QFuture<QString> future;
    QFutureWatcher<QString> watcher;

    QObject::connect(&watcher, &QFutureWatcher<QString>::finished, [&]{

        if(watcher.result().isEmpty()) {
            init_dialog->close();
            form = std::make_unique<PlayerGUI>(std::move(can_controller));
            form->setWindowTitle("CAN Player");
            form->show();
        } else {
            QMessageBox(QMessageBox::Warning,
                        "CAN device error", watcher.result(), QMessageBox::Ok, init_dialog.get()).exec();
            init_dialog->setEnabled(true);
        }
    });

    QObject::connect(init_dialog.get(), &Dialog::rejected, [&] {
        if (!can_controller) {
            exit(0);
        }
    });

    QObject::connect(init_dialog.get(), &Dialog::btnOk_click, [&]{

        future = QtConcurrent::run([&] {
            auto settings = init_dialog->getSettings();
            try {
                can_controller = std::make_unique<CanController>(settings.port_name,
                                                                 static_cast<uint32_t>(!settings.autodetect
                                                                                       ? settings.baudrate : 0),
                                                                 settings.maximize);
            } catch (std::runtime_error& ex) {
                return QString(ex.what());
            }
            return QString();
        });
        watcher.setFuture(future);

    });

    init_dialog->setWindowTitle("Settings");
    init_dialog->show();

    return QApplication::exec();
}
