#include "dht11.h"

#include "stm32f1xx_hal.h"
#include "dwt_stm32_delay.h"

float Humidity = 0;
float Temperature = 0;

int dht11_data[5] = { 0, 0, 0, 0, 0 };			// dht11 data array [0, 1] => Humidity, [2, 3] => Temperature, [5] => Checksum 

typedef enum 
{
	OUTPUT = GPIO_MODE_OUTPUT_PP,
	INPUT = GPIO_MODE_INPUT
} Pinmde;

void Gpio_Pinmode(GPIO_TypeDef * port, uint32_t pin, uint32_t mode){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	if(mode == OUTPUT){ 
		GPIO_InitStruct.Pin = pin;
		GPIO_InitStruct.Mode = mode;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;	
	}
	
	else {
		GPIO_InitStruct.Pin = pin;
		GPIO_InitStruct.Mode = mode;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
	}
	
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void Read_Dht11_Data(void)
{
	uint8_t state = 1;							// Pulse state, def. : HIGH
	uint8_t counter = 0;						// Data length counter
 
	for(uint8_t i = 0; i < 5; i++){				// Data reset
		dht11_data[i] = 0;
	}
	
	Gpio_Pinmode(GPIOA, GPIO_PIN_2, OUTPUT);	// Command write mode
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_2, 0);	// Start signal
	DWT_Delay_us (18000);						// Delay atleast 18ms
	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_2, 1);	// Wait for response	
	DWT_Delay_us(40);							// Delay 20 ~ 40us
	Gpio_Pinmode(GPIOA, GPIO_PIN_2, INPUT);		// Data read mode
 
	// 1 Data => 8 Bit
	// 8 Bit == 8 Pulse
	// 1 Pulse => 2 State Change
	// 5 Data == 40 Pulse
	// 40 Pulse == 80 Pulse
	// 80 Pulse + extra Bit == 85
	for (uint8_t  i = 0, j = 0; i < TIMINGS_COUNT; i++ )
	{
		counter = 0;
		while ( HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == state )
		{
			counter++;
			DWT_Delay_us(1);					// Delay 1ms
			if (counter == 255)					// No Signal
			{
				break;
			}
		}
		state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);	// Read State
 
		if (counter == 255) 			// No Signal
			break;
 
		if ((i >= 4) && (i % 2 == 0)) 	// Skip First 3 state change
		{
			dht11_data[j / 8] <<= 1;		// Shift 1 bit
			if ( counter > 28 )			// If high-voltage length is bigger than 28
				dht11_data[j / 8] |= 1;	// Data "1" bit
			j++;						// Data count
		}
	}
 
	if (dht11_data[4] == ((dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3]) & 0xFF))	// Compare checksum
	{
		printf( "Humidity = %d.%d %% Temperature = %d.%d C\r\n", dht11_data[0], dht11_data[1], dht11_data[2], dht11_data[3]);
		
		Humidity = dht11_data[0] + (dht11_data[1] * 0.1f);
		Temperature = dht11_data[2] + (dht11_data[3] * 0.1f);
	}else  {
		printf( "error\r\n" );
	}
}

float Get_Humidity(void){
	return Humidity;
}

float Get_Temperature(void){
	return Temperature;
}
