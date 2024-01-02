#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "defines.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadSettings();

    setWindowTitle(APP_NAME "-" APP_VERSION);

    _sequenceChartsModel = new QStandardItemModel(0, 0, ui->SequenceChart);
    ui->SequenceChartSeriesList->setModel(_sequenceChartsModel);

    _fullChartsModel = new QStandardItemModel(0, 0, ui->FullChart);
    ui->FullChartSeriesList->setModel(_fullChartsModel);

    ui->endpointline->setText(DEFAULT_ENDPOINT);
    ui->topicline->setText(DEFAULT_TOPIC);

    _zmq = new MXZmq(this, ui->topicline->text(), ui->endpointline->text().toStdString());
    // if _zmq init fails, endpoint is reset to default, synchro back to the UI
    ui->endpointline->setText(_zmq->endpoint());

    // Lambda function
    // [=] -> init lambda, means take all outside the scope
    // () -> argument of the lambda

    // object, method, object, method
    connect(ui->connectButton, &QPushButton::toggled, this, &MainWindow::connectButtonToggled);
    connect(ui->endpointline, &QLineEdit::textEdited, this, &MainWindow::endpointLineEdited);

    connect(_zmq, &MXZmq::gotNewMessage, this, &MainWindow::newMessageReceived);
    connect(_zmq, &MXZmq::gotInvalidPayload, this, &MainWindow::invalidPayloadReceived);
    connect(_zmq, &MXZmq::gotWrongMessage, this, &MainWindow::invalidMessageReceived);
    connect(_zmq, &MXZmq::gotNoMessage, this, &MainWindow::noMessageReceived);

    connect(_sequenceChartsModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item)
    {
        bool onOff = (item->checkState() == Qt::Checked);
        _sequenceCharts[item->text()]->setVisible(onOff);
        ui->SequenceChart->replot();
    });

    connect(_fullChartsModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item)
    {
        bool onOff = (item->checkState() == Qt::Checked);
        _fullCharts[item->text()]->setVisible(onOff);
        ui->FullChart->replot();
    });

    // timer for replotting chart
    _replotTimer->setInterval(20);
    connect(_replotTimer, &QTimer::timeout, this, [=](){
        if (_idle) return;
        ui->SequenceChart->xAxis->rescale(true);
        ui->SequenceChart->yAxis->rescale(true);
        ui->SequenceChart->replot();
        ui->FullChart->xAxis->rescale(true);
        ui->FullChart->yAxis->rescale(true);
        ui->FullChart->replot();
    });
}

MainWindow::~MainWindow()
{
    delete _sequenceChartsModel;
    delete _fullChartsModel;
    delete ui;
}

void MainWindow::saveSettings() {
    _settings.beginGroup("MainWindow");
    _settings.setValue("endpoint", ui->endpointline->text());
    _settings.setValue("topic", ui->topicline->text());
    _settings.setValue("geometry", saveGeometry());
    _settings.endGroup();
}

void MainWindow::loadSettings() {
    _settings.beginGroup("MainWindow");
    auto endpoint = _settings.value("endpoint", DEFAULT_ENDPOINT).toString();
    ui->endpointline->setText(endpoint);

    auto topic = _settings.value("topic", DEFAULT_TOPIC).toString();
    ui->topicline->setText(topic);

    auto geometry = _settings.value("geometry").toByteArray();
    if (geometry.isEmpty()) {
        setGeometry(800, 800, 600, 600);
    } else {
        restoreGeometry(geometry);
    }
    _settings.endGroup();
}

QColor MainWindow::colorSequence(unsigned long int n) {
    if (n >= _palette.count()) n %= _palette.count();
    return _palette.at(n);
}

QColor MainWindow::textColorSequence(unsigned long int n) {
    QColor bg = colorSequence(n), result;
    int h, s, l;
    bg.getHsl(&h, &s, &l);
    if (l < 256 * 0.25) {
        result = QColor("#EEEEEE");     // gray
    } else {
        result = QColor("#000000");     // black
    }
    return result;
}

void MainWindow::appendLogMessage(bool status) {
    ui->logMessageArea->appendPlainText(status ? "Checked" :
                                        "Not checked");
}

void MainWindow::connectButtonToggled(bool checked) {
    if (checked) {
        // ui->logMessageArea->appendPlainText(checked ? "Checked" : "Not checked");
        _zmq->connect();
        ui->connectButton->setText("Disconnect");
        ui->statusbar->showMessage("Connected", 5000);
        ui->endpointline->setEnabled(false);
        ui->topicline->setEnabled(false);
        _replotTimer->start(20);    // ms
    } else {
        _zmq->disconnect();
        ui->connectButton->setText("Connect");
        ui->statusbar->showMessage("Disconnected", 5000);
        ui->endpointline->setEnabled(true);
        ui->topicline->setEnabled(true);
        _replotTimer->stop();
    }
}

void MainWindow::endpointLineEdited(const QString &text) {
    if (_zmq->setEndpoint(text)) {
        ui->connectButton->setEnabled(true);
        ui->endpointline->setStyleSheet("QLineEdit { color: white;}");
    } else {
        ui->connectButton->setEnabled(false);
        ui->endpointline->setStyleSheet("QLineEdit { color: red;}");
    }
}

void MainWindow::newMessageReceived(const QJsonObject &obj) {
    // ui->logMessageArea->appendPlainText(_zmq->payload());
    QJsonValue val;
    _idle = false;
    foreach(auto &k, obj.keys()) {
        val = obj.value(k);
        try {
            // Scalar: add to the lower chart
            if (val.isDouble()) {
                if (!_sequenceCharts.contains(k)) {
                    unsigned long int n = _sequenceCharts.count();
                    QStandardItem *item = new QStandardItem(k);
                    item->setCheckable(true);       // check box
                    item->setCheckState(Qt::Checked);
                    item->setData(QBrush(colorSequence(n)), Qt::BackgroundRole);
                    item->setData(QBrush(textColorSequence(n)), Qt::ForegroundRole);
                    _sequenceChartsModel->appendRow(item);
                    _sequenceCharts[k] = ui->SequenceChart->addGraph();
                    _sequenceCharts[k]->setName(k);
                    _sequenceCharts[k]->setPen(QPen(colorSequence(n)));
                }
                _sequenceCharts[k]->addData(_messageCount, val.toDouble());
            }
            // Array: add to upper chart
            else if (val.isArray()) {
                QJsonArray ary = val.toArray();
                if (ary.count() != 2)
                    throw std::invalid_argument("XY plot data must have two columns");
                if (!ary.at(0).isArray() && !ary.at(1).isArray())
                    throw std::invalid_argument("XY plot data must contain two arrays");
                if (ary.at(0).toArray().count() != ary.at(1).toArray().count())
                    throw std::invalid_argument("XY must have same size");
                if (!_fullCharts.contains(k)) {
                    unsigned long int n = _fullCharts.count();
                    QStandardItem *item = new QStandardItem(k);
                    item->setCheckable(true);
                    item->setCheckState(Qt::Checked);
                    item->setData(QBrush(colorSequence(n)), Qt::BackgroundRole);
                    item->setData(QBrush(textColorSequence(n)), Qt::ForegroundRole);
                    _fullChartsModel->appendRow(item);
                    _fullCharts[k] = new QCPCurve(ui->FullChart->xAxis, ui->FullChart->yAxis);
                    _fullCharts[k]->setName(k);
                    _fullCharts[k]->setPen(QPen(colorSequence(n)));
                }
                unsigned long n = ary.at(0).toArray().count();
                _fullCharts[k]->data().data()->clear();
                for (int i = 0; i < n; i++) {
                    _fullCharts[k]->addData(ary.at(0).toArray().at(i).toDouble(),
                                            ary.at(1).toArray().at(i).toDouble());
                }
            }
            // Unsupported type
            else {
                throw std::invalid_argument(std::string("Unexpected type in JSON object " + k.toStdString()));
            }
        } catch (std::invalid_argument &ex) {
            ui->statusbar->showMessage("Error dealing with message");
            ui->logMessageArea->appendPlainText(ex.what());
        }
    }
    _messageCount++;
    ui->statusbar->clearMessage();
}

void MainWindow::invalidPayloadReceived(const std::invalid_argument &ex, const QString &payload) {
    ui->logMessageArea->appendPlainText(
        QString::asprintf("Invalid JSON: %s\npayload: %s", ex.what(), payload.toStdString().c_str()));
}

void MainWindow::invalidMessageReceived(int parts) {
    ui->logMessageArea->appendPlainText(
        QString::asprintf("Message with wrong number of parts (%d)", parts));
}
void MainWindow::noMessageReceived() {
    // in statusbar (to avoid continuously print when no message comming
    ui->statusbar->showMessage("No incoming messages", SOCKET_TIMEOUT * 2);
    _idle = true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    _zmq->disconnect();
    delete _zmq;
    saveSettings();
}




