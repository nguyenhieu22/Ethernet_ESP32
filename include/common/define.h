#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdint.h>

#define ETH_CONNECTED_BIT BIT0
#define ETH_FAIL_BIT BIT1
#define BLE_MESH_INIT_BIT BIT0
#define BLE_MESH_FAIL_BIT      BIT1

typedef struct {
    char * data;
    uint16_t len;
} xsolar_buf_t;

#endif