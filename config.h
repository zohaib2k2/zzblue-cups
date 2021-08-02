#include <stdint.h>


/*
 * This is the maximum file size in bytes that server recives
 */
#define MAX_FILE_SIZE (1024*1024*3)


uint8_t rfcomm_channel = 15;


const char *service_name = "HP inkjet 2130 printer";

const char *service_dsc = "An expermential project to convert a legacy printer, wireless";


const char *service_prov = "HP deskjet 2130";


const int uuid128[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xab, 0xcd};

typedef enum {FALSE, TRUE} boolean;

#define TMP_FILE "/tmp/zzblue-sv-buffer"
