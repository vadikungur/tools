#include "mainwindow.h"
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isLogging(false)
    , serialPort(new QSerialPort(this))
    , packetTimer(new QTimer(this))
    , logFile(nullptr)
    , logStream(nullptr)
    , settings(new QSettings("COMLogger", "COMPortLogger", this))
{
    setWindowTitle("COM Port Logger");
    resize(900, 700);
    
    setupUI();
    setupConnections();
    loadSettings();
    
    logMessage("Приложение запущено");
}

MainWindow::~MainWindow()
{
    if (isLogging) {
        onStartStopClicked();
    }
    saveSettings();
    
    if (logStream) {
        delete logStream;
    }
    if (logFile && logFile->isOpen()) {
        logFile->close();
    }
    
    qDeleteAll(statsMap);
    statsMap.clear();
}

void MainWindow::setupUI()
{
    // Central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Settings group
    QGroupBox *settingsGroup = new QGroupBox("Настройки подключения");
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsGroup);
    
    // First row: COM port and baud rate
    QHBoxLayout *row1Layout = new QHBoxLayout();
    
    QLabel *portLabel = new QLabel("COM-порт:");
    portComboBox = new QComboBox();
    updatePortList();
    row1Layout->addWidget(portLabel);
    row1Layout->addWidget(portComboBox);
    
    QLabel *baudLabel = new QLabel("Скорость:");
    baudRateComboBox = new QComboBox();
    baudRateComboBox->addItem("9600", QSerialPort::Baud9600);
    baudRateComboBox->addItem("19200", QSerialPort::Baud19200);
    baudRateComboBox->addItem("38400", QSerialPort::Baud38400);
    baudRateComboBox->addItem("57600", QSerialPort::Baud57600);
    baudRateComboBox->addItem("115200", QSerialPort::Baud115200);
    baudRateComboBox->setCurrentIndex(4); // 115200 by default
    row1Layout->addWidget(baudLabel);
    row1Layout->addWidget(baudRateComboBox);
    
    row1Layout->addStretch();
    settingsLayout->addLayout(row1Layout);
    
    // Second row: Data bits, parity, stop bits
    QHBoxLayout *row2Layout = new QHBoxLayout();
    
    QLabel *dataBitsLabel = new QLabel("Биты данных:");
    dataBitsComboBox = new QComboBox();
    dataBitsComboBox->addItem("5", QSerialPort::Data5);
    dataBitsComboBox->addItem("6", QSerialPort::Data6);
    dataBitsComboBox->addItem("7", QSerialPort::Data7);
    dataBitsComboBox->addItem("8", QSerialPort::Data8);
    dataBitsComboBox->setCurrentIndex(3); // 8 by default
    row2Layout->addWidget(dataBitsLabel);
    row2Layout->addWidget(dataBitsComboBox);
    
    QLabel *parityLabel = new QLabel("Чётность:");
    parityComboBox = new QComboBox();
    parityComboBox->addItem("Нет", QSerialPort::NoParity);
    parityComboBox->addItem("Чётный", QSerialPort::EvenParity);
    parityComboBox->addItem("Нечётный", QSerialPort::OddParity);
    parityComboBox->addItem("Маркер", QSerialPort::MarkParity);
    parityComboBox->addItem("Пробел", QSerialPort::SpaceParity);
    parityComboBox->setCurrentIndex(0); // No parity by default
    row2Layout->addWidget(parityLabel);
    row2Layout->addWidget(parityComboBox);
    
    QLabel *stopBitsLabel = new QLabel("Стоп-биты:");
    stopBitsComboBox = new QComboBox();
    stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
    stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
    stopBitsComboBox->setCurrentIndex(0); // 1 by default
    row2Layout->addWidget(stopBitsLabel);
    row2Layout->addWidget(stopBitsComboBox);
    
    row2Layout->addStretch();
    settingsLayout->addLayout(row2Layout);
    
    // Third row: Packet detection settings
    QHBoxLayout *row3Layout = new QHBoxLayout();
    
    QLabel *pauseTypeLabel = new QLabel("Тип паузы:");
    pauseTypeComboBox = new QComboBox();
    pauseTypeComboBox->addItem("По длительности (мс)");
    pauseTypeComboBox->addItem("По длине символа");
    row3Layout->addWidget(pauseTypeLabel);
    row3Layout->addWidget(pauseTypeComboBox);
    
    QLabel *pauseValueLabel = new QLabel("Значение:");
    pauseValueSpinBox = new QSpinBox();
    pauseValueSpinBox->setRange(1, 10000);
    pauseValueSpinBox->setValue(10);
    pauseValueSpinBox->setSuffix(pauseTypeComboBox->currentIndex() == 0 ? " мс" : " символов");
    row3Layout->addWidget(pauseValueLabel);
    row3Layout->addWidget(pauseValueSpinBox);
    
    crcCheckBox = new QCheckBox("Контроль CRC-16 Modbus");
    row3Layout->addWidget(crcCheckBox);
    
    row3Layout->addStretch();
    settingsLayout->addLayout(row3Layout);
    
    mainLayout->addWidget(settingsGroup);
    
    // Start/Stop button
    startStopButton = new QPushButton("Старт");
    startStopButton->setFixedSize(150, 40);
    startStopButton->setStyleSheet(
        "QPushButton { background-color: green; color: white; font-weight: bold; border-radius: 5px; }"
        "QPushButton:hover { background-color: darkgreen; }"
    );
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(startStopButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Tab widget
    tabWidget = new QTabWidget();
    
    // Statistics tab
    statsTab = new QWidget();
    QVBoxLayout *statsLayout = new QVBoxLayout(statsTab);
    statsTable = new QTableWidget();
    statsTable->setColumnCount(3);
    statsTable->setHorizontalHeaderLabels({"Преамбула", "Длина", "Количество"});
    statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    statsLayout->addWidget(statsTable);
    tabWidget->addTab(statsTab, "Статистика");
    
    // Packets tab
    packetsTab = new QWidget();
    QVBoxLayout *packetsLayout = new QVBoxLayout(packetsTab);
    packetsList = new QListWidget();
    packetsLayout->addWidget(packetsList);
    tabWidget->addTab(packetsTab, "Пакеты");
    
    mainLayout->addWidget(tabWidget);
    
    // Status bar
    statusLabel = new QLabel("Готов к работе");
    statusBar()->addWidget(statusLabel);
}

void MainWindow::setupConnections()
{
    connect(startStopButton, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);
    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::onSerialDataReceived);
    connect(packetTimer, &QTimer::timeout, this, &MainWindow::onPacketTimeout);
    connect(pauseTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        pauseValueSpinBox->setSuffix(index == 0 ? " мс" : " символов");
    });
    connect(portComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (!isLogging) {
            updatePortList();
        }
    });
}

void MainWindow::onStartStopClicked()
{
    if (!isLogging) {
        // Start logging
        QString portName = portComboBox->currentText();
        if (portName.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Выберите COM-порт");
            return;
        }
        
        serialPort->setPortName(portName);
        serialPort->setBaudRate(baudRateComboBox->currentData().value<QSerialPort::BaudRate>());
        serialPort->setDataBits(dataBitsComboBox->currentData().value<QSerialPort::DataBits>());
        serialPort->setParity(parityComboBox->currentData().value<QSerialPort::Parity>());
        serialPort->setStopBits(stopBitsComboBox->currentData().value<QSerialPort::StopBits>());
        
        if (!serialPort->open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Ошибка", 
                QString("Не удалось открыть порт %1: %2").arg(portName, serialPort->errorString()));
            logMessage(QString("Ошибка открытия порта: %1").arg(serialPort->errorString()));
            return;
        }
        
        openLogFile();
        
        receiveBuffer.clear();
        isLogging = true;
        
        startStopButton->setText("Стоп");
        startStopButton->setStyleSheet(
            "QPushButton { background-color: red; color: white; font-weight: bold; border-radius: 5px; }"
            "QPushButton:hover { background-color: darkred; }"
        );
        
        // Calculate timeout based on pause type
        int pauseValue = pauseValueSpinBox->value();
        int timeoutMs;
        if (pauseTypeComboBox->currentIndex() == 0) {
            // By duration in ms
            timeoutMs = pauseValue;
        } else {
            // By character length
            int baudRate = baudRateComboBox->currentData().toInt();
            // Approximate: 10 bits per character (start + data + parity + stop)
            int bitsPerChar = 10;
            int charTimeMs = (bitsPerChar * 1000) / baudRate;
            timeoutMs = pauseValue * charTimeMs;
        }
        packetTimer->start(qMax(timeoutMs, 10)); // Minimum 10ms
        
        portComboBox->setEnabled(false);
        baudRateComboBox->setEnabled(false);
        dataBitsComboBox->setEnabled(false);
        parityComboBox->setEnabled(false);
        stopBitsComboBox->setEnabled(false);
        pauseTypeComboBox->setEnabled(false);
        pauseValueSpinBox->setEnabled(false);
        crcCheckBox->setEnabled(false);
        
        logMessage(QString("Логирование начато. Порт: %1, Файл: %2").arg(portName, currentFileName));
        statusBar()->showMessage("Идёт запись данных");
        
    } else {
        // Stop logging
        packetTimer->stop();
        
        // Process any remaining data in buffer
        if (!receiveBuffer.isEmpty()) {
            writePacketToFile(receiveBuffer);
            addPacketToStats(receiveBuffer);
            addPacketToList(receiveBuffer);
            receiveBuffer.clear();
        }
        
        if (logStream) {
            delete logStream;
            logStream = nullptr;
        }
        if (logFile && logFile->isOpen()) {
            logFile->close();
        }
        
        serialPort->close();
        isLogging = false;
        
        startStopButton->setText("Старт");
        startStopButton->setStyleSheet(
            "QPushButton { background-color: green; color: white; font-weight: bold; border-radius: 5px; }"
            "QPushButton:hover { background-color: darkgreen; }"
        );
        
        portComboBox->setEnabled(true);
        baudRateComboBox->setEnabled(true);
        dataBitsComboBox->setEnabled(true);
        parityComboBox->setEnabled(true);
        stopBitsComboBox->setEnabled(true);
        pauseTypeComboBox->setEnabled(true);
        pauseValueSpinBox->setEnabled(true);
        crcCheckBox->setEnabled(true);
        
        logMessage("Логирование остановлено");
        statusBar()->showMessage("Запись остановлена", 3000);
    }
}

void MainWindow::onSerialDataReceived()
{
    if (!isLogging) return;
    
    QByteArray data = serialPort->readAll();
    receiveBuffer.append(data);
    
    // Reset timer on each data reception
    packetTimer->start();
}

void MainWindow::onPacketTimeout()
{
    if (!isLogging || receiveBuffer.isEmpty()) return;
    
    QByteArray packet = receiveBuffer;
    receiveBuffer.clear();
    
    // Validate CRC if enabled
    if (crcCheckBox->isChecked()) {
        if (packet.size() < 2) {
            logMessage("Пакет слишком короткий для CRC проверки");
            return;
        }
        if (!validatePacketCRC(packet)) {
            logMessage("Ошибка CRC в пакете");
            // Still log the packet but mark it as invalid
        }
    }
    
    writePacketToFile(packet);
    addPacketToStats(packet);
    addPacketToList(packet);
}

void MainWindow::updatePortList()
{
    QString currentPort = portComboBox->currentText();
    portComboBox->clear();
    
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        portComboBox->addItem(port.portName());
    }
    
    if (ports.isEmpty()) {
        portComboBox->addItem("Нет доступных портов");
    }
    
    // Try to restore previous selection
    int index = portComboBox->findText(currentPort);
    if (index >= 0) {
        portComboBox->setCurrentIndex(index);
    }
}

void MainWindow::loadSettings()
{
    settings->beginGroup("SerialPort");
    
    QString portName = settings->value("PortName", "").toString();
    int baudRateIndex = settings->value("BaudRateIndex", 4).toInt();
    int dataBitsIndex = settings->value("DataBitsIndex", 3).toInt();
    int parityIndex = settings->value("ParityIndex", 0).toInt();
    int stopBitsIndex = settings->value("StopBitsIndex", 0).toInt();
    int pauseTypeIndex = settings->value("PauseTypeIndex", 0).toInt();
    int pauseValue = settings->value("PauseValue", 10).toInt();
    bool crcEnabled = settings->value("CRCEnabled", false).toBool();
    
    settings->endGroup();
    
    if (!portName.isEmpty()) {
        int index = portComboBox->findText(portName);
        if (index >= 0) {
            portComboBox->setCurrentIndex(index);
        }
    }
    
    baudRateComboBox->setCurrentIndex(qMin(baudRateIndex, baudRateComboBox->count() - 1));
    dataBitsComboBox->setCurrentIndex(qMin(dataBitsIndex, dataBitsComboBox->count() - 1));
    parityComboBox->setCurrentIndex(qMin(parityIndex, parityComboBox->count() - 1));
    stopBitsComboBox->setCurrentIndex(qMin(stopBitsIndex, stopBitsComboBox->count() - 1));
    pauseTypeComboBox->setCurrentIndex(qMin(pauseTypeIndex, pauseTypeComboBox->count() - 1));
    pauseValueSpinBox->setValue(pauseValue);
    crcCheckBox->setChecked(crcEnabled);
}

void MainWindow::saveSettings()
{
    settings->beginGroup("SerialPort");
    
    settings->setValue("PortName", portComboBox->currentText());
    settings->setValue("BaudRateIndex", baudRateComboBox->currentIndex());
    settings->setValue("DataBitsIndex", dataBitsComboBox->currentIndex());
    settings->setValue("ParityIndex", parityComboBox->currentIndex());
    settings->setValue("StopBitsIndex", stopBitsComboBox->currentIndex());
    settings->setValue("PauseTypeIndex", pauseTypeComboBox->currentIndex());
    settings->setValue("PauseValue", pauseValueSpinBox->value());
    settings->setValue("CRCEnabled", crcCheckBox->isChecked());
    
    settings->endGroup();
}

quint16 MainWindow::calculateCRC16Modbus(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<quint8>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

bool MainWindow::validatePacketCRC(const QByteArray &packet)
{
    if (packet.size() < 2) return false;
    
    // Last two bytes are CRC (little-endian)
    QByteArray dataWithoutCrc = packet.left(packet.size() - 2);
    quint16 receivedCrc = static_cast<quint8>(packet[packet.size() - 2]) |
                         (static_cast<quint8>(packet[packet.size() - 1]) << 8);
    
    quint16 calculatedCrc = calculateCRC16Modbus(dataWithoutCrc);
    
    return receivedCrc == calculatedCrc;
}

QByteArray MainWindow::appendCRC(const QByteArray &data)
{
    QByteArray result = data;
    quint16 crc = calculateCRC16Modbus(data);
    result.append(static_cast<char>(crc & 0xFF));
    result.append(static_cast<char>((crc >> 8) & 0xFF));
    return result;
}

void MainWindow::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    statusLabel->setText(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::addPacketToStats(const QByteArray &packet)
{
    if (packet.isEmpty()) return;
    
    // Get preamble (first 2 bytes)
    QString preamble;
    if (packet.size() >= 2) {
        preamble = QString("%1 %2")
            .arg(static_cast<unsigned char>(packet[0]), 2, 16, QChar('0'))
            .arg(static_cast<unsigned char>(packet[1]), 2, 16, QChar('0'));
    } else if (packet.size() == 1) {
        preamble = QString("%1").arg(static_cast<unsigned char>(packet[0]), 2, 16, QChar('0'));
    }
    
    int length = packet.size();
    QString key = QString("%1_%2").arg(preamble).arg(length);
    
    if (!statsMap.contains(key)) {
        PacketStats *stats = new PacketStats();
        stats->preamble = preamble;
        stats->length = length;
        stats->count = 0;
        statsMap[key] = stats;
        
        // Add new row to table
        int row = statsTable->rowCount();
        statsTable->insertRow(row);
        statsTable->setItem(row, 0, new QTableWidgetItem(preamble));
        statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(length)));
        statsTable->setItem(row, 2, new QTableWidgetItem("0"));
    }
    
    statsMap[key]->count++;
    
    // Find row and update count
    for (int row = 0; row < statsTable->rowCount(); ++row) {
        QTableWidgetItem *preambleItem = statsTable->item(row, 0);
        QTableWidgetItem *lengthItem = statsTable->item(row, 1);
        if (preambleItem && lengthItem && 
            preambleItem->text() == preamble && 
            lengthItem->text().toInt() == length) {
            
            statsTable->item(row, 2)->setText(QString::number(statsMap[key]->count));
            highlightStatsRow(row);
            break;
        }
    }
}

void MainWindow::highlightStatsRow(int row)
{
    // Highlight row yellow
    for (int col = 0; col < statsTable->columnCount(); ++col) {
        QTableWidgetItem *item = statsTable->item(row, col);
        if (item) {
            item->setBackground(QColor(255, 255, 0)); // Yellow
        }
    }
    
    // Schedule removal of highlight after 200ms
    QTimer::singleShot(200, this, [this, row]() {
        for (int col = 0; col < statsTable->columnCount(); ++col) {
            QTableWidgetItem *item = statsTable->item(row, col);
            if (item) {
                item->setBackground(QColor()); // Clear background
            }
        }
    });
}

void MainWindow::addPacketToList(const QByteArray &packet)
{
    if (packet.isEmpty()) return;
    
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    
    // Convert to hex string with spaces
    QStringList hexParts;
    for (int i = 0; i < packet.size(); ++i) {
        hexParts << QString("%1").arg(static_cast<unsigned char>(packet[i]), 2, 16, QChar('0'));
    }
    QString hexString = hexParts.join(" ");
    
    packetsList->addItem(QString("[%1] %2").arg(timestamp, hexString));
    packetsList->scrollToBottom();
}

void MainWindow::openLogFile()
{
    if (logStream) {
        delete logStream;
        logStream = nullptr;
    }
    if (logFile && logFile->isOpen()) {
        logFile->close();
    }
    
    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    currentFileName = QString("com_log_%1.txt").arg(dateTime);
    
    logFile = new QFile(currentFileName, this);
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", 
            QString("Не удалось создать файл лога: %1").arg(logFile->errorString()));
        logMessage("Ошибка создания файла лога");
        delete logFile;
        logFile = nullptr;
        return;
    }
    
    logStream = new QTextStream(logFile);
    logMessage(QString("Файл лога создан: %1").arg(currentFileName));
}

void MainWindow::writePacketToFile(const QByteArray &packet)
{
    if (!logStream) return;
    
    // Convert to hex string with spaces
    QStringList hexParts;
    for (int i = 0; i < packet.size(); ++i) {
        hexParts << QString("%1").arg(static_cast<unsigned char>(packet[i]), 2, 16, QChar('0'));
    }
    QString hexString = hexParts.join(" ");
    
    *logStream << hexString << "\n";
    logStream->flush();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isLogging) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Подтверждение", 
            "Логирование активно. Остановить и выйти?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            onStartStopClicked();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        saveSettings();
        event->accept();
    }
}
