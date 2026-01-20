#include <UsRange.h>
#include <esp_log.h>
#define BUF_SIZE 128

static char *TAG="UsRange";
UsRange::UsRange(int com_port,int rxPin):
    Thread("UsRange"),
    com_port((uart_port_t)com_port),
    distance(0.0f)
{
    bzero((void*)&uart_config,sizeof(uart_config));
    uart_config.baud_rate = 9600;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    int intr_alloc_flags = 0;
    ESP_ERROR_CHECK(uart_driver_install((uart_port_t)com_port, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config((uart_port_t)com_port, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin((uart_port_t)com_port, -1, rxPin, -1, -1));
    Thread::start();

}

UsRange::~UsRange()
{

}

float UsRange::getMeasure()
{
    return distance;
}

void UsRange::loop(void)
{
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    while (1) {
        
        // Read data from the UART
        int len = uart_read_bytes(com_port, data, (BUF_SIZE - 1), 25 / portTICK_PERIOD_MS);
        if ( (len==4) && (data[0]==0xFF) && ( (uint8_t)(data[0]+data[1]+data[2])==data[3]))  {
            float tmp_distance = (256.0*data[1]+data[2])/1000.0;
            if (distance == 0.0f) distance = tmp_distance;
            distance = distance*0.95+0.05*tmp_distance;
        }
    }
}