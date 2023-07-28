#include "properties.h"
#include "ui_properties.h"
#include <initguid.h>
#include <cfg.h>
#include "glue.h"
#include "device.h"

Properties::Properties(QWidget *parent, const device& a_device) :
  QDialog(parent),
  the_device(a_device),
  ui(new Ui::Properties)
{
  Glue g;
  ui->setupUi(this);
  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

  ui->the_icon->setPixmap(g.IconFromPath(the_device.icon,true,false,false));
  ui->the_description->setText(the_device.description);
  ui->the_manufacturer->setText(the_device.manufacturer.length() ? the_device.manufacturer : tr("Unknown"));

  ui->the_device_type->setText(the_device.device_type);
  ui->the_device_status->setText(Status(the_device.problem_code));
  setWindowTitle(QString("%1 Properties").arg(the_device.description));
}

Properties::~Properties()
{
    delete ui;
}

QString Properties::Status(uint32_t a_code)
{
  QString txt;
  switch (a_code)
  { // TODO, fill out additional texts 
    case 0: return(tr("This device is working properly."));
    case CM_PROB_NOT_CONFIGURED:              txt = "CM_PROB_NOT_CONFIGURED"; break;    // no config for device
    case CM_PROB_DEVLOADER_FAILED:            txt = "CM_PROB_DEVLOADER_FAILED"; break;    // service load failed
    case CM_PROB_OUT_OF_MEMORY:               txt = "CM_PROB_OUT_OF_MEMORY"; break;    // out of memory
    case CM_PROB_ENTRY_IS_WRONG_TYPE:         txt = "CM_PROB_ENTRY_IS_WRONG_TYPE"; break;    //
    case CM_PROB_LACKED_ARBITRATOR:           txt = "CM_PROB_LACKED_ARBITRATOR"; break;    //
    case CM_PROB_BOOT_CONFIG_CONFLICT:        txt = "CM_PROB_BOOT_CONFIG_CONFLICT"; break;    // boot config conflict
    case CM_PROB_FAILED_FILTER:               txt = "CM_PROB_FAILED_FILTER"; break;    //
    case CM_PROB_DEVLOADER_NOT_FOUND:         txt = "CM_PROB_DEVLOADER_NOT_FOUND"; break;    // Devloader not found
    case CM_PROB_INVALID_DATA:                txt = "CM_PROB_INVALID_DATA"; break;    // Invalid ID
    case CM_PROB_FAILED_START:                txt = "CM_PROB_FAILED_START"; break;    //
    case CM_PROB_LIAR:                        txt = "CM_PROB_LIAR"; break;    //
    case CM_PROB_NORMAL_CONFLICT:             txt = "CM_PROB_NORMAL_CONFLICT"; break;    // config conflict
    case CM_PROB_NOT_VERIFIED:                txt = "CM_PROB_NOT_VERIFIED"; break;    //
    case CM_PROB_NEED_RESTART:                txt = "CM_PROB_NEED_RESTART"; break;    // requires restart
    case CM_PROB_REENUMERATION:               txt = "CM_PROB_REENUMERATION"; break;    //
    case CM_PROB_PARTIAL_LOG_CONF:            txt = "CM_PROB_PARTIAL_LOG_CONF"; break;    //
    case CM_PROB_UNKNOWN_RESOURCE:            txt = "CM_PROB_UNKNOWN_RESOURCE"; break;    // unknown res type
    case CM_PROB_REINSTALL:                   txt = "CM_PROB_REINSTALL"; break;    //
    case CM_PROB_REGISTRY:                    txt = "CM_PROB_REGISTRY"; break;    //
    case CM_PROB_VXDLDR:                      txt = "CM_PROB_VXDLDR"; break;    // WINDOWS 95 ONLY
    case CM_PROB_WILL_BE_REMOVED:             txt = "CM_PROB_WILL_BE_REMOVED"; break;    // devinst will remove
    case CM_PROB_DISABLED:                    txt = tr("This device is disabled."); break;    // devinst is disabled
    case CM_PROB_DEVLOADER_NOT_READY:         txt = "CM_PROB_DEVLOADER_NOT_READY"; break;    // Devloader not ready
    case CM_PROB_DEVICE_NOT_THERE:            txt = "CM_PROB_DEVICE_NOT_THERE"; break;    // device doesn't exist
    case CM_PROB_MOVED:                       txt = "CM_PROB_MOVED"; break;    //
    case CM_PROB_TOO_EARLY:                   txt = "CM_PROB_TOO_EARLY"; break;    //
    case CM_PROB_NO_VALID_LOG_CONF:           txt = "CM_PROB_NO_VALID_LOG_CONF"; break;    // no valid log config
    case CM_PROB_FAILED_INSTALL:              txt = "CM_PROB_FAILED_INSTALL"; break;    // install failed
    case CM_PROB_HARDWARE_DISABLED:           txt = "CM_PROB_HARDWARE_DISABLED"; break;    // device disabled
    case CM_PROB_CANT_SHARE_IRQ:              txt = "CM_PROB_CANT_SHARE_IRQ"; break;    // can't share IRQ
    case CM_PROB_FAILED_ADD:                  txt = "CM_PROB_FAILED_ADD"; break;    // driver failed add
    case CM_PROB_DISABLED_SERVICE:            txt = "CM_PROB_DISABLED_SERVICE"; break;    // service's Start = 4
    case CM_PROB_TRANSLATION_FAILED:          txt = "CM_PROB_TRANSLATION_FAILED"; break;    // resource translation failed
    case CM_PROB_NO_SOFTCONFIG:               txt = "CM_PROB_NO_SOFTCONFIG"; break;    // no soft config
    case CM_PROB_BIOS_TABLE:                  txt = "CM_PROB_BIOS_TABLE"; break;    // device missing in BIOS table
    case CM_PROB_IRQ_TRANSLATION_FAILED:      txt = "CM_PROB_IRQ_TRANSLATION_FAILED"; break;    // IRQ translator failed
    case CM_PROB_FAILED_DRIVER_ENTRY:         txt = "CM_PROB_FAILED_DRIVER_ENTRY"; break;    // DriverEntry() failed.
    case CM_PROB_DRIVER_FAILED_PRIOR_UNLOAD:  txt = "CM_PROB_DRIVER_FAILED_PRIOR_UNLOAD"; break;    // Driver should have unloaded.
    case CM_PROB_DRIVER_FAILED_LOAD:          txt = "CM_PROB_DRIVER_FAILED_LOAD"; break;    // Driver load unsuccessful.
    case CM_PROB_DRIVER_SERVICE_KEY_INVALID:  txt = "CM_PROB_DRIVER_SERVICE_KEY_INVALID"; break;    // Error accessing driver's service key
    case CM_PROB_LEGACY_SERVICE_NO_DEVICES:   txt = "CM_PROB_LEGACY_SERVICE_NO_DEVICES"; break;    // Loaded legacy service created no devices
    case CM_PROB_DUPLICATE_DEVICE:            txt = "CM_PROB_DUPLICATE_DEVICE"; break;    // Two devices were discovered with the same name
    case CM_PROB_FAILED_POST_START:           txt = "CM_PROB_FAILED_POST_START"; break;    // The drivers set the device state to failed
    case CM_PROB_HALTED:                      txt = "CM_PROB_HALTED"; break;    // This device was failed post start via usermode
    case CM_PROB_PHANTOM:                     txt = "CM_PROB_PHANTOM"; break;    // The devinst currently exists only in the registry
    case CM_PROB_SYSTEM_SHUTDOWN:             txt = "CM_PROB_SYSTEM_SHUTDOWN"; break;    // The system is shutting down
    case CM_PROB_HELD_FOR_EJECT:              txt = "CM_PROB_HELD_FOR_EJECT"; break;    // The device is offline awaiting removal
    case CM_PROB_DRIVER_BLOCKED:              txt = "CM_PROB_DRIVER_BLOCKED"; break;    // One or more drivers is blocked from loading
    case CM_PROB_REGISTRY_TOO_LARGE:          txt = "CM_PROB_REGISTRY_TOO_LARGE"; break;    // System hive has grown too large
    case CM_PROB_SETPROPERTIES_FAILED:        txt = "CM_PROB_SETPROPERTIES_FAILED"; break;    // Failed to apply one or more registry properties  
    case CM_PROB_WAITING_ON_DEPENDENCY:       txt = "CM_PROB_WAITING_ON_DEPENDENCY"; break;    // Device is stalled waiting on a dependency to start
    case CM_PROB_UNSIGNED_DRIVER:             txt = "CM_PROB_UNSIGNED_DRIVER"; break;    // Failed load driver due to unsigned image.
    case CM_PROB_USED_BY_DEBUGGER:            txt = "CM_PROB_USED_BY_DEBUGGER"; break;    // Device is being used by kernel debugger
    case CM_PROB_DEVICE_RESET:                txt = "CM_PROB_DEVICE_RESET"; break;    // Device is being reset
    case CM_PROB_CONSOLE_LOCKED:              txt = "CM_PROB_CONSOLE_LOCKED"; break;    // Device is blocked while console is locked
    case CM_PROB_NEED_CLASS_CONFIG:           txt = "CM_PROB_NEED_CLASS_CONFIG"; break;    // Device needs extended class configuration to start
    case CM_PROB_GUEST_ASSIGNMENT_FAILED:     txt = "CM_PROB_GUEST_ASSIGNMENT_FAILED"; break;    // Assignment to guest partition failed

  }
  return(QString("%1 (Code %2)").arg(txt).arg(a_code));

}




