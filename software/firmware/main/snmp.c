#include "snmp.h"

#include "lwip/apps/snmp_opts.h"
#include "lwip/snmp.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_core.h"
#include "lwip/apps/snmp_mib2.h"
#include "lwip/apps/snmp_scalar.h"

#include "fluepdot_mib.h"

u8_t * SNMP_SYSCONTACT = (u8_t*) "support@luepke.email";
u16_t SNMP_SYSCONTACT_LEN = sizeof("support@luepke.email");

u8_t * SNMP_SYSNAME = (u8_t*) "ESP32_Core_board_V2";
u16_t SNMP_SYSNAME_LEN = sizeof("ESP32_Core_board_V2");

u8_t * SNMP_SYSLOCATION = (u8_t*) "Your Institute or Company";
u16_t SNMP_SYSLOCATION_LEN = sizeof("Your Institute or Company");

static const struct snmp_mib* my_mibs[] = { &fluepke };

esp_err_t snmp_initialize(void) {
    snmp_mib2_set_syscontact(SNMP_SYSCONTACT, &SNMP_SYSCONTACT_LEN, 0);
    snmp_mib2_set_syslocation(SNMP_SYSLOCATION, &SNMP_SYSLOCATION_LEN, 0);
    snmp_set_auth_traps_enabled(0);

    snmp_set_mibs(my_mibs, 1);
    snmp_init();

    return ESP_OK;
}
