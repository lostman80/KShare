#include "recordingformats.hpp"

#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTimer>
#include <formats.hpp>
#include <gif-h/gif.h>
#include <platformbackend.hpp>
#include <settings.hpp>
#include <time.h>
#include <unistd.h>

RecordingFormats::RecordingFormats(formats::Recording f) {
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    if (tmp.isEmpty()) {
        validator = [](QSize) { return false; };
        return;
    }
    tmpDir = QDir(tmp);
    QString name
    = QString("KShareTemp-") + QString::number(PlatformBackend::inst().pid()) + "-" + QTime::currentTime().toString();
    tmpDir.mkdir(name);
    tmpDir.cd(name);
    iFormat = QImage::Format_RGB888;
    path = tmpDir.absoluteFilePath("res." + formats::recordingFormatName(f).toLower());
    finalizer = [&] {
        delete enc;
        QFile res(path);
        if (!res.open(QFile::ReadOnly)) {
            return QByteArray();
        }
        QByteArray data = res.readAll();
        tmpDir.removeRecursively();
        QScopedPointer<RecordingFormats>(this);
        return data;
    };
    validator = [&](QSize s) {
        if (!enc) {
            enc = new Encoder(path, s);
            if (!enc->isRunning()) {
                delete enc;
                return false;
            }
        }
        return true;
    };
    consumer = [&](QImage img) { enc->addFrame(img); };
    anotherFormat = formats::recordingFormatName(f);
}

std::function<void(QImage)> RecordingFormats::getConsumer() {
    return consumer;
}

std::function<QByteArray()> RecordingFormats::getFinalizer() {
    return finalizer;
}

std::function<bool(QSize)> RecordingFormats::getValidator() {
    return validator;
}

QImage::Format RecordingFormats::getFormat() {
    return iFormat;
}

QString RecordingFormats::getAnotherFormat() {
    return anotherFormat;
}
