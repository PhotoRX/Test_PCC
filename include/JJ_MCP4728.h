#pragma once
#ifndef JJ_MCP4728_H
#define JJ_MCP4728_H

#include "Arduino.h"
#include <Wire.h>

class JJ_MCP4728 {
	public:
		enum class CMD {
			FAST_WRITE		= 0x00,
			MULTI_WRITE		= 0x40,
			SINGLE_WRITE	= 0x58,
			SEQ_WRITE		= 0x50,
			SELECT_VREF		= 0x80,
			SELECT_GAIN		= 0xC0,
			SELECT_PWRDOWN	= 0xA0
		};
		
		enum class VREF { VDD, INTERNAL_2_8V };
		enum class PWR_DOWN { NORMAL, GND_1KOHM, GND_100KOHM, GND_500KOHM };		
		//Usando la referencia externa (VDD) de Vdd:
		//Solo funciona X1 = 0mV a Vdd (Vdd puede ser de 2V7 a 5V5)
		//Usando la referencia interna (INTERNAL_2_8V) de 2.048mV:
		//X1 = 0mV a 2.048mV (0,5mV*LSB)		X2 = 0mV a 4.096mV (1mV*LSB)
		enum class GAIN { X1, X2 };

		void begin(TwoWire& oTW, uint8_t uPin) {
			_poTW = &oTW;
			_uPinLDAC = uPin;
			pinMode(_uPinLDAC, OUTPUT);
			enable(false);
			readRegisters();
		}

		void enable(bool bAct) {	digitalWrite(_uPinLDAC, !bAct);	}

		uint8_t analogWrite(uint8_t uCanal, uint16_t uDato, bool bEPP = false) {
			if (bEPP) {
				_sidEEP[uCanal].dato = uDato;
				return singleWrite(uCanal);
			}
			else {
				_sidReg[uCanal].dato = uDato;
				return fastWrite();
			}
		}

		uint8_t analogWrite(uint16_t a, uint16_t b, uint16_t c, uint16_t d, bool bEPP = false) {
			if (bEPP) {
				_sidReg[0].dato = _sidEEP[0].dato = a;
				_sidReg[1].dato = _sidEEP[1].dato = b;
				_sidReg[2].dato = _sidEEP[2].dato = c;
				_sidReg[3].dato = _sidEEP[3].dato = d;
				return seqWrite();
			}
			else {
				_sidReg[0].dato = a;
				_sidReg[1].dato = b;
				_sidReg[2].dato = c;
				_sidReg[3].dato = d;
				return fastWrite();
			}
		}

		uint8_t selectVref(VREF a, VREF b, VREF c, VREF d) {
			_sidReg[0].vref = a;
			_sidReg[1].vref = b;
			_sidReg[2].vref = c;
			_sidReg[3].vref = d;

			uint8_t uDato = (uint8_t)CMD::SELECT_VREF;
			for (uint8_t b1=0; b1<4; ++b1) {
				bitWrite(uDato, 3 - b1, (uint8_t)_sidReg[b1].vref);
			}

			_poTW->beginTransmission(_uAddr);
			_poTW->write(uDato);
			return _poTW->endTransmission();
		}

		uint8_t selectPowerDown(PWR_DOWN a, PWR_DOWN b, PWR_DOWN c, PWR_DOWN d) {
			_sidReg[0].pd = a;
			_sidReg[1].pd = b;
			_sidReg[2].pd = c;
			_sidReg[3].pd = d;

			uint8_t h = ((uint8_t)CMD::SELECT_PWRDOWN) | ((uint8_t)a << 2) | (uint8_t)b;
			uint8_t l = 0 | ((uint8_t)c << 6) | ((uint8_t)d << 4);

			_poTW->beginTransmission(_uAddr);
			_poTW->write(h);
			_poTW->write(l);
			return _poTW->endTransmission();
		}

		uint8_t selectGain(GAIN a, GAIN b, GAIN c, GAIN d) {
			_sidReg[0].gain = a;
			_sidReg[1].gain = b;
			_sidReg[2].gain = c;
			_sidReg[3].gain = d;

			uint8_t uDato = (uint8_t)CMD::SELECT_GAIN;
			for (uint8_t b1=0; b1<4; ++b1) {
				bitWrite(uDato, 3 - b1, (uint8_t)_sidReg[b1].gain);
			}

			_poTW->beginTransmission(_uAddr);
			_poTW->write(uDato);
			return _poTW->endTransmission();
		}

		void setID(uint8_t uID) {
			_uAddr = I2C_ADDR + uID;
		}

		void readRegisters() {
			_poTW->requestFrom((int)_uAddr, 24);
			if (_poTW->available()==24) {
				for (uint8_t b1=0; b1<8; ++b1) {
					uint8_t uDato[3];
					bool isEeprom = b1 % 2;
					
					for (uint8_t b1=0; b1<3; ++b1) {
						uDato[b1] = _poTW->read();
					}

					uint8_t uCanal = (uDato[0] & 0x30) >> 4;
					if (isEeprom) {
						_sidLeeEPP[uCanal].vref = (VREF)    ((uDato[1] & 0b10000000) >> 7);
						_sidLeeEPP[uCanal].pd   = (PWR_DOWN)((uDato[1] & 0b01100000) >> 5);
						_sidLeeEPP[uCanal].gain = (GAIN)    ((uDato[1] & 0b00010000) >> 4);
						_sidLeeEPP[uCanal].dato = (uint16_t)((uDato[1] & 0b00001111) << 8 | uDato[2]);
					}
					else {
						_sidLeeReg[uCanal].vref = (VREF)    ((uDato[1] & 0b10000000) >> 7);
						_sidLeeReg[uCanal].pd   = (PWR_DOWN)((uDato[1] & 0b01100000) >> 5);
						_sidLeeReg[uCanal].gain = (GAIN)    ((uDato[1] & 0b00010000) >> 4);
						_sidLeeReg[uCanal].dato = (uint16_t)((uDato[1] & 0b00001111) << 8 | uDato[2]);
					}
				}
			}
		}

		uint8_t getVref(uint8_t uCanal, bool bEPP = false) {
			return bEPP ? (uint8_t)_sidLeeEPP[uCanal].vref : (uint8_t)_sidLeeReg[uCanal].vref;
		}
		uint8_t getGain(uint8_t uCanal, bool bEPP = false) {
			return bEPP ? (uint8_t)_sidLeeEPP[uCanal].gain: (uint8_t)_sidLeeReg[uCanal].gain;
		}
		uint8_t getPowerDown(uint8_t uCanal, bool bEPP = false) {
			return bEPP ? (uint8_t)_sidLeeEPP[uCanal].pd : (uint8_t)_sidLeeReg[uCanal].pd;
		}
		uint16_t getDACData(uint8_t uCanal, bool bEPP = false) {
			return bEPP ? (uint16_t)_sidLeeEPP[uCanal].dato : (uint16_t)_sidLeeReg[uCanal].dato;
		}

	private:

		uint8_t fastWrite() {
			_poTW->beginTransmission(_uAddr);
			for (uint8_t b1 = 0; b1 < 4; ++b1) {
				_poTW->write((uint8_t)CMD::FAST_WRITE | highByte(_sidReg[b1].dato));
				_poTW->write(lowByte(_sidReg[b1].dato));
			}
			return _poTW->endTransmission();
		}

		uint8_t multiWrite() {
			_poTW->beginTransmission(_uAddr);
			for (uint8_t b1=0; b1<4; ++b1) {
				_poTW->write((uint8_t)CMD::MULTI_WRITE | (b1 << 1));
				_poTW->write(((uint8_t)_sidReg[b1].vref << 7)
					| ((uint8_t)_sidReg[b1].pd << 5)
					| ((uint8_t)_sidReg[b1].gain << 4)
					| highByte(_sidReg[b1].dato));
				_poTW->write(lowByte(_sidReg[b1].dato));
			}
			return _poTW->endTransmission();
		}

		uint8_t seqWrite() {
			_poTW->beginTransmission(_uAddr);
			_poTW->write((uint8_t)CMD::SEQ_WRITE);
			for (uint8_t b1=0; b1<4; ++b1) {
				_poTW->write(((uint8_t)_sidEEP[b1].vref << 7)
					| ((uint8_t)_sidEEP[b1].pd << 5)
					| ((uint8_t)_sidEEP[b1].gain << 4)
					| highByte(_sidEEP[b1].dato));
				_poTW->write(lowByte(_sidEEP[b1].dato));
			}
			return _poTW->endTransmission();
		}

		uint8_t singleWrite(uint8_t uCanal) {
			_poTW->beginTransmission(_uAddr);
			_poTW->write((uint8_t)CMD::SINGLE_WRITE | (uCanal << 1));
			_poTW->write(((uint8_t)_sidEEP[uCanal].vref << 7)
				| ((uint8_t)_sidEEP[uCanal].pd << 5)
				| ((uint8_t)_sidEEP[uCanal].gain << 4)
				| highByte(_sidEEP[uCanal].dato));
			_poTW->write(lowByte(_sidEEP[uCanal].dato));
			return _poTW->endTransmission();
		}

	private:

		struct DACInputData {
			VREF		vref;
			PWR_DOWN	pd;
			GAIN		gain;
			uint16_t	dato;
		};

		const uint8_t	I2C_ADDR {0x60};
		uint8_t			_uAddr {I2C_ADDR};
		uint8_t			_uPinLDAC;
		
		DACInputData	_sidReg[4];
		DACInputData	_sidEEP[4];
		DACInputData	_sidLeeReg[4];
		DACInputData	_sidLeeEPP[4];

		TwoWire * _poTW;
};

#endif // JJ_MCP4728_H
