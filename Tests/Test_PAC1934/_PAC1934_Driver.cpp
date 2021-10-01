
#include "Arduino.h"

#include "Wire.h"

#include "_PAC1934_Driver.h"

//uint8_t PAC1934_I2CAddress (0x11) //ou 0x10 ?

uint8_t _PAC1934_OK = 0;                                              /**< NO ERROR */
uint8_t _PAC1934_ERR = 1;                                              /**< ERROR OCCURED */

uint8_t _PAC1934_REFRESH_CMD = 0x00;                             /**< Refresh command */
uint8_t _PAC1934_CTRL_REG = 0x01;                             /**< Control register */

uint8_t _PAC1934_ACC_COUNT = 0x02;                             /**< Accumulator count */
uint8_t _PAC1934_VPOWER1_ACC = 0x03;                             /**< Accumulated power 1 */
uint8_t _PAC1934_VPOWER2_ACC = 0x04;                             /**< Accumulated power 2 */
uint8_t _PAC1934_VPOWER3_ACC = 0x05;                             /**< Accumulated power 3 */
uint8_t _PAC1934_VPOWER4_ACC = 0x06;                             /**< Accumulated power 4 */

uint8_t _PAC1934_VBUS1 = 0x07;                             /**< Most recent Vbus sample 1 */
uint8_t _PAC1934_VBUS2 = 0x08;                             /**< Most recent Vbus sample 2 */
uint8_t _PAC1934_VBUS3 = 0x09;                             /**< Most recent Vbus sample 3 */
uint8_t _PAC1934_VBUS4 = 0x0A;                             /**< Most recent Vbus sample 4 */
uint8_t _PAC1934_VSENSE1 = 0x0B;                             /**< Most recent Vsense sample 1 */
uint8_t _PAC1934_VSENSE2 = 0x0C;                             /**< Most recent Vsense sample 2 */
uint8_t _PAC1934_VSENSE3 = 0x0D;                             /**< Most recent Vsense sample 3 */
uint8_t _PAC1934_VSENSE4 = 0x0E;                             /**< Most recent Vsense sample 4 */

uint8_t _PAC1934_VBUS1_AVG = 0x0F;                             /**< Average Vbus sample 1 */
uint8_t _PAC1934_VBUS2_AVG = 0x10;                             /**< Average Vbus sample 2 */
uint8_t _PAC1934_VBUS3_AVG = 0x11;                             /**< Average Vbus sample 3 */
uint8_t _PAC1934_VBUS4_AVG = 0x12;                             /**< Average Vbus sample 4 */
uint8_t _PAC1934_VSENSE1_AVG = 0x13;                             /**< Average Vsense sample 1 */
uint8_t _PAC1934_VSENSE2_AVG = 0x14;                             /**< Average Vsense sample 2 */
uint8_t _PAC1934_VSENSE3_AVG = 0x15;                             /**< Average Vsense sample 3 */
uint8_t _PAC1934_VSENSE4_AVG = 0x16;                             /**< Average Vsense sample 4 */

uint8_t _PAC1934_VPOWER1 = 0x17;                             /**< Vbus and Vsense product 1 */
uint8_t _PAC1934_VPOWER2 = 0x18;                             /**< Vbus and Vsense product 2 */
uint8_t _PAC1934_VPOWER3 = 0x19;                             /**< Vbus and Vsense product 3 */
uint8_t _PAC1934_VPOWER4 = 0x1A;                             /**< Vbus and Vsense product 4 */

uint8_t _PAC1934_CHANNEL_DIS = 0x1C;                         /**< Channel disable and other settings */
uint8_t _PAC1934_NEG_PWR = 0x1D;                         /**< Bidirectional measurements settings */

uint8_t _PAC1934_REFRESH_G_CMD = 0x1E;                             /**< Refresh G command */
uint8_t _PAC1934_REFRESH_V_CMD = 0x1F;                             /**< Refresh V command */

uint8_t _PAC1934_SLOW = 0x20;                             /**< Slow pin state and settings */

uint8_t _PAC1934_CTRL_ACT = 0x21;                           /**< Actual state of CTRL register*/
uint8_t _PAC1934_DIS_ACT = 0x22;                           /**< Actual state of CHANNEL_DIS register*/
uint8_t _PAC1934_NEG_PWR_ACT = 0x23;                           /**< Actual state of NEG_PWR register*/

uint8_t _PAC1934_CTRL_LAT = 0x24;                           /**< Latched state of CTRL register*/
uint8_t _PAC1934_DIS_LAT = 0x25;                           /**< Latched state of CHANNEL_DIS register*/
uint8_t _PAC1934_NEG_PWR_LAT = 0x26;                           /**< Latched state of NEG_PWR register*/

uint8_t _PAC1934_PRODUCT_ID = 0x24;                           /**< Product ID register*/
uint8_t _PAC1934_MANUFACT_ID = 0x25;                           /**< Manufacturer ID register*/
uint8_t _PAC1934_REVISION_ID = 0x26;                           /**< Revision ID register*/

PAC1934::PAC1934(uint8_t addr) {
 PAC1934_i2caddr = addr;
}


void PAC1934::begin(TwoWire *theWire)
{
	_i2c = theWire;
	//_i2c->begin();

}


void PAC1934::PAC1934_sendCommand(uint8_t command)
{
	

	if (command == _PAC1934_REFRESH_G_CMD)
	{
		_i2c->beginTransmission(0x00);
		_i2c->write(command);
		_i2c->endTransmission(false);
	}
	else
	{
		_i2c->beginTransmission(PAC1934_i2caddr);
		_i2c->write(command);
		_i2c->endTransmission(false);
	}

}

void PAC1934::PAC1934_writeReg(uint8_t reg, uint16_t value)
{
	_i2c->beginTransmission(PAC1934_i2caddr);
	_i2c->write(reg);                 // Register
	_i2c->write((value >> 8) & 0xFF); // Upper 8-bits
	_i2c->write(value & 0xFF);        // Lower 8-bits
	_i2c->endTransmission(false);
}

void PAC1934::PAC1934_readReg(uint8_t reg, uint16_t *value)
{
	_i2c->beginTransmission(PAC1934_i2caddr);
	_i2c->write(reg); // Register
	_i2c->endTransmission(false);

	_i2c->requestFrom(PAC1934_i2caddr, (uint8_t)2);
	// Shift values to create properly formed integer
	*value = ((_i2c->read() << 8) | _i2c->read());

}


float PAC1934::PAC1934_measureVoltage(uint8_t channel)
{
  Serial.println("");
	Serial.print("sensing voltage on I2C adresse : ");
	Serial.println(PAC1934_i2caddr);


	uint8_t reg = 0;
	uint16_t returnInt;
	float returnValue;


	reg = channel + 0x06;
	
	
	  _i2c->beginTransmission(PAC1934_i2caddr);
	_i2c->write(reg); // Register
	 _i2c->endTransmission(false);
	
	delay(20);

	_i2c->requestFrom(PAC1934_i2caddr, 2);
	delay(20);
	returnInt = Wire.read() << 8;
	returnInt = returnInt | Wire.read();
	// Shift values to create properly formed integer

	returnValue = returnInt;
	returnValue = returnValue * 32;                  //FSV
	returnValue = returnValue / 0xFFFF;

	return returnValue;
}

float PAC1934::PAC1934_measureCurrent(uint8_t channel)
{
	uint8_t reg = 0;
	uint16_t returnInt;
	float returnValue;

	reg = channel + 0x0A;

	_i2c->beginTransmission(0x0B);
	_i2c->write(reg); // Register
	
	int error = _i2c->endTransmission(false);
	delay(1);
 Serial.println("");
Serial.print("error code: ");
	Serial.println(error);
	Serial.println(_i2c->available());
	delay(1);
	
	_i2c->requestFrom(0x0B, 2);
	delay(1);
	Serial.println(_i2c->available());
	delay(1);
	returnInt = Wire.read() << 8;
	returnInt = returnInt | Wire.read();

	returnValue = returnInt;
	Serial.println(returnValue);
	returnValue = returnValue * 25000;              // FSC
	returnValue = returnValue / 0xFFFF;

	return returnValue;
}


float PAC1934::PAC1934_measurePower(uint8_t channel)
{
	uint8_t reg = 0;
	uint32_t returnInt;
	float returnValue;


	reg = channel + 0x16;


	_i2c->beginTransmission(PAC1934_i2caddr);
	_i2c->write(reg); // Register
	_i2c->endTransmission();
	delay(1);
	_i2c->requestFrom(PAC1934_i2caddr, (uint8_t)4);
	// Shift values to create properly formed integer
	returnInt = ((_i2c->read() << 8) | (_i2c->read() << 8) | (_i2c->read() << 4) | _i2c->read());

	returnValue = returnInt;
	returnValue = returnValue * 800;            // PowerFSR
	returnValue = returnValue / 0xFFFFFFF;

	return returnValue;
}

float PAC1934::PAC1934_measureEnergy(uint8_t channel, uint16_t sampleRate)
{
	uint8_t reg = 0;
	uint64_t accumulatorValue;
	float returnValue;

	//Count register needs to be read, even when not used.
	reg = 0x02;
	_i2c->beginTransmission(PAC1934_i2caddr);
	_i2c->write(reg); // Register
	_i2c->endTransmission();
	delay(1);
	_i2c->requestFrom(PAC1934_i2caddr, (uint8_t)3);
	// Shift values to create properly formed integer
	accumulatorValue = ((_i2c->read() << 8) | (_i2c->read() << 8) | _i2c->read());


	reg = channel + 0x02;

	_i2c->beginTransmission(PAC1934_i2caddr);
	_i2c->write(reg); // Register
	_i2c->endTransmission();
	delay(1);
	_i2c->requestFrom(PAC1934_i2caddr, (uint8_t)6);
	accumulatorValue = accumulatorValue | ((_i2c->read() << 8) | (_i2c->read() << 8) | (_i2c->read() << 8) | (_i2c->read() << 8) | (_i2c->read() << 8) | _i2c->read());

	returnValue = accumulatorValue;
	returnValue = returnValue * 800;                           // PowerFSR
	returnValue = returnValue / 0xFFFFFFF;
	returnValue = returnValue / sampleRate;                    // 1/sample rate

	return returnValue;
}
