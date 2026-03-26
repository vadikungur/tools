#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QSettings>
#include <QMap>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTableWidget>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QTabWidget>
#include <QStatusBar>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>

struct PacketStats {
    QString preamble;
    int length;
    int count;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onStartStopClicked();
    void onSerialDataReceived();
    void onPacketTimeout();
    void updatePortList();
    void loadSettings();
    void saveSettings();

private:
    void setupUI();
    void setupConnections();
    quint16 calculateCRC16Modbus(const QByteArray &data);
    bool validatePacketCRC(const QByteArray &packet);
    QByteArray appendCRC(const QByteArray &data);
    void logMessage(const QString &message);
    void addPacketToStats(const QByteArray &packet);
    void addPacketToList(const QByteArray &packet);
    void openLogFile();
    void writePacketToFile(const QByteArray &packet);
    void highlightStatsRow(int row);

    // UI Components
    QTabWidget *tabWidget;
    QWidget *statsTab;
    QWidget *packetsTab;
    
    QTableWidget *statsTable;
    QListWidget *packetsList;
    
    QPushButton *startStopButton;
    QComboBox *portComboBox;
    QComboBox *baudRateComboBox;
    QComboBox *dataBitsComboBox;
    QComboBox *parityComboBox;
    QComboBox *stopBitsComboBox;
    QComboBox *pauseTypeComboBox;
    QSpinBox *pauseValueSpinBox;
    QCheckBox *crcCheckBox;
    
    QLabel *statusLabel;
    
    // Serial port
    QSerialPort *serialPort;
    
    // Timer for packet detection
    QTimer *packetTimer;
    
    // Data buffer
    QByteArray receiveBuffer;
    
    // Log file
    QFile *logFile;
    QTextStream *logStream;
    
    // Statistics
    QMap<QString, PacketStats*> statsMap;
    
    // Settings
    QSettings *settings;
    
    QString currentFileName;
    bool isLogging;
};

#endif // MAINWINDOW_H
