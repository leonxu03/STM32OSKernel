## About STM32 Real-Time OS Kernel
Developed a custom **Real-Time Executive Operating System** for the STM32 Nucleo Board. 

<img width="1259" height="707" alt="Screenshot 2025-08-01 at 5 23 00 PM" src="https://github.com/user-attachments/assets/c7b1704e-8811-41a2-b648-12d800c57ca4" />

Includes support for: 
- **multitasking scheduling policies**
  
https://github.com/user-attachments/assets/df2b271c-6fa0-4921-8973-ae076411e725

- **efficient dynamic memory allocation**

https://github.com/user-attachments/assets/9f5d5ac5-65df-4213-b4c7-c31340124dfc

- **priority based task pre-emption**

https://github.com/user-attachments/assets/e48b8cba-1f2c-4ad9-bd26-a944aac8293b

  
## Requirements
- STM32 NUCLEO-F401RE board
- STM32CubeIDE is the recommended code editor

## Run
- Plug in STM32 Nucleo board
- Clone and open repository using STM32CubeIDE as the code editor
- Click green play button at the top of the editor
- Observe the following test case results:
  - Performance of different multitasking scheduling policies
  - Efficient dynamic memory allocation and deallocation via buddy system algorithm
  - Efficient heap memory usage
  - Priority based task pre-emption
