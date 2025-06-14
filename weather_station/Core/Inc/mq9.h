#ifndef INC_MQ9_H_
#define INC_MQ9_H_

int8_t MQ9_calbration(void);
int8_t MQ9_read_data(void);
void MQ9_print_sensor_data(float ppm_val, float ratio_val);

#endif /* INC_MQ9_H_ */
