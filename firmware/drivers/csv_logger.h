#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include "system_data.h"

int  csv_logger_init(const char *filepath);

void csv_logger_write(const SensorData *data, const ControlStatus *status);

void csv_logger_close(void);

#endif 