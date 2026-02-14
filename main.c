#include <serialPort.h>
#define   RCC_base_addr          0x40021000
#define   PORTB_base_addr        0x40010C00
#define   APB1_offset            0x1C
#define   APB2_offset            0x18
#define   GPIOB_CRL_offset       0x00
#define   I2C_base_addr          0x40005400
#define   I2C_CR1                0x00
#define   I2C_CR2                0x04
#define   I2C_SR1                0x14
#define   I2C_SR2                0x18
#define   I2C_CCR                0x1C
#define   I2C_TRISE              0x20
#define   I2C_DR                 0x10
#define   rtc_base_addr          0x68

typedef struct{
  uint8_t seconds ;
  uint8_t minutes;
  uint8_t hours;
  char *meridiem;
  char *day;
  uint8_t date;  // as year would greater than 255
  uint8_t month;
  uint16_t year;
}RTC_time;

void config_I2C();
void I2C_START();
void I2C_Stop(void);
void RTC_SetTime(uint8_t sec, uint8_t minutes, uint8_t hour,uint8_t day,uint8_t date,uint8_t month,uint8_t year) ;
uint8_t bcdToDec(uint8_t val);
void RTC_ReadTime(uint8_t data[]);
uint8_t decToBcd(int val) ;
void I2C_Write(int data);
RTC_time process_rtc_data();

volatile uint32_t *const abp1 =(volatile uint32_t*) (RCC_base_addr + APB1_offset);
// volatile uint32_t *const abp2 =(volatile uint32_t*) (RCC_base_addr + APB2_offset);
volatile uint32_t *const portB =  (volatile uint32_t*) (PORTB_base_addr + GPIOB_CRL_offset);
volatile uint32_t * const i2c_cr1 =    (volatile uint32_t*)(I2C_base_addr + I2C_CR1);

volatile uint32_t * const i2c_cr2 =    (volatile uint32_t*)(I2C_base_addr + I2C_CR2);

volatile uint32_t * const i2c_ccr =    (volatile uint32_t*)(I2C_base_addr + I2C_CCR);

volatile uint32_t * const i2c_trise =  (volatile uint32_t*)(I2C_base_addr + I2C_TRISE);

volatile uint32_t * const i2c_dr =     (volatile uint32_t*)(I2C_base_addr + I2C_DR);

volatile uint32_t * const i2c_sr1 =    (volatile uint32_t*)(I2C_base_addr + I2C_SR1);

volatile uint32_t * const i2c_sr2 =    (volatile uint32_t*)(I2C_base_addr + I2C_SR2);


enum Day {
    MONDAY = 1,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY
};

RTC_time sensor_data;


 void setup() {
  config_UART();
  config_I2C();
 

  printUART("hello rtc driver ");

  UART_SendNewLine();

  // config_I2C();

 

  RTC_SetTime(0,30,02,6,13,2,26);

for(volatile int i=0; i<50000; i++);  //delay required

// sensor_data=process_rtc_data();
// UART_SendInt(sensor_data.date);
// UART_SendInt(sensor_data.month);


}
void config_I2C(){
  *abp1 |= (1 << 21);   //enabling clk for i2c1
  *abp2 |= (1 << 3);   //port B enable

  *portB &= ~(15 << 24);
  *portB &= ~(15 << 28);

  *portB |= (15 << 24); 
  *portB |= (15 << 28);

  
  *i2c_cr2 = (36 << 0);  //apb1 operating freq: 36MHZ
  *i2c_ccr = 180;       //ccr value: how to toggle scl
  *i2c_trise = 37;
  *i2c_cr1 |= (1 << 0); //peripheral enable
}

void I2C_START(volatile uint8_t address, int r_w){
  *i2c_cr1 |= (1 << 8); //set start bit
  while (!(*i2c_sr1 & (1 << 0)));  // 2. Wait for SB (EV5)
  *i2c_dr = (address << 1) | r_w;

   while (!(*i2c_sr1 & (1 << 1)));  //addr bit  set waitn(indicates end of address transmission)
// For 7-bit addressing, the bit is set after the ACK of the byte
// ADDR is not set after a NACK reception
  uint32_t temp = *i2c_sr1;        // Read SR1

  temp = *i2c_sr2;                  //Read SR2

}


void I2C_Write(int data){
  while (!(*i2c_sr1 & (1 << 7)));  // Wait for TxE (Transmit Empty) (set by hardware on receiving acknowledgement)
  *i2c_dr = data;
  while (!(*i2c_sr1 & (1 << 2)));  //check for BTF flag
  // printUART("in write func");
}

void I2C_Stop(void) {

  *i2c_cr1 |= (1 << 9);            // Generate STOP

}

void RTC_SetTime(uint8_t sec, uint8_t minutes, uint8_t hour,uint8_t day,uint8_t date,uint8_t month,uint8_t year) {
    I2C_START(0x68, 0);

    I2C_Write(0x00); // Start at Seconds register
    I2C_Write(decToBcd(sec)); // Bit 7 is 0, so clock starts!
    I2C_Write(decToBcd(minutes));
    I2C_Write(decToBcd(hour)); //12 hr mode selected
    I2C_Write(decToBcd(day));
    I2C_Write(decToBcd(date));
    I2C_Write(decToBcd(month));
    I2C_Write(decToBcd(year));


    I2C_Stop();
}

void RTC_ReadTime(uint8_t data[]){
  I2C_START(0x68, 0); 
  I2C_Write(0x00);   //point of first register

  I2C_Stop();

  while (*i2c_sr2 & (1 << 1)); // Wait until the BUSY bit is cleared by hardware
 
  *i2c_cr1 |= (1 << 10);   //ENABLE ACK BEFORE ReSTART to start reading ---
  I2C_START(0x68, 1); // Start in Read Mode
  
 
  for(int i = 0; i < 6; i++) {
        while (!(*i2c_sr1 & (1 << 6))); // Wait for RxNE
        data[i] = *i2c_dr;
        *i2c_cr1 |= (1 << 10);          // Ensure ACK is still set
    }

    // 6. The "AVR NACK" equivalent for the 7th byte
    *i2c_cr1 &= ~(1 << 10); // Set NACK (Match: TWCR = (1 << TWINT) | (1 << TWEN))
    *i2c_cr1 |= (1 << 9);   // Set STOP
    
    while (!(*i2c_sr1 & (1 << 6))); // Wait for last byte
    data[6] = *i2c_dr;

}


RTC_time process_rtc_data(){
  uint8_t raw[7];
  RTC_ReadTime(raw);

  RTC_time sensor;
  sensor.seconds  =  bcdToDec(raw[0]);
  sensor.minutes  =  bcdToDec(raw[1]);
  uint8_t hourData    = (raw[2]);
  sensor.hours    = bcdToDec(hourData & 0x1F);
  if(hourData & (1<<5)){
    sensor.meridiem = "PM";
  }
  else{
    sensor.meridiem = "AM";
  }
  // UART_SendInt(sensor.hours);
  int dayNumber      =  bcdToDec(raw[3]);
  switch(dayNumber){
    case MONDAY:     sensor.day = "Monday";  break;
    case TUESDAY:     sensor.day = "Tuesday";  break;
    case WEDNESDAY:   sensor.day = "Wednesday";  break;
    case THURSDAY:    sensor.day = "Thursday";  break;
    case FRIDAY:      sensor.day = "Friday";  break;
    case SATURDAY:    sensor.day = "Saturday";  break;
    case SUNDAY:      sensor.day = "Sunday";  break;
    default:         sensor.day = "Invalid day";  break;  // Handle invalid input (not 1-7)
  }
  sensor.date     =  bcdToDec(raw[4]);
  sensor.month    =  bcdToDec(raw[5]);
  uint8_t year     =  bcdToDec(raw[6]);
  sensor.year= (uint16_t) (year+2000);

  
  return sensor;
  
}

uint8_t decToBcd(int val) {
  return (uint8_t)( (val/10 << 4) | (val % 10) );
}

uint8_t bcdToDec(uint8_t val) {
  return (uint8_t)( (val/16*10) + (val%16) );
}

void loop(){
sensor_data=process_rtc_data();
    printUART(" Time : ");
    UART_SendInt(sensor_data.hours);
    printUART(":");
    UART_SendInt(sensor_data.minutes);
    printUART(":");
    UART_SendInt(sensor_data.seconds);
    printUART(" ");
    printUART(sensor_data.day);
    printUART("  Date: ");
    UART_SendInt(sensor_data.date);
    printUART(":");
    UART_SendInt(sensor_data.month);
    printUART(":");
    UART_SendInt(sensor_data.year);

    UART_SendNewLine();
    delay(2000);
}
