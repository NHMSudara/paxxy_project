#!/usr/bin/python3

import dbus
import random
import threading
import time

from advertisement import Advertisement
from service import Application, Service, Characteristic, Descriptor
#from gpiozero import CPUTemperature

GATT_CHRC_IFACE = "org.bluez.GattCharacteristic1"
NOTIFY_TIMEOUT = 6000

class paxxyAdvertisement(Advertisement):
    def __init__(self, index):
        Advertisement.__init__(self, index, "peripheral")
        self.add_local_name("PAXXY-R5")
        self.include_tx_power = True

class paxxyService(Service):
    PAXXY_SVC_UUID = "77c40383-ee88-425d-a5d1-0d9b8f0ba4dd"

    def __init__(self, index):
        self.farenheit = True

        Service.__init__(self, index, self.PAXXY_SVC_UUID, True)
        self.add_characteristic(fetalDataCharacteristic(self))
        #self.add_characteristic(UnitCharacteristic(self))

    def is_farenheit(self):
        return self.farenheit

    def set_farenheit(self, farenheit):
        self.farenheit = farenheit


class fetalDataCharacteristic(Characteristic):
    PAXXY_DATA_CHARACTERISTIC_UUID = "1ebf5ec3-c894-41f2-aaee-8f7d3616943e"

    def __init__(self, service):
        self.notifying = True

        Characteristic.__init__(
                self, self.PAXXY_DATA_CHARACTERISTIC_UUID,
                ["notify", "read"], service)
        self.add_descriptor(paxxyDescriptor(self))
    
    def get_data_dm(self):
        #tran = random.randint(100,144)
        value = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        return value
    
    def set_data_callback(self):
        if self.notifying:
            value = self.get_data_dm()
            self.PropertiesChanged(GATT_CHRC_IFACE, {"Value": value}, [])
        return self.notifying
    def inject_value(self,value):
        if self.notifying:
            #value = self.get_data_dm()
            #value[0] = random.randint(0,255)
            #self.set_data_callback()
            #self.PropertiesChanged(GATT_CHRC_IFACE, {"Value" : value}, [])
            self.PropertiesChanged(GATT_CHRC_IFACE, {'Value': dbus.Array(value, signature=dbus.Signature('y'))}, [])
    def StartNotify(self):
        if self.notifying:
            return

        self.notifying = True

        value = self.get_data_dm()
        #self.PropertiesChanged(GATT_CHRC_IFACE, value, [])
        self.PropertiesChanged(GATT_CHRC_IFACE, {'Value': dbus.Array(value, signature=dbus.Signature('y'))}, [])
        self.add_timeout(NOTIFY_TIMEOUT, self.set_data_callback)

    def StopNotify(self):
        self.notifying = False

    def ReadValue(self, options):
        value = self.get_data_dm()

        return value

class paxxyDescriptor(Descriptor):
    TEMP_DESCRIPTOR_UUID = "2902"
    TEMP_DESCRIPTOR_VALUE = "PA"

    def __init__(self, characteristic):
        Descriptor.__init__(
                self, self.TEMP_DESCRIPTOR_UUID,
                ["read"],
                characteristic)

    def ReadValue(self, options):
        value = []
        desc = self.TEMP_DESCRIPTOR_VALUE

        for c in desc:
            value.append(dbus.Byte(c.encode()))

        return value
##########################
def r_ble(app_nm):
    try:
        app_nm.run()
    except KeyboardInterrupt:
        app_nm.quit()





app = Application()
paxs = paxxyService(0)
app.add_service(paxs)
app.register()
#cpath = paxs.get_characteristic_paths
#print(cpath)

bus = dbus.SystemBus()

#print(len(paxs.get_characteristics()))
ble_point = (paxs.get_characteristics())[0]



#char_obj = bus.get_object("org.bluez", "/org/bluez/example/service0")
adv = paxxyAdvertisement(0)
adv.register()

#char_obj = bus.get_object("org.bluez", "example/service0/paxxyService")
#char_iface = dbus.Interface(char_obj, "org.bluez.GattCharacteristic1")


#try:
#    app.run()
#except KeyboardInterrupt:
#    app.quit()


x = threading.Thread(target=r_ble, args=(app,))
x.start()

def boink(value):
    ble_point.inject_value(value)
