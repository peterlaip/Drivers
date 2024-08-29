//this program processed the digital data converted from ADS1115(an ADC) 
//compile:  gcc mq3_reader.c -o mq3_reader -ljson-c

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <json-c/json.h>
#include <math.h>

#define DEVICE_FILE "/dev/mq3_driver"
#define FILE_TO_WRITE "/home/user/path/to/alcohol_concentration.txt"
#define ALCOHOL_HISTORY_FILE "/home/user/path/to/alcohol_history.json"
#define MAX_RECORDS 10080
#define ADC_MAX_VALUE 65535
#define REFERENCE_VOLTAGE 4.096
#define RL 200000  // Load resistance 
#define RO 60      // Reference resistance in air

void add_to_history(float concentration);

int main(void) {
    int fd;
    ssize_t bytes_read;
    uint16_t adc_value;
    float voltage, alcohol_concentration;
    struct timespec sleep_time = {3, 0}; // 3秒
    FILE *file;

    fd = open(DEVICE_FILE, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device file");
        return EXIT_FAILURE;
    }

    while (1) {
        bytes_read = read(fd, &adc_value, sizeof(adc_value));
        if (bytes_read <= 0) {
            perror("Failed to read from device");
            close(fd);
            return EXIT_FAILURE;
        }

        // formula belowed is generated from Sensitivity characteristic curve [ mg/L (x axis) - Rs/Ro(y axis) ] 
        // log(x) = [log(y) - (−0.22806)] / (−0.69897)
        // MQ-3, ADS1115 parameters : RL = 200kΩ, Rs = 1MΩ - 8MΩ, Ro = 60, FSR = ±4.096 V
        alcohol_concentration = calculate_alcohol_concentration(adc_value);

        // Write to file for server display
        file = fopen(FILE_TO_WRITE, "w");
        if (file != NULL) {
            fprintf(file, "%.2f\n", alcohol_concentration);
            fclose(file);
        } else {
            perror("Failed to open alcohol concentration file for writing");
        }

        // Print the latest values to the terminal
        printf("ADC Value: %d, Alcohol Concentration: %.2f mg/L\n",
               adc_value,  alcohol_concentration);

        // Add to alcohol history
        add_to_history(alcohol_concentration);

        nanosleep(&sleep_time, NULL);
    }

    close(fd);
    return EXIT_SUCCESS;
}

void add_to_history(float concentration) {
    FILE *file = fopen(ALCOHOL_HISTORY_FILE, "r+");
    struct json_object *json_obj, *records_array, *new_record_obj;
    time_t now;
    struct tm *tm_info;
    char date_buf[26];
    size_t record_count;

    if (file == NULL) {
        // File doesn't exist or can't be opened, create a new one
        json_obj = json_object_new_object();
        records_array = json_object_new_array();
        json_object_object_add(json_obj, "records", records_array);
    } else {
        // File exists, read existing data
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (file_size > 0) {
            char *json_str = malloc(file_size + 1);
            fread(json_str, 1, file_size, file);
            json_str[file_size] = '\0';
            json_obj = json_tokener_parse(json_str);
            free(json_str);
        } else {
            json_obj = json_object_new_object();
        }

        if (json_object_get_type(json_obj) == json_type_object) {
            records_array = json_object_object_get(json_obj, "records");
            if (json_object_get_type(records_array) != json_type_array) {
                records_array = json_object_new_array();
                json_object_object_add(json_obj, "records", records_array);
            }
        } else {
            json_object_put(json_obj);
            json_obj = json_object_new_object();
            records_array = json_object_new_array();
            json_object_object_add(json_obj, "records", records_array);
        }

        fclose(file);
    }

    // Create new record
    now = time(NULL);
    tm_info = localtime(&now);
    strftime(date_buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    new_record_obj = json_object_new_object();
    json_object_object_add(new_record_obj, "date", json_object_new_string(date_buf));
    json_object_object_add(new_record_obj, "concentration", json_object_new_double(concentration));

    // Add new record to the array
    json_object_array_add(records_array, new_record_obj);

    // Limit the number of records
    record_count = json_object_array_length(records_array);
    if (record_count > MAX_RECORDS) {
        json_object_array_del_idx(records_array, 0, 1); // Remove the oldest record
    }

    // Write updated history to file
    file = fopen(ALCOHOL_HISTORY_FILE, "w");
    if (file != NULL) {
        fprintf(file, "%s", json_object_to_json_string(json_obj));
        fclose(file);
    } else {
        perror("Failed to open alcohol history file for writing");
    }

    json_object_put(json_obj); // Free memory
}

// Function to calculate alcohol concentration from ADC value
float calculate_alcohol_concentration(uint16_t adc_value) {
    // Calculate voltage from ADC value
    float voltage = (float)adc_value * REFERENCE_VOLTAGE / ADC_MAX_VALUE;

    // Calculate Rs (sensor resistance)
    float rs = RL * (REFERENCE_VOLTAGE - voltage) / voltage;

    // Calculate Rs/Ro ratio
    float rs_ro_ratio = rs / RO;

    // Calculate log(x) where x is the alcohol concentration
    float log_x = (log10f(rs_ro_ratio) + 0.22806f) / -0.69897f;

    // Calculate alcohol concentration
    float alcohol_concentration = powf(10, log_x);

    return alcohol_concentration;
}
