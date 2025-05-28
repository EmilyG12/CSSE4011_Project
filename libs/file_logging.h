#ifndef LOGGING_H_
#define LOGGING_H_
#include <stdbool.h>

int init_file_systems(void);
void write_sensor_file(char *fileName, char *data, int bufflen);

extern bool log_fights;

#endif
