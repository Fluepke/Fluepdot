#include "bluetooth.h"

#include "esp_log.h"

#include "main.h"
#include "flipdot_framebuffer.h"

/* BLE */
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *manuf_name = "Fluepke";
static const char *model_num = "Fluepdot";
static const char *bluetooth_device_name = "fluepdot";
static const char *TAG = "bluetooth.c";

static uint16_t conn_handle;
static uint8_t bluetooth_addr_type;

static int bluetooth_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);
static int bluetooth_access_flipdot_width(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);
static int bluetooth_access_flipdot_line(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);
static void bluetooth_start_advertise(void);
static void bluetooth_on_sync(void);
static void bluetooth_on_reset(int reason);
static void bluetooth_host_task(void *param);
static int bluetooth_gap_event(struct ble_gap_event *event, void *arg);
static int bluetooth_gatt_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
        void *dst, uint16_t *len);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = 
{
    {
        /* Service: Custom fluepdot service */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        {
            /* TODO build some preprocessor macro for this mess */
            {
                /* Characteristic: Fluepdot line y=0 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE0_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=1 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE1_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=2 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE2_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=3 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE3_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=4 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE4_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=5 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE5_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=6 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE6_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=7 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE7_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=8 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE8_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=9 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE9_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=10 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE10_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=11 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE11_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=12 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE12_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=13 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE13_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=14 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE14_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Fluepdot line y=15 */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_LINE15_UUID),
                .access_cb = bluetooth_access_flipdot_line,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC,
            },
            {
                /* Characteristic: Flipdot width */
                .uuid = BLE_UUID16_DECLARE(BLUETOOTH_FLIPDOT_WIDTH_UUID),
                .access_cb = bluetooth_access_flipdot_width,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                0, /* No more characteristics in this service */
            },
        },
    },
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        {
            {
                /* Characteristic: * Manufacturer name */
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                .access_cb = bluetooth_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                /* Characteristic: Model number string */
                .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                .access_cb = bluetooth_access_device_info,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                0, /* No more characteristics in this service */
            },
        }
    },
    {
        0, /* No more services */
    },
};

static int bluetooth_access_device_info(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == GATT_MODEL_NUMBER_UUID) {
        rc = os_mbuf_append(ctxt->om, model_num, strlen(model_num));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    if (uuid == GATT_MANUFACTURER_NAME_UUID) {
        rc = os_mbuf_append(ctxt->om, manuf_name, strlen(manuf_name));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int bluetooth_access_flipdot_width(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid == BLUETOOTH_FLIPDOT_WIDTH_UUID) {
        rc = os_mbuf_append(ctxt->om, &(flipdot.width), sizeof(flipdot.width));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

static int bluetooth_access_flipdot_line(uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid;
    size_t buf_len = flipdot.width + 1;
    char* buf = malloc(buf_len);
    if (buf == NULL) {
        return BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    int rc;

    uuid = ble_uuid_u16(ctxt->chr->uuid);

    if (uuid > 0x000F) {
        assert(0);
        return BLE_ATT_ERR_UNLIKELY;
    }

    switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            flipdot_framebuffer_encode_line(flipdot.framebuffer, buf, buf_len, (uint8_t)uuid);
            rc = os_mbuf_append(ctxt->om, buf, buf_len - 1);
            free(buf);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            rc = bluetooth_gatt_chr_write(ctxt->om, buf_len - 1, buf_len -1, buf, NULL);
            buf[buf_len - 1] = 0;
            if (rc == 0) {
                esp_err_t error = flipdot_framebuffer_decode_line(flipdot.framebuffer, buf,
                        buf_len, (uint8_t)uuid);
                free(buf);
                return (error == ESP_OK) ? 0 : 1;
            }
            free(buf);
            return rc;
        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
    }
}

/*
 * Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
static void bluetooth_start_advertise(void) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /*
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info)
     *     o Advertising tx power
     *     o Device name
     */
    memset(&fields, 0, sizeof(fields));

    /*
     * Advertise two flags:
     *      o Discoverability in forthcoming advertisement (general)
     *      o BLE-only (BR/EDR unsupported)
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /*
     * Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *)bluetooth_device_name;
    fields.name_len = strlen(bluetooth_device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "error setting advertisement data; rc=%d", rc);
        return;
    }

    /* Begin advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(bluetooth_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, bluetooth_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "error enabling advertisement; rc=%d", rc);
        return;
    }
}

static void bluetooth_on_sync(void) {
    int rc = ble_hs_id_infer_auto(0, &bluetooth_addr_type);
    assert(rc == 0);

    bluetooth_start_advertise();
}

static void bluetooth_on_reset(int reason) {
    ESP_LOGI(TAG, "Resetting state; reason=%d", reason);
}

static void bluetooth_host_task(void *param) {
    ESP_LOGI(TAG, "BLE Host Task started");
    nimble_port_run();

    nimble_port_freertos_deinit();
}

static int bluetooth_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        ESP_LOGI(TAG, "connection %s; status=%d",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising */
            bluetooth_start_advertise();
        }
        conn_handle = event->connect.conn_handle;
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "disconnect; reason=%d", event->disconnect.reason);

        /* Connection terminated; resume advertising */
        bluetooth_start_advertise();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "adv complete");
        bluetooth_start_advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
//        MODLOG_DFLT(INFO, "subscribe event; cur_notify=%d\n value handle; "
//                    "val_handle=%d\n",
//                    event->subscribe.cur_notify, hrs_hrm_handle);
//        if (event->subscribe.attr_handle == hrs_hrm_handle) {
//            notify_state = event->subscribe.cur_notify;
//            blehr_tx_hrate_reset();
//        } else if (event->subscribe.attr_handle != hrs_hrm_handle) {
//            notify_state = event->subscribe.cur_notify;
//            blehr_tx_hrate_stop();
//        }
//        ESP_LOGI("BLE_GAP_SUBSCRIBE_EVENT", "conn_handle from subscribe=%d", conn_handle);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "mtu update event; conn_handle=%d mtu=%d",
                    event->mtu.conn_handle,
                    event->mtu.value);
        break;

    }

    return 0;
}

static int bluetooth_gatt_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
        void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

esp_err_t bluetooth_initialize(void) {
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());

    nimble_port_init();

    ble_hs_cfg.sync_cb = bluetooth_on_sync;
    ble_hs_cfg.reset_cb = bluetooth_on_reset;

    // gatt init
    ble_svc_gap_init();
    ble_svc_gatt_init();
    int rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return ESP_FAIL;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return ESP_FAIL;
    }

    rc = ble_svc_gap_device_name_set(bluetooth_device_name);
    if (rc != 0) {
        return ESP_FAIL;
    }

    nimble_port_freertos_init(bluetooth_host_task);

    return ESP_OK;
}
