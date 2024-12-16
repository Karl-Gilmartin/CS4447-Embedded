#include "bluetooth.h"
#include "esp_bt.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_log.h"


uint8_t ble_addr_type;
void ble_app_advertise(void);

#include <esp_wifi.h>

void get_mac_address(void) {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
void get_bluetooth_mac_address(void) {
    uint8_t mac[6];
    esp_err_t ret = esp_read_mac(mac, ESP_MAC_BT); // Read Bluetooth MAC address
    if (ret == ESP_OK) {
        printf("Bluetooth MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        printf("Failed to read Bluetooth MAC Address\n");
    }
}

// Write data to ESP32 defined as server
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // Print the raw data received from the client
    printf("Data from the client: %.*s\n", ctxt->om->om_len, (char *)ctxt->om->om_data);

    return 0; // Return success
}


int send_data(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    DhtData data;

    // Protect buffer access with the mutex
    if (xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(100))) {
        if (get_from_buffer(&dht_buffer, &data)) {
            uint8_t buffer[12] = {0};

            // Add header
            buffer[0] = 'D';
            buffer[1] = 'H';
            buffer[2] = 'T';

            // Serialize temperature and humidity
            union {
                float value;
                uint8_t bytes[sizeof(float)];
            } temp_union, hum_union;

            temp_union.value = data.temperature;
            hum_union.value = data.humidity;

            memcpy(&buffer[3], temp_union.bytes, sizeof(float));
            memcpy(&buffer[7], hum_union.bytes, sizeof(float));

            // Calculate checksum
            buffer[11] = 0;
            for (int i = 0; i < 11; i++) {
                buffer[11] ^= buffer[i];
            }

            // Send the serialized data to the BLE client
            os_mbuf_append(ctxt->om, buffer, sizeof(buffer));

            ESP_LOGI("BLE", "Sent DHT data: Temp=%.2f°C, Humidity=%.2f%%", data.temperature, data.humidity);
        } else {
            ESP_LOGW("BLE", "Buffer is empty, no data to send");
            os_mbuf_append(ctxt->om, "No data", strlen("No data"));
        }
        xSemaphoreGive(buffer_mutex);
    } else {
        ESP_LOGW("BLE", "Failed to acquire mutex for buffer");
        os_mbuf_append(ctxt->om, "Error", strlen("Error"));
    }
    return 0;
}


static int read_window_state(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    const char *state = window_open ? "OPEN" : "CLOSED";
    os_mbuf_append(ctxt->om, state, strlen(state));
    ESP_LOGI("BLE", "Window state read: %s", state);
    return 0;
}

static int read_temperature(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.2f", potentiometer_temperature);
    os_mbuf_append(ctxt->om, temp_str, strlen(temp_str));
    ESP_LOGI("BLE", "Temperature read: %s°C", temp_str);
    return 0;
}




// // Read data from ESP32 defined as server
// static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
// {
//     os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));
//     return 0;
// }

// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180), // Service UUID
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),           // DHT characteristic
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = send_data},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Write characteristic
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {.uuid = BLE_UUID16_DECLARE(0xBEEF),           // Window state characteristic
          .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          .access_cb = read_window_state},
         {.uuid = BLE_UUID16_DECLARE(0xC0FF),           // Temperature characteristic
          .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          .access_cb = read_temperature},
         {0}}},
    {0}};


// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status == 0) { // Connection successful
            struct ble_gap_conn_desc conn_desc;
            if (ble_gap_conn_find(event->connect.conn_handle, &conn_desc) == 0) {
                ESP_LOGI(TAG, "Connected to device with address: %02X:%02X:%02X:%02X:%02X:%02X",
                        conn_desc.peer_ota_addr.val[0],
                        conn_desc.peer_ota_addr.val[1],
                        conn_desc.peer_ota_addr.val[2],
                        conn_desc.peer_ota_addr.val[3],
                        conn_desc.peer_ota_addr.val[4],
                        conn_desc.peer_ota_addr.val[5]);
            } else {
                ESP_LOGE(TAG, "Failed to find connection details for handle: %d", event->connect.conn_handle);
            }
        } else {
            // Restart advertising if connection failed
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnected from device with address: %02X:%02X:%02X:%02X:%02X:%02X",
                 event->disconnect.conn.peer_ota_addr.val[0],
                 event->disconnect.conn.peer_ota_addr.val[1],
                 event->disconnect.conn.peer_ota_addr.val[2],
                 event->disconnect.conn.peer_ota_addr.val[3],
                 event->disconnect.conn.peer_ota_addr.val[4],
                 event->disconnect.conn.peer_ota_addr.val[5]);
        // Restart advertising
        ble_app_advertise();
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}

void bluetooth_init()
{
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    // esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set("Karls BLE"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(host_task);      // 6 - Run the thread
}