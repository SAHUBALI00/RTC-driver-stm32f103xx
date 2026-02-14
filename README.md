
🚀 Overview
The goal of this project was to implement a robust I2C interface from scratch. By manipulating the STM32 registers directly, this driver provides an efficient way to keep track of time in embedded applications. The development process involved deep-dive signal analysis to ensure protocol timing accuracy.

🛠 Features
Register-Level Control: Direct manipulation of I2C_CR1, I2C_SR1, and I2C_DR registers.

BCD Conversion: Custom logic to handle Binary Coded Decimal format required by the DS1307.

Time & Date Tracking: Full support for Seconds, Minutes, Hours, Day, Date, Month, and Year (including Leap Year support).

High Performance: Configured for Standard Mode (100kHz) I2C communication.

🔍 Debugging & Signal Analysis
This project was verified using GTKWave and logic analyzers. Key debugging milestones included:

ACK/NACK Verification: Used signal traces to confirm the 9th-bit handshake.

Bus Contention Fix: Resolved a critical "Halt" issue caused by a shared I2C address conflict with an MPU6050 sensor.

Overflow Protection: Fixed a 16-bit year truncation bug (2026 showing as 234) by optimizing data structures.

📋 Hardware Requirements
Microcontroller: STM32F103C8T6 (Blue Pill)

RTC Module: DS1307 (5V VCC recommended)

Pull-up Resistors: 4.7kΩ on SCL (PB6) and SDA (PB7)

Logic Analyzer: (Optional) for VCD file generation and signal verification.

Also , i have intergrate my self written code for UART for debugging and displaying the results.
