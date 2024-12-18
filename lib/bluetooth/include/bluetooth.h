#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"
#include "../../../src/dht11.h"
#include "../../../src/potentiometer.h"
#include "../../../src/servo.h"
#include "../../../src/main.h"
#include "../../../src/statemachine.h"
#include "../../../src/config.h"
#include "../../../src/led.h"



#define TAG "[MyProject]"  // Log tag

void get_mac_address();
void bluetooth_init();
int send_data(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
void get_bluetooth_mac_address();
static int read_temperature(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int read_window_state(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);