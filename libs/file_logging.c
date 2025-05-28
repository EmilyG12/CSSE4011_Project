/** 
 * File Name: file_logging.c
 * Date: 6/04/2025
 * Function: Library for reading/writing files, and logging sensor data to a file
 * Code inspired by:
 * REF: https://docs.zephyrproject.org/latest/samples/subsys/fs/littlefs/README.html
 * REF: https://docs.zephyrproject.org/latest/samples/subsys/shell/fs/README.html
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(logging_lib);

#ifdef CONFIG_FILE_SYSTEM

#include "file_logging.h"
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/shell/shell.h>
#include <zephyr/data/json.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/littlefs.h>

bool log_fights = false;

/* Command to log fight data to a file
* shell: the shell in which the command is entered
* argc: the number of arguments
* argv: the arguments
* Return: 0 when complete
*/
static int log_file(const struct shell *shell, size_t argc, char *argv[]) {
    // Start logging
    if (argc == 2) {

        if (strcmp(argv[1], "s") == 0) {
            LOG_DBG("Start logging");
            log_fights = true;
        } else if (strcmp(argv[1], "p") == 0) { 
            LOG_DBG("End logging");
            log_fights = false;
        } else {
            LOG_ERR("Usage: logfile <s/p>");
        }

    } else {
        LOG_ERR("Usage: logfile <s/p>");
    }
    return 0;
}

SHELL_CMD_REGISTER(logfile, NULL,"Read the value from the sensor", log_file);

/* Opens file and appends to file
* fileName: the path to the file
* data: the data to write to the file
* bufflen: the amount of data to write
*/
void write_sensor_file(char *fileName, char *data, int bufflen) {
    
    struct fs_file_t file;
    fs_file_t_init(&file);
    LOG_INF("Attempt to open file %s", fileName);
    int rc = fs_open(&file, fileName, FS_O_CREATE|FS_O_RDWR|FS_O_APPEND);
    if (rc < 0) {
        LOG_ERR("Fail: open %s:, %d", fileName,rc);
        return;
    }
    LOG_INF("Opened %s", fileName);

    char log_entry[bufflen];
    snprintf(log_entry, sizeof(log_entry), "%s\n", data);

    ssize_t write_res = fs_write(&file, log_entry, strlen(log_entry));
    if (write_res < 0) {
        LOG_ERR("Write failed %d", write_res);
    }

    fs_close(&file);
    return;
}

#define PARTITION_NODE DT_NODELABEL(lfs1)
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
   
struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
    
/* Init file system
*/
int init_file_systems(void) {
    fs_unmount(mp);
    int rc = fs_mount(mp);

    if (rc == -EINVAL) {
        LOG_ERR("Invalid file system");

        rc = fs_mount(mp);
    }

    if (rc < 0) {
        return rc;
    }

    LOG_INF("Filesystem mounted");
    return rc;
}

SYS_INIT(init_file_systems, APPLICATION,CONFIG_APPLICATION_INIT_PRIORITY);
#endif
