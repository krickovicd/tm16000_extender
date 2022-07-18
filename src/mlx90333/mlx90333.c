#include "mlx90333.h"

uint8_t read_buffer[8];

static inline void cs_select(const mlx_90333_t *sensor)
{
    asm volatile("nop \n nop \n nop");
    gpio_put(sensor->PIN_CS, 0); // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(const mlx_90333_t *sensor)
{
    asm volatile("nop \n nop \n nop");
    gpio_put(sensor->PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

void fill_data(const uint8_t buffer[], mlx_90333_axis_data_t *data);

void mlx90333_setup(mlx_90333_t *sensor, spi_inst_t *spi, uint miso, uint mosi, uint sck, uint cs)
{
    sensor->SPI_PORT = spi;
    sensor->PIN_MOSI = mosi;
    sensor->PIN_MISO = miso;
    sensor->PIN_SCK = sck;
    sensor->PIN_CS = cs;

    spi_init(sensor->SPI_PORT, 240 * 1000); // 320.000 is about max
    spi_set_format(sensor->SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(sensor->PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(sensor->PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(sensor->PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(sensor->PIN_CS);
    gpio_set_dir(sensor->PIN_CS, GPIO_OUT);
    gpio_put(sensor->PIN_CS, 1);
}

void fill_data(const uint8_t buffer[8], mlx_90333_axis_data_t *data)
{
    data->x_lsb = buffer[1];
    data->x_msb = buffer[2];
    data->y_lsb = buffer[3];
    data->y_msb = buffer[4];
    data->x = (int16_t)(buffer[2] << 8 | buffer[1]);
    data->y = (int16_t)(buffer[4] << 8 | buffer[3]);
    data->valid = false;
    uint16_t checksum = 0;
    checksum += buffer[1];
    checksum += buffer[2];
    checksum += buffer[3];
    checksum += buffer[4];

    bool checksumCorrect = (checksum & 0xFF) == buffer[7];

    if (buffer[0] == 255 && buffer[5] == 0 && buffer[6] == 0 && checksumCorrect)
    {
        data->valid = true;
    }
}

void mlx90333_get_axis_data(const mlx_90333_t *sensor, mlx_90333_axis_data_t *data)
{
    const uint8_t sendBuff = 0;
    uint8_t readBuff = 0;
    cs_select(sensor);
    sleep_ms(3);
    spi_write_read_blocking(sensor->SPI_PORT, &sendBuff, &readBuff, 1);
    read_buffer[0] = readBuff;
    sleep_us(50);
    spi_write_read_blocking(sensor->SPI_PORT, &sendBuff, &readBuff, 1);
    read_buffer[1] = readBuff;
    for (int i = 2; i < 8; i++)
    {
        sleep_us(20);
        spi_write_read_blocking(sensor->SPI_PORT, &sendBuff, &readBuff, 1);
        read_buffer[0] = readBuff;
    }
    sleep_us(3);
    cs_deselect(sensor);

    fill_data(read_buffer, data);
}
