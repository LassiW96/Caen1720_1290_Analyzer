#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QLabel>
#include <QGroupBox>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <TFile.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TKey.h>
#include <TROOT.h>
#include <TSystem.h>

#include "V1720EvioDecode.h"
#include <iostream>
#include <thread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("V1720 Decoding & Analysis UI");
    resize(1000, 700);
    setupUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    m_tabWidget = new QTabWidget();
    mainLayout->addWidget(m_tabWidget);

    setupDecodeTab();
    setupAnalysisTab();
}

void MainWindow::setupDecodeTab() {
    QWidget *decodeTab = new QWidget();
    QVBoxLayout *decodeLayout = new QVBoxLayout(decodeTab);
    
    QGroupBox *inputGroup = new QGroupBox("Input Configuration");
    QGridLayout *inputLayout = new QGridLayout(inputGroup);
    
    m_inputFileEdit = new QLineEdit();
    QPushButton *browseInputBtn = new QPushButton("Browse...");
    connect(browseInputBtn, &QPushButton::clicked, this, &MainWindow::browseInputFile);
    
    m_outputDirEdit = new QLineEdit();
    QPushButton *browseOutputBtn = new QPushButton("Browse...");
    connect(browseOutputBtn, &QPushButton::clicked, this, &MainWindow::browseOutputDir);

    inputLayout->addWidget(new QLabel("EVIO File:"), 0, 0);
    inputLayout->addWidget(m_inputFileEdit, 0, 1);
    inputLayout->addWidget(browseInputBtn, 0, 2);
    
    inputLayout->addWidget(new QLabel("Output Dir:"), 1, 0);
    inputLayout->addWidget(m_outputDirEdit, 1, 1);
    inputLayout->addWidget(browseOutputBtn, 1, 2);
    
    decodeLayout->addWidget(inputGroup);
    
    m_decodeBtn = new QPushButton("Start Decode");
    m_decodeBtn->setMinimumHeight(40);
    connect(m_decodeBtn, &QPushButton::clicked, this, &MainWindow::startDecode);
    decodeLayout->addWidget(m_decodeBtn);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 0); // Indeterminate
    m_progressBar->setVisible(false);
    decodeLayout->addWidget(m_progressBar);
    
    m_logArea = new QTextEdit();
    m_logArea->setReadOnly(true);
    decodeLayout->addWidget(new QLabel("Logs:"));
    decodeLayout->addWidget(m_logArea);
    
    m_tabWidget->addTab(decodeTab, "Decode");
}

void MainWindow::setupAnalysisTab() {
    QWidget *analysisTab = new QWidget();
    QHBoxLayout *analysisLayout = new QHBoxLayout(analysisTab);
    
    // Left Panel: File Selection & Scripting
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    QGroupBox *fileGroup = new QGroupBox("ROOT File");
    QHBoxLayout *fileLayout = new QHBoxLayout(fileGroup);
    m_analysisFileEdit = new QLineEdit();
    QPushButton *browseAnalysisBtn = new QPushButton("Browse...");
    connect(browseAnalysisBtn, &QPushButton::clicked, this, &MainWindow::browseAnalysisFile);
    fileLayout->addWidget(m_analysisFileEdit);
    fileLayout->addWidget(browseAnalysisBtn);
    leftLayout->addWidget(fileGroup);
    
    QGroupBox *scriptGroup = new QGroupBox("Analysis Script");
    QVBoxLayout *scriptLayout = new QVBoxLayout(scriptGroup);
    m_scriptEditor = new QTextEdit();
    m_scriptEditor->setPlaceholderText("// Enter ROOT C++ script here...\n// e.g.\n// TH1F *h = (TH1F*)gDirectory->Get(\"h1\");\n// h->Draw();");
    
    QHBoxLayout *scriptBtnLayout = new QHBoxLayout();
    QPushButton *runScriptBtn = new QPushButton("Run Script");
    QPushButton *loadScriptBtn = new QPushButton("Load Script...");
    QPushButton *loadExampleBtn = new QPushButton("Load Example");
    
    connect(runScriptBtn, &QPushButton::clicked, this, &MainWindow::runAnalysisScript);
    connect(loadScriptBtn, &QPushButton::clicked, this, &MainWindow::loadAnalysisScript);
    connect(loadExampleBtn, &QPushButton::clicked, this, &MainWindow::loadExampleScript);
    
    scriptBtnLayout->addWidget(loadScriptBtn);
    scriptBtnLayout->addWidget(loadExampleBtn);
    scriptBtnLayout->addWidget(runScriptBtn);
    
    scriptLayout->addWidget(m_scriptEditor);
    scriptLayout->addLayout(scriptBtnLayout);
    leftLayout->addWidget(scriptGroup);
    
    // Right Panel: Histogram List & Preview
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    m_histList = new QListWidget();
    connect(m_histList, &QListWidget::itemClicked, this, &MainWindow::onHistogramSelected);
    
    m_previewLabel = new QLabel("Select a histogram to preview");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    m_previewLabel->setMinimumSize(400, 300);
    // Scale contents
    m_previewLabel->setScaledContents(true);

    QPushButton *openInteractiveBtn = new QPushButton("Open Interactive Canvas");
    connect(openInteractiveBtn, &QPushButton::clicked, this, &MainWindow::openInteractiveCanvas);

    rightLayout->addWidget(new QLabel("Histograms in File:"));
    rightLayout->addWidget(m_histList);
    rightLayout->addWidget(new QLabel("Preview:"));
    rightLayout->addWidget(m_previewLabel);
    rightLayout->addWidget(openInteractiveBtn);
    
    // Adjust stretch
    analysisLayout->addWidget(leftPanel, 1);
    analysisLayout->addWidget(rightPanel, 1);
    
    m_tabWidget->addTab(analysisTab, "Analysis & Visualization");
}

void MainWindow::browseInputFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select EVIO File", "", "EVIO Files (*.evio *.evio.*)");
    if (!fileName.isEmpty()) {
        m_inputFileEdit->setText(fileName);
    }
}

void MainWindow::browseOutputDir() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
    if (!dir.isEmpty()) {
        m_outputDirEdit->setText(dir);
    }
}

void MainWindow::startDecode() {
    QString inputFile = m_inputFileEdit->text();
    QString outputDir = m_outputDirEdit->text();
    
    if (inputFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an input file.");
        return;
    }
    
    m_decodeBtn->setEnabled(false);
    m_progressBar->setVisible(true);
    m_logArea->append("Starting decoding...");
    
    std::thread([this, inputFile, outputDir]() {
        try {
            V1720EvioDecode decoder(inputFile.toStdString(), outputDir.toStdString());
            decoder.decode();
            
            QMetaObject::invokeMethod(this, [this, outputDir, inputFile]() {
                m_logArea->append("Decoding finished successfully.");
                
                // Print Branch Summary
                std::filesystem::path p(inputFile.toStdString());
                while (p.has_extension()) p = p.stem();
                std::string outputFileName = p.string() + "_decoded.root";
                std::filesystem::path fullPath = std::filesystem::path(outputDir.toStdString()) / outputFileName;
                if (outputDir.isEmpty()) fullPath = outputFileName;

                TFile *f = TFile::Open(fullPath.string().c_str(), "READ");
                if (f && !f->IsZombie()) {
                    TTree *t = (TTree*)f->Get("t1");
                    if (t) {
                        m_logArea->append("\n--- Root File Branch Summary ---");
                        TObjArray *branches = t->GetListOfBranches();
                        for (int i = 0; i < branches->GetEntries(); ++i) {
                            TBranch *b = (TBranch*)branches->At(i);
                            m_logArea->append(QString("Branch: %1  (Title: %2)").arg(b->GetName()).arg(b->GetTitle()));
                        }
                        m_logArea->append("--------------------------------\n");
                    }
                    f->Close();
                    delete f;
                }

                m_decodeBtn->setEnabled(true);
                m_progressBar->setVisible(false);
                QMessageBox::information(this, "Success", "Decoding Completed!");
            }, Qt::QueuedConnection);
            
        } catch (const std::exception &e) {
            QString errorMsg = QString::fromStdString(e.what());
            QMetaObject::invokeMethod(this, [this, errorMsg]() {
                m_logArea->append("Error: " + errorMsg);
                m_decodeBtn->setEnabled(true);
                m_progressBar->setVisible(false);
                QMessageBox::critical(this, "Error", "Decoding Failed:\n" + errorMsg);
            }, Qt::QueuedConnection);
        }
    }).detach();
}


void MainWindow::browseAnalysisFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select ROOT File", "", "ROOT Files (*.root)");
    if (!fileName.isEmpty()) {
        m_analysisFileEdit->setText(fileName);
        m_currentRootFile = fileName;
        
        m_histList->clear();
        TFile *f = TFile::Open(fileName.toStdString().c_str(), "READ");
        if (f && !f->IsZombie()) {
             TIter next(f->GetListOfKeys());
             TKey *key;
             while ((key = (TKey*)next())) {
                 TClass *cl = gROOT->GetClass(key->GetClassName());
                 if (cl->InheritsFrom("TH1")) {
                     m_histList->addItem(key->GetName());
                 }
             }
             f->Close();
             delete f;
        } else {
             if (f) delete f;
        }
    }
}

void MainWindow::onHistogramSelected(QListWidgetItem *item) {
    if (m_currentRootFile.isEmpty()) return;
    
    QString histName = item->text();
    
    QString tempImg = QDir::tempPath() + "/v1720_preview.png";
    
    TFile *f = TFile::Open(m_currentRootFile.toStdString().c_str(), "READ");
    if (!f || f->IsZombie()) return;
    
    TH1 *h = (TH1*)f->Get(histName.toStdString().c_str());
    if (h) {
        gROOT->SetBatch(kTRUE);
        TCanvas *c = new TCanvas("c_preview", "Preview", 800, 600);
        h->Draw();
        c->SaveAs(tempImg.toStdString().c_str());
        delete c;
        gROOT->SetBatch(kFALSE);
        
        QPixmap pix(tempImg);
        m_previewLabel->setPixmap(pix);
    }
    
    f->Close();
    delete f;
}

void MainWindow::openInteractiveCanvas() {
    QList<QListWidgetItem*> items = m_histList->selectedItems();
    if (items.isEmpty() || m_currentRootFile.isEmpty()) return;
    
    QString histName = items.first()->text();
    
    auto *f = new TFile(m_currentRootFile.toStdString().c_str(), "READ"); 
    TH1 *h = (TH1*)f->Get(histName.toStdString().c_str());
    if (h) {
         h->SetDirectory(0);
         TCanvas *c = new TCanvas("c_interactive", histName.toStdString().c_str(), 800, 600);
         h->Draw();
         c->Update();
    }
}

void MainWindow::loadAnalysisScript() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Script", "", "C++ Macros (*.C *.cxx)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
             m_scriptEditor->setText(file.readAll());
        }
    }
}

void MainWindow::runAnalysisScript() {
    QString script = m_scriptEditor->toPlainText();
    if (script.isEmpty()) return;

    // Save to temp file to run
    QString tempScript = QDir::currentPath() + "/temp_analysis.C";
    QFile file(tempScript);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // If users provided a named function, it needs to match filename for simple .x execution
        if (!script.contains("temp_analysis")) {
             if (!script.contains("void ") && !script.contains("int ")) {
                 out << "void temp_analysis() {\n" << script << "\n}\n";
             } else {
                 out << script;
             }
        } else {
             out << script;
        }
        file.close();

        m_logArea->append("Running script...");

        // Enable batch mode so ROOT renders to an offscreen canvas
        gROOT->SetBatch(kTRUE);

        // Open the current ROOT file so the script can find "t1" in gDirectory
        TFile *f = nullptr;
        if (!m_currentRootFile.isEmpty()) {
            f = TFile::Open(m_currentRootFile.toStdString().c_str(), "READ");
        }

        // Run the script via gROOT
        gROOT->ProcessLine(QString(".x " + tempScript).toStdString().c_str());

        // Capture the active canvas (created by the script) as a PNG for the preview
        QString tempImg = QDir::tempPath() + "/v1720_script_preview.png";
        TCanvas *activeCanvas = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c_ex");
        if (activeCanvas) {
            activeCanvas->SaveAs(tempImg.toStdString().c_str());
            QPixmap pix(tempImg);
            if (!pix.isNull()) {
                m_previewLabel->setPixmap(pix.scaled(
                    m_previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            // Clean up the canvas
            delete activeCanvas;
            m_logArea->append("Histogram rendered in preview.");
        } else {
            m_logArea->append("Warning: No canvas 'c_ex' was produced by the script.");
        }

        gROOT->SetBatch(kFALSE);

        if (f) {
            f->Close();
            delete f;
        }

        m_logArea->append("Script finished.");
    }
}

void MainWindow::loadExampleScript() {
    // Search for resources/temp_script.C relative to the executable or source tree
    QStringList searchPaths;
    QString appDir = QCoreApplication::applicationDirPath();
    searchPaths << appDir + "/../resources/temp_script.C"   // build/ -> source root
                << appDir + "/resources/temp_script.C"       // installed layout
                << QDir::currentPath() + "/resources/temp_script.C";

    QString scriptPath;
    for (const QString &path : searchPaths) {
        if (QFile::exists(path)) {
            scriptPath = path;
            break;
        }
    }

    if (scriptPath.isEmpty()) {
        QMessageBox::warning(this, "Not Found",
            "Could not locate resources/temp_script.C.\n"
            "Searched relative to executable and working directory.");
        return;
    }

    QFile file(scriptPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_scriptEditor->setText(file.readAll());
        m_logArea->append("Loaded example script from: " + scriptPath);
    } else {
        QMessageBox::warning(this, "Error", "Failed to open: " + scriptPath);
    }
}
