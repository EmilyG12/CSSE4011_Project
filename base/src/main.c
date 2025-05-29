/* 
* Basenode
*/

#include "ultrasonic.h"
#include "file_logging.h"

#include <zephyr/drivers/uart.h>
#include <stdint.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <string.h>
#include <zephyr/data/json.h>

LOG_MODULE_REGISTER(ultrasonic_app);

uint8_t ultraM[2] = {0xFF, 0xFF};
uint8_t ultraC[2] = {0xFF, 0xFF};

/* Ultrasonic Callback */
void ultrasonic_callback(int id, uint8_t metres, uint8_t cms) {
    if (metres > 12) {
        return;
    }
    ultraM[id] = metres;
    ultraC[id] = cms;
}

#define INVALID_COMMAND 1

#define STACKSIZE 2048

#define PRIORITY 7
struct player_distance {
    int m;
    int cms;
};

struct sensor_ultrasonic {
    struct player_distance player1;
    struct player_distance player2;
};

struct json_obj_descr player_dist_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct player_distance, m, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct player_distance, cms, JSON_TOK_NUMBER),
};

struct json_obj_descr ultra_sensor_descr[] = {
    JSON_OBJ_DESCR_OBJECT(struct sensor_ultrasonic,
                           player1, player_dist_descr),
    JSON_OBJ_DESCR_OBJECT(struct sensor_ultrasonic,
                           player2, player_dist_descr),
};

void print_base(const struct shell *shell) {
    struct player_distance player1 = { .m = ultraM[0], .cms = ultraC[0] };
    struct player_distance player2 = { .m = ultraM[1], .cms = ultraC[1] };

    struct sensor_ultrasonic ultrasonic_data = { .player1 = player1, .player2 = player2 };

    char json_output[300];

    int ret = json_obj_encode_buf(ultra_sensor_descr, ARRAY_SIZE(ultra_sensor_descr),
                    &ultrasonic_data, json_output, sizeof(json_output));
    if (ret < 0) {
        LOG_ERR("JSON Encode Fail");
    }

    shell_fprintf_normal(shell, "%s\n", json_output);
}

extern int cli_base(const struct shell *shell, size_t argc, char **argv) {
    
    while (true) {
        print_base(shell);
        k_msleep(1000);
    }

    return 0;
}

SHELL_CMD_ARG_REGISTER(base, NULL, "Stream the iBeacon readings of the mobile component", cli_base, 0, 0);

static int get_ultrasonic(const struct shell *shell, size_t argc, char *argv[]) {
    
    if (argc != 2) {
        LOG_ERR("Usage: ultra <sensor-id>");
        return INVALID_COMMAND; 

    } else if (strcmp(argv[1], "0") == 0) {
        LOG_INF("Metres: %d, Centimetres: %d", ultraM[0], ultraC[0]);

    } else if (strcmp(argv[1], "1") == 0) {
        LOG_INF("Metres: %d, Centimetres: %d", ultraM[1], ultraC[1]);

    } else {
        LOG_ERR("Invalid sensor id");
        return INVALID_COMMAND; 
    }
    
    return 0;
}

SHELL_CMD_REGISTER(ultra, NULL, "Command that retrieves the ultrasonic sensor values\n Usage: ultra \"sensor id\"", get_ultrasonic);

void log_data_thread(void) {
    
    while (1) {

        if (log_fights) {
            LOG_INF("Logging now");
            struct player_distance player1 = { .m = ultraM[0], .cms = ultraC[0] };
            struct player_distance player2 = { .m = ultraM[1], .cms = ultraC[1] };

            struct sensor_ultrasonic ultrasonic_data = { .player1 = player1, .player2 = player2 };

            char json_output[300];

            int ret = json_obj_encode_buf(ultra_sensor_descr, ARRAY_SIZE(ultra_sensor_descr),
                            &ultrasonic_data, json_output, sizeof(json_output));
            if (ret < 0) {
                LOG_ERR("JSON Encode Fail");
            }

            LOG_INF("Send to file");
            write_sensor_file("/lfs1/logs/fight.txt", json_output, 300);
        }

        k_msleep(200);
    }
}

K_THREAD_DEFINE(log_data_tid, STACKSIZE,
    log_data_thread, NULL, NULL, NULL,
    PRIORITY, 0, 0);

int main(void){
    const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

    if (!device_is_ready(dev)) {
        LOG_ERR("uart device not ready\n");
        return 1;
    }

    if (init_ultrasonic(0, ultrasonic_callback)){
        LOG_ERR("Failed to initialize ultrasonic");
        return 2;
    }
    
    if (init_ultrasonic(1, ultrasonic_callback)){
        LOG_ERR("Failed to initialize ultrasonic");
        return 2;
    }
    
    return 0;
}











