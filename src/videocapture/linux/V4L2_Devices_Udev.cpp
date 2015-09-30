#include <libudev.h>
#include <videocapture/linux/V4L2_Devices.h>

namespace ca {


  /* ---------------------------------------------------------------- */

  /* 
     This implementation uses udev to retrieve capture devices.
     We support both udev and default v4l2 ways to retrieve 
     devices. Udev is not supported on all systems.

   */
  std::vector<V4L2_Device> v4l2_get_devices() {
    
    std::vector<V4L2_Device> result;
    struct udev* udev;
    struct udev_enumerate* enumerate;
    struct udev_list_entry* devices;
    struct udev_list_entry* dev_list_entry;
    struct udev_device* dev;
 
    udev = udev_new();
    if(!udev) {
      printf("Error: Cannot udev_new()\n");
      return result;
    }
 
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {

      /* Get the device by syspath. */
      const char* syspath = udev_list_entry_get_name(dev_list_entry);
      dev = udev_device_new_from_syspath(udev, syspath);

      if(!dev) {
        printf("Error: cannot get the device using the syspath: %s\n", syspath);
        continue;
      }

      V4L2_Device v4l2_device;
      v4l2_device.path = udev_device_get_devnode(dev);

      if(v4l2_device.path.size() == 0) {
        printf("Error: Cannot find devpath.\n");
        continue;
      }

      dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

      if(!dev) {
        printf("Error:Cannot find related usb device.\n");
        continue;
      }

      v4l2_device.id_vendor = udev_device_get_sysattr_value(dev, "idVendor");
      v4l2_device.id_product = udev_device_get_sysattr_value(dev, "idProduct");

      result.push_back(v4l2_device);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    
    return result;
  }
  
  /* ---------------------------------------------------------------- */
  
} /* namespace ca */
