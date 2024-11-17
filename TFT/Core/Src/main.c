#include "main.h"

// SPI Handle
SPI_HandleTypeDef hspi1;

// Prototype Functions
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_SPI1_Init(void);
void ILI9341_Init(void);
void ILI9341_WriteCommand(uint8_t cmd);
void ILI9341_WriteData(uint8_t data);
void ILI9341_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

int main(void) {
    // HAL Initialization
    HAL_Init();
    SystemClock_Config();

    // Initialize GPIO and SPI
    MX_GPIO_Init();
    MX_SPI1_Init();

    // Initialize ILI9341 Display
    ILI9341_Init();

    // Draw a red rectangle (x=50, y=50, width=100, height=100, color=0xF800 for red)
    ILI9341_DrawRectangle(50, 50, 100, 100, 0xF800);

    while (1) {
        // Main loop can handle button interactions or other logic
    }
}

// ILI9341 Initialization
void ILI9341_Init(void) {
    // Reset the display
    HAL_GPIO_WritePin(RESX_GPIO_Port, RESX_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(RESX_GPIO_Port, RESX_Pin, GPIO_PIN_SET);
    HAL_Delay(100);

    // Send initialization commands
    ILI9341_WriteCommand(0x01); // Software reset
    HAL_Delay(100);

    ILI9341_WriteCommand(0x28); // Display OFF
    ILI9341_WriteCommand(0x3A); // Pixel format set
    ILI9341_WriteData(0x55);    // 16 bits/pixel

    ILI9341_WriteCommand(0x29); // Display ON
}

// Send a command to ILI9341
void ILI9341_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, GPIO_PIN_RESET); // Command mode
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_RESET);         // Select screen
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);                  // Send command
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);           // Deselect screen
}

// Send data to ILI9341
void ILI9341_WriteData(uint8_t data) {
    HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, GPIO_PIN_SET);   // Data mode
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_RESET);         // Select screen
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);                 // Send data
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);           // Deselect screen
}

// Draw a rectangle on the display
void ILI9341_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    // Set column address
    ILI9341_WriteCommand(0x2A);
    ILI9341_WriteData(x >> 8);
    ILI9341_WriteData(x & 0xFF);
    ILI9341_WriteData((x + width - 1) >> 8);
    ILI9341_WriteData((x + width - 1) & 0xFF);

    // Set row address
    ILI9341_WriteCommand(0x2B);
    ILI9341_WriteData(y >> 8);
    ILI9341_WriteData(y & 0xFF);
    ILI9341_WriteData((y + height - 1) >> 8);
    ILI9341_WriteData((y + height - 1) & 0xFF);

    // Write data to memory
    ILI9341_WriteCommand(0x2C);
    for (uint32_t i = 0; i < width * height; i++) {
        ILI9341_WriteData(color >> 8);   // High byte of color
        ILI9341_WriteData(color & 0xFF); // Low byte of color
    }
}

// GPIO Initialization
void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO Clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Configure RESX_PIN
    GPIO_InitStruct.Pin = RESX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RESX_GPIO_Port, &GPIO_InitStruct);

    // Configure CSX_PIN
    GPIO_InitStruct.Pin = CSX_Pin;
    HAL_GPIO_Init(CSX_GPIO_Port, &GPIO_InitStruct);

    // Configure WRX_DCX_PIN
    GPIO_InitStruct.Pin = WRX_DCX_Pin;
    HAL_GPIO_Init(WRX_DCX_GPIO_Port, &GPIO_InitStruct);

    // Set default states
    HAL_GPIO_WritePin(RESX_GPIO_Port, RESX_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, GPIO_PIN_SET);
}

// SPI Initialization
void MX_SPI1_Init(void) {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; // Adjust SPI speed as needed
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

// System Clock Configuration
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the main internal regulator output voltage
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);

    // Initialize the CPU, AHB, and APB bus clocks
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;

