#ifndef _PAC1934_H_
#define _PAC1934_H_

#include "Arduino.h"
#include "Wire.h"

/** default I2C address **/
#define PAC1934_I2CAddress (0x10)

extern uint8_t _PAC1934_OK;
extern uint8_t _PAC1934_ERR;

//REGISTERS
extern uint8_t _PAC1934_REFRESH_CMD;                             /**< Refresh command */
extern uint8_t _PAC1934_CTRL_REG;                             /**< Control register */

extern uint8_t _PAC1934_ACC_COUNT;                             /**< Accumulator count */
extern uint8_t _PAC1934_VPOWER1_ACC;                             /**< Accumulated power 1 */
extern uint8_t _PAC1934_VPOWER2_ACC;                             /**< Accumulated power 2 */
extern uint8_t _PAC1934_VPOWER3_ACC;                             /**< Accumulated power 3 */
extern uint8_t _PAC1934_VPOWER4_ACC;                             /**< Accumulated power 4 */

extern uint8_t _PAC1934_VBUS1;                             /**< Most recent Vbus sample 1 */
extern uint8_t _PAC1934_VBUS2;                             /**< Most recent Vbus sample 2 */
extern uint8_t _PAC1934_VBUS3;                             /**< Most recent Vbus sample 3 */
extern uint8_t _PAC1934_VBUS4;                             /**< Most recent Vbus sample 4 */
extern uint8_t _PAC1934_VSENSE1;                             /**< Most recent Vsense sample 1 */
extern uint8_t _PAC1934_VSENSE2;                             /**< Most recent Vsense sample 2 */
extern uint8_t _PAC1934_VSENSE3;                             /**< Most recent Vsense sample 3 */
extern uint8_t _PAC1934_VSENSE4;                             /**< Most recent Vsense sample 4 */

extern uint8_t _PAC1934_VBUS1_AVG;                             /**< Average Vbus sample 1 */
extern uint8_t _PAC1934_VBUS2_AVG;                             /**< Average Vbus sample 2 */
extern uint8_t _PAC1934_VBUS3_AVG;                             /**< Average Vbus sample 3 */
extern uint8_t _PAC1934_VBUS4_AVG;                             /**< Average Vbus sample 4 */
extern uint8_t _PAC1934_VSENSE1_AVG;                             /**< Average Vsense sample 1 */
extern uint8_t _PAC1934_VSENSE2_AVG;                             /**< Average Vsense sample 2 */
extern uint8_t _PAC1934_VSENSE3_AVG;                             /**< Average Vsense sample 3 */
extern uint8_t _PAC1934_VSENSE4_AVG;                             /**< Average Vsense sample 4 */

extern uint8_t _PAC1934_VPOWER1;                             /**< Vbus and Vsense product 1 */
extern uint8_t _PAC1934_VPOWER2;                             /**< Vbus and Vsense product 2 */
extern uint8_t _PAC1934_VPOWER3;                             /**< Vbus and Vsense product 3 */
extern uint8_t _PAC1934_VPOWER4;                             /**< Vbus and Vsense product 4 */

extern uint8_t _PAC1934_CHANNEL_DIS;                         /**< Channel disable and other settings */
extern uint8_t _PAC1934_NEG_PWR;                         /**< Bidirectional measurements settings */

extern uint8_t _PAC1934_REFRESH_G_CMD;                             /**< Refresh G command */
extern uint8_t _PAC1934_REFRESH_V_CMD;                             /**< Refresh V command */

extern uint8_t _PAC1934_SLOW;                             /**< Slow pin state and settings */

extern uint8_t _PAC1934_CTRL_ACT;                           /**< Actual state of CTRL register*/
extern uint8_t _PAC1934_DIS_ACT;                           /**< Actual state of CHANNEL_DIS register*/
extern uint8_t _PAC1934_NEG_PWR_ACT;                           /**< Actual state of NEG_PWR register*/

extern uint8_t _PAC1934_CTRL_LAT;                           /**< Latched state of CTRL register*/
extern uint8_t _PAC1934_DIS_LAT;                           /**< Latched state of CHANNEL_DIS register*/
extern uint8_t _PAC1934_NEG_PWR_LAT;                           /**< Latched state of NEG_PWR register*/

extern uint8_t _PAC1934_PRODUCT_ID;                           /**< Product ID register*/
extern uint8_t _PAC1934_MANUFACT_ID;                           /**< Manufacturer ID register*/
extern uint8_t _PAC1934_REVISION_ID;                           /**< Revision ID register*/

	class PAC1934 {
	public:
		PAC1934(uint8_t addr = PAC1934_I2CAddress);
		void begin(TwoWire *theWire = &Wire);
		void PAC1934_sendCommand(uint8_t command);
		void PAC1934_writeReg(uint8_t reg, uint16_t value);
		void PAC1934_readReg(uint8_t reg, uint16_t *value);
		float PAC1934_measureVoltage(uint8_t channel);
		float PAC1934_measureCurrent(uint8_t channel);
		float PAC1934_measurePower(uint8_t channel);
		float PAC1934_measureEnergy(uint8_t channel, uint16_t sampleRate);
	private:
TwoWire *_i2c;
 uint8_t PAC1934_i2caddr;
	};
#endif