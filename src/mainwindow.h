#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "qcustomplot.h"
#include "mxzmq.h"
#include "defines.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void saveSettings();
    void loadSettings();
    QColor colorSequence(unsigned long n);
    QColor textColorSequence(unsigned long n);

private slots:
    // Functions that are called as reaction to a message
    // slots -> is a Qt feature, can be public or private
    void appendLogMessage(bool status);
    void connectButtonToggled(bool checked);
    void endpointLineEdited(const QString &text);
    void newMessageReceived(const QJsonObject &obj);
    void invalidPayloadReceived(const std::invalid_argument &ex, const QString &payload);
    void invalidMessageReceived(int parts);
    void noMessageReceived();
    void closeEvent(QCloseEvent *event);    // slot executed before closing window

private:
    Ui::MainWindow *ui;
    MXZmq *_zmq;
    bool _idle = true;
    QSettings _settings = QSettings(APP_DOMAIN, APP_NAME);
    QMap<QString, QCPGraph *> _sequenceCharts;
    QMap<QString, QCPCurve *> _fullCharts;
    unsigned long int _messageCount = 0;
    QTimer *_replotTimer = new QTimer(this);
    QStandardItemModel *_sequenceChartsModel;
    QStandardItemModel *_fullChartsModel;
    QList<QColor> _palette = QList<QColor>({QColor("#000000"), QColor("#F8766D")});
};
#endif // MAINWINDOW_H
