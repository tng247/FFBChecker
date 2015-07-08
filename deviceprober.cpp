#include "deviceprober.h"
#include "ffbdevice.h"
#include <QtCore/QDebug>
#include <QtWidgets/QMessageBox>
#include <linux/input.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const QString DeviceProber::DEVICE_NODES_PATH("/dev/input");
const QString DeviceProber::res_ffbdeviceErrCap("FFB Device error");

DeviceProber::DeviceProber(QObject* parent) :
  QObject(parent)
{
}

DeviceProber::DeviceList DeviceProber::listDevices()
{
  DeviceProber::DeviceList list;
  char deviceName[64];
  QDir devDir(DEVICE_NODES_PATH);
  //QStringList devices = DeviceProber::s_deviceNodesPath.entryList(QDir::NoDotAndDotDot);
  QStringList devices = devDir.entryList(QDir::System);

  deviceName[63] = '\0';
  for (const QString& d : devices) {
    int fd, ret;
    DeviceInfo dinfo;
    QString devicePath = devDir.absoluteFilePath(d);

    fd = open(devicePath.toLocal8Bit(), O_RDWR);
    if (fd == -1) {
      qDebug() << "Device" << d << "unaccessible" << strerror(errno);
      continue;
    }

    dinfo.path = devicePath;
    ret = ioctl(fd, EVIOCGNAME(63), deviceName);
    if (ret < 0)
      qDebug() << "Cannot get name of device" << d << strerror(errno);
    else
      dinfo.name = deviceName;

    list.append(dinfo);
  }

  return list;
}

std::shared_ptr<FFBDevice> DeviceProber::openDevice(const QString& path)
{
  /* Check if the device is already opened */
  for (std::shared_ptr<FFBDevice> dev : m_openedDevices) {
    if (QString::compare(path, dev->path()) == 0) {
      qDebug() << "Device" << path << "already opened";
      return dev;
    }
  }

  int fd = open(path.toLocal8Bit(), O_RDWR);
  if (!fd) {
    QMessageBox::critical(nullptr, res_ffbdeviceErrCap, "Cannot open device.");
    return nullptr;
  }

  int maxEffectCount;
  int ret = ioctl(fd, EVIOCGEFFECTS, &maxEffectCount);
  if (ret < 0) {
    QMessageBox::critical(nullptr, res_ffbdeviceErrCap, "Cannot query maximum effects count.\nDevice probably does not support Force Feedback (errno " + QString::number(ret) + ")");
    close(fd);
    return nullptr;
  }
  if (maxEffectCount < 1) {
    QMessageBox::critical(nullptr, res_ffbdeviceErrCap, "Maximum effect count for this device is zero.");
    close(fd);
    return nullptr;
  }

  std::shared_ptr<FFBDevice> device(new FFBDevice(fd, path, maxEffectCount));
  if (!device->queryDeviceCapabilities()) {
    QMessageBox::critical(nullptr, res_ffbdeviceErrCap, "Unable to query device capabilities.");
    return nullptr;
  }

  m_openedDevices.push_back(device);
  return device;
}
