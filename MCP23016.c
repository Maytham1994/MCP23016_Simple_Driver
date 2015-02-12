/*
* MCP23016 16-Bit I/O Expander Library
* Author: Maytham Sabbagh
* Revision Date: February 12th, 2015
*/

#include <Wire.h>

/* The A2,A1,A0 pins are connected to ground, therefore the address is 0x20
* The address is 0b0100[A2][A1][A0]
*/
#define IOAddress 0x20
// Addresses for the GP0 and GP1 registers in the MCP23016
#define GP0	0x00
#define GP1	0x01
// When set to 1, the port is INPUT, when set to 0, the port is OUTPUT.
#define IODIR0 0x06
#define IODIR1 0x07

// Setting all the pins in a port to either output, or input
#define ALL_OUTPUT	0x00
#define ALL_INPUT	0xFF

struct IOState{
  // These variables hold the current state of each of the two ports, directions of the I/O
  unsigned char GP0CurrentState;
  unsigned char GP1CurrentState;
};

struct IOPortValues{
  // These variables hold the current state of each of the two ports, directions of the I/O
  unsigned char IOPort0;
  unsigned char IOPort1;
};

// This function start the transmission I2C protocol
void Start_I2C(){
	Wire.begin();
}

/*
*	This function sets the direction of the two I/O ports
*	Parameters: GP0State and GP1State are the desired states for the two ports\
*	This function updates the two current state parameters
*/
struct IOState MCP_SetDirection(unsigned char GP0State, unsigned char GP1State){
	IOState CurrentState;
        // Update the two CurrentState parameters
	CurrentState.GP0CurrentState = GP0State;
	CurrentState.GP1CurrentState = GP1State;
	// Start I2C transmission with the device
	Wire.beginTransmission(IOAddress);
	// Selecting IODIR0 register to write to
	Wire.write(IODIR0);
	// Setting the state for port0
	Wire.write(GP0State);
	// Selecting IODIR1 register to write to
	Wire.write(IODIR1);
	// Setting the state for port1
	Wire.write(GP1State);
	// Ending I2C transmission
	Wire.endTransmission();

        return CurrentState;
}

/*
*	This function writes data to the ports on the I/O expander
*	Parameters: port (true for port1, false for port0), data: holds the data desired to be writted
*/
void MCP_WritePort(bool port, unsigned char data){
	// Checking if the desired port is 0 or 1
	if(port){
		// Start the transmission with the device
		Wire.beginTransmission(IOAddress);
		// Select port1 to write to
		Wire.write(GP1);
		// Write the data to port1
		Wire.write(data);
		// End the transmission
		Wire.endTransmission();
	}else{
		// Start the transmission with the device
		Wire.beginTransmission(IOAddress);
		// Select port0 to write to
		Wire.write(GP0);
		// Write the data to port0
		Wire.write(data);
		// End the transmission
		Wire.endTransmission();
	}
}

/*
*	This function reads the data from ports 0 and 1 and returns an array with the data
*	Parameters: port (true for port1, false for port0)
*	Return: array with data from port0 and port1
*/
struct IOPortValues MCP_ReadPorts(bool port){
        IOPortValues result;
	// Temporary array to hold he values
	unsigned char array[2];
	// Temporarily save 0 in both values
	array[0]=0x00;
	array[1]=0x00;
	// Start transmission with the device
	Wire.beginTransmission(IOAddress);
	// Request the device for the number of bytes, if port1 then the bytes are 2, if port0 then the bytes are 1
	Wire.requestFrom(IOAddress, port+1);
	// Save the data in the array
	for(int i = 0; Wire.available(); i++){
		array[i] = Wire.read();
	}
        result.IOPort0 = array[0];
        result.IOPort1 = array[1];
	// Return the array
	return result;
}

/*
*	This function is specific to the H-Bridge used for Group Hotel in ELEC3907 Project
*	Parameters: AIN1,AIN2,BIN1,BIN2 are specific for the H-Bridge
*/
void Motor_Directions(bool AIN1, bool AIN2, bool BIN1, bool BIN2){
	// Read the current values in the port
	IOPortValues values = MCP_ReadPorts(0);
	// Save the port in a temporary direction variable
	unsigned char directions = values.IOPort0;
	// Temporary Direction Variables
	unsigned char temp1 = 0x0F;
	unsigned char temp2 = 0x00;
	// Set AIN1
	temp1 |= AIN1 << 7;
	temp2 |= AIN1 << 7;
	// Set AIN2
	temp1 |= AIN2 << 6;
	temp2 |= AIN2 << 6;
	// Set BIN1
	temp1 |= BIN1 << 5;
	temp2 |= BIN1 << 5;
	// Set BIN2
	temp1 |= BIN2 << 4;
	temp2 |= BIN2 << 4;
	
	// ANDing the current values with temp1
	directions &= temp1;
	// ORing the current values with temp2
	directions |= temp2;
	
	/*
	*	Code above does this;
	*	If current output is 0b10101010, and the desired 4 bits are 0b1100
	*	Set temp1 to 0b11001111, and temp2 to 0b11000000
	*	Then when ANDing 0b10101010 & 0b11001111 = 0b10001010
	*	Then ORing 0b10001010 | 0b11000000 = 11001010
	*	Therefore direction is now 0b11001010, therefore only the 4 bits changed and the rest remained the same
	*/
	// Call the write function to write the data to port0
	MCP_WritePort(0, directions);
}