
#include <altera_avalon_pio_regs.h>
#include <system.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <io.h>
#include <time.h>

#define START_BIT 			14
#define WRITE_BIT 			13
#define DATA_BIT_0 			12
#define DATA_BIT_7 			5
#define INTERRUPT_BIT 		4
#define WAIT_BIT 			3
#define DATA_STROBE_BIT 	2
#define RESET_BIT 			1
#define ADDRESS_STROBE_BIT 	0
#define GPIO_base (volatile char *) 0x00002400

typedef struct{
	unsigned short start 			: 1;
	unsigned short write 			: 1;
	unsigned short data[8] 			;
	unsigned short interrupt 		: 1;
	unsigned short wait 			: 1;
	unsigned short data_strobe 		: 1;
	unsigned short reset 			: 1;
	unsigned short address_strobe 	: 1;
} parallel_port;
parallel_port GPIO;
parallel_port bus_read(void){
	parallel_port temp;
	unsigned int bus = 0;
	unsigned int compare = 0x0001;
	bus = IORD_ALTERA_AVALON_PIO_DATA(GPIO_base);
	//printf("bus is %u \n", bus);
	temp.address_strobe = bus&compare;
	//printf("address strobe is %u \n", temp.address_strobe);
	bus = bus >> 1;
	temp.reset = bus&compare;
	//printf("reset is %u \n", temp.reset);
	bus = bus >> 1;
	temp.data_strobe = bus&compare;
	//printf("data strobe is %u \n",temp.data_strobe);
	bus = bus >> 1;
	temp.wait = bus&compare;
	//printf("wait is %u \n", temp.wait);
	bus = bus >> 1;
	temp.interrupt = bus&compare;
	//printf("interrupt is %u \n", temp.interrupt);
	int i = 0;
	for ( i=7; i>=0;i-- ){
		bus = bus >> 1;
		temp.data[i] = bus&compare;
		//printf("data[%d] is %u \n", i, temp.data[i]);
	}
	bus = bus >> 1;
	temp.write = bus&compare;
	//printf("write is %u \n", temp.write);
	bus = bus >> 1;
	temp.start = bus&compare;
	//printf("start is %u \n", temp.start);

	return temp;
}
unsigned int* data_read(){
	unsigned int temp = 0;
	parallel_port GPIO;
	IOWR_ALTERA_AVALON_PIO_DIRECTION(GPIO_base, 0x0018);
	GPIO = bus_read();
	int x = 0;
	for (x=0; x<= 7; x++){
		temp += GPIO.data[x]<<x;
	}
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0000);
	while(1){
		GPIO = bus_read();
		if (GPIO.start == 0){
			break;
		}
	}
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0018);
	return temp;

}

unsigned int* address_read(unsigned int address_read[]){
	parallel_port GPIO;
	IOWR_ALTERA_AVALON_PIO_DIRECTION(GPIO_base, 0x0018);
	GPIO = bus_read();
	int x = 0;
	for (x=0; x<= 7; x++){
		address_read[x] = GPIO.data[x];
	}
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0000);
	return address_read;
}

void data_write(data_write){
	unsigned int temp;
	IOWR_ALTERA_AVALON_PIO_DIRECTION(GPIO_base, 0x1FF8);
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0018);
	temp = data_write <<5;
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, temp);
	while(1){
		parallel_port GPIO;
		GPIO = bus_read();
		if (GPIO.start == 0 ){
			IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0018);
			break;
		}
	}


}

void address_write(address_write){
	unsigned int temp;
	IOWR_ALTERA_AVALON_PIO_DIRECTION(GPIO_base, 0x1FF8);
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0018);
	temp = address_write <<5;

	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, temp);
	while(1){
		parallel_port GPIO;
		GPIO = bus_read();
		if (GPIO.start == 0 ){
			IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0018);
			break;
		}
	}

}



int main (void){
//	IOWR_ALTERA_AVALON_PIO_DIRECTION(GPIO_base, 0x0018);
//IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0000);

/*

	IOWR_ALTERA_AVALON_PIO_DIRECTION(GPIO_base, 0x0000);
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0000);
	int i, j;
	while(1){
		GPIO = bus_read();
		for(i=0;i<10000;i++){
			for(j=0;j<33;j++);
		}
	}

*/
	unsigned int array_read[8];
	unsigned int data_fromread[8];
	unsigned int address_fromread[8];
	unsigned int data_fromwrite = 200;
	unsigned int address_fromwrite;
	unsigned int firstbyte;
	unsigned int lastbyte;
	unsigned int temp;
	unsigned int data_first;
	unsigned int data_last;
	int count =0;
	int count2 = 2;
	int count_read= 0;

	IOWR_ALTERA_AVALON_PIO_DIRECTION(GPIO_base, 0x0018);
	IOWR_ALTERA_AVALON_PIO_DATA(GPIO_base, 0x0018);
	parallel_port GPIO;
	while(1){
	do {
		GPIO = bus_read();
		//int i,j;
		//for(i=0;i<10000;i++){
		//	for(j=0;j<33;j++);
		//}

	}
	while (GPIO.start != 1);
	if (GPIO.write == 0){
		//do read
		if(GPIO.data_strobe == 1 && GPIO.address_strobe == 0){
			//printf ("data read mode \n");
			if (count == 0){
				firstbyte = data_read();
				//printf("read first byte %u \n", firstbyte);
			}
			if (count == 1){
				lastbyte = data_read();
				//printf("read last byte %u \n", lastbyte);
			}
			count++;
			if (count==2) {
					temp = firstbyte*256+lastbyte;
					array_read[count_read] = temp;
					//printf("An integer received is %u \n", temp);
					count = 0;
					count_read++;
					if (count_read == 8){
						count_read = 0;
						printf("The array received is ");
						int m;
						for (m=0; m <=7; m++){
							printf("%u  ",array_read[m]);
						}
					}
				}
		}else if(GPIO.data_strobe == 0 && GPIO.address_strobe == 1){
			address_read(address_fromread);
		}else{
			printf("data strobe and/or address strobe error!!!");
		}
	}else if(GPIO.write == 1){
		//do write
		if(GPIO.data_strobe == 1 && GPIO.address_strobe == 0){
			printf("data write mode \n");
			if (count2 == 2){
				data_fromwrite=6000;
				data_first = data_fromwrite/256;
				data_last = data_fromwrite%256;
				count2 = 0;
			}
			if (count2 == 0){
				data_write(data_first);
				printf("write first byte %u \n", data_first);
			}
			if (count2 == 1){
				data_write(data_last);
				printf("write last byte %u \n", data_last);
			}
			count2 ++;

			data_write(data_fromwrite);
		}else if(GPIO.data_strobe == 0 && GPIO.address_strobe == 1){
			address_write(address_fromwrite);
		}else{
			printf("data strobe and/or address strobe error!!!");
		}
	}else{
		printf(" write pin error!");
	}


	}

	return 0;

}




