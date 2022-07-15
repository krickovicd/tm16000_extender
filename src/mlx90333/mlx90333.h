#ifndef _tmext_mlx_90333_h
#define _tmext_mlx_90333_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

typedef struct {
    uint PIN_MISO;
    uint PIN_MOSI;
    uint PIN_SCK;
    uint PIN_CS;
    spi_inst_t * SPI_PORT;
} mlx_90333_t;

typedef struct 
{
    uint8_t x_lsb;
    uint8_t x_msb;
    uint8_t y_lsb;
    uint8_t y_msb;
    int16_t x;
    int16_t y;
    bool valid;
} mlx_90333_axis_data_t;

/**
*	@brief initialize mlx 90333 hal sensor and spi port
*
*	@param[in] sensor : pointer to instance of mlx_90333_t
*	@param[in] spi : pointer to SPI instance
*	@param[in] miso : miso pin number
*	@param[in] mosi : mosi pin number
*	@param[in] sck : clock pin
*	@param[in] cs : chip select pin
*	
* 	@return void.
*/
void mlx90333_setup(mlx_90333_t *sensor, spi_inst_t *spi, uint miso, uint mosi, uint sck, uint cs);

/**
*	@brief read data from mlx 90333 hal sensor
*
*	@param[in] sensor : pointer to instance of mlx_90333_t
*	@param[in] data : pointer to mlx_90333_axis_data_t instance
*	
* 	@return mlx_90333_axis_data_t.
*   @retval data is successfully read if valid is true
*/
void mlx90333_get_axis_data(const mlx_90333_t* sensor, mlx_90333_axis_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* _tmext_mlx90333_h */