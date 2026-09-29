#include <UsRange.h>
#include <esp_log.h>
#include <strings.h>

#define BUF_SIZE 128

static const char *TAG="UsRange";
UsRange::UsRange(int com_port,int rxPin):
    Thread("UsRange",2048),
    com_port((uart_port_t)com_port),
    distance(0.0f),
    fail(true),
    failCnt(0)
{
    ESP_LOGI(TAG,"init on com %d gpio %d",com_port,rxPin);
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

bool UsRange::getMeasure(float &val)
{
    bool ret;
    take();
    val = distance;
    ret = fail;
    give();
    return ret;
}

void UsRange::loop(void)
{
    while (1) {
        uint8_t data[4];
        // Read data from the UART
        int len = uart_read_bytes(com_port, &data[0], sizeof(data), pdMS_TO_TICKS(150));
        take();
        fail = false;
        if ( (len==sizeof(data)) && (data[0]==0xFF))
        {
            uint8_t cks = data[0] + data[1] + data[2] ;
            if (cks == data[3])
            {
                distance = (float)(256*data[1]+data[2])/1000.0f;
                failCnt = 0;
            }
            else
            {
                len = 0;
            }

        }
        if (len != sizeof(data))
        {
            uart_flush_input(com_port);
            failCnt++;
            if (failCnt > 10)
            {
              fail = true;
            }
        }
        give();
    }
}