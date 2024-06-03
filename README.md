# NUC029xEE_SPI_Mode_Swtich
 NUC029xEE_SPI_Mode_Swtich


update @ 2024/06/03

1. initial SPI TX with mode switch function , output SPI MOSI data (8 bytes) per 500 ms

2. use terminal , press 0~3 , to switch to different SPI MODE

    mode	    CPOL	CPHA
	
    mode 0	    0	    0
	
    mode 1	    0	    1
	
    mode 2	    1	    0
	
    mode 3	    1	    1
	
	
below is MODE 0
![image](https://github.com/released/NUC029xEE_SPI_Mode_Swtich/blob/main/mode_0.jpg)


below is MODE 1	
![image](https://github.com/released/NUC029xEE_SPI_Mode_Swtich/blob/main/mode_1.jpg)


below is MODE 2	
![image](https://github.com/released/NUC029xEE_SPI_Mode_Swtich/blob/main/mode_2.jpg)
	
	
below is MODE 3	
![image](https://github.com/released/NUC029xEE_SPI_Mode_Swtich/blob/main/mode_3.jpg)	


3. use define : MANUAL_SWITCH_MOSI_TO_GPIO , to manual change MOSI PIN to high when SPI IDLE

![image](https://github.com/released/NUC029xEE_SPI_Mode_Swtich/blob/main/gpio_function_swap.jpg)


4. below is log capture 

![image](https://github.com/released/NUC029xEE_SPI_Mode_Swtich/blob/main/log.jpg)

