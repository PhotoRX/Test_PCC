#pragma once
//
//    FILE: ADS1X15.H
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.7
//    DATE: 2013-03-24
// PUPROSE: Arduino library for ADS1015 and ADS1115
//     URL: https://github.com/RobTillaart/ADS1X15
//  MODIFICADO  Juan José Plá 20210508
//

#include "Arduino.h"
#include "Wire.h"

// allow compile time default address
// address in { 0x48, 0x49, 0x4A, 0x4B }, no test...
#ifndef ADS1015_ADDRESS
  #define ADS1015_ADDRESS     0x48
#endif

#ifndef ADS1115_ADDRESS
  #define ADS1115_ADDRESS     0x48
#endif

#define ADS1X1X_OK                  0
//En algunas funciones se pasa de -100 a -10000 porque -100 puede ser un valor válido
#define ADS1X1X_INVALID_VOLTAGE   -100 
#define ADS1X1X_INVALID_GAIN    0xFF
#define ADS1X1X_INVALID_MODE    0xFE

#define ADS1015_CONVERSION_DELAY  1
#define ADS1115_CONVERSION_DELAY  8


// Kept #defines a bit in line with Adafruits library.

// REGISTERS
#define ADS1X1X_REG_CONVERT     0x00
#define ADS1X1X_REG_CONFIG      0x01
#define ADS1X1X_REG_LOW_THRESHOLD 0x02
#define ADS1X1X_REG_HIGH_THRESHOLD  0x03


// CONFIG REGISTER

// BIT 15       Operational Status         // 1 << 15
#define ADS1X1X_OS_BUSY       0x0000
#define ADS1X1X_OS_NOT_BUSY     0x8000
#define ADS1X1X_OS_START_SINGLE   0x8000

// BIT 12-14    read differential
#define ADS1X15_MUX_DIFF_0_1    0x0000
#define ADS1X15_MUX_DIFF_0_3    0x1000
#define ADS1X15_MUX_DIFF_1_3    0x2000
#define ADS1X15_MUX_DIFF_2_3    0x3000
//              read single
#define ADS1X15_READ_0        0x4000      //pin << 12
#define ADS1X15_READ_1        0x5000      //pin = 0..3
#define ADS1X15_READ_2        0x6000
#define ADS1X15_READ_3        0x7000


// BIT 8        mode                        // 1 << 8
#define ADS1X1X_MODE_CONTINUE   0x0000
#define ADS1X1X_MODE_SINGLE     0x0100

// BIT 5-7      datarate sample per second  // (0..7) << 5


//BIT 9-11  ganancia  (0..5) << 9
#define ADS1X1X_RANGO_6144      0x0000      //0 = ±6.144mV  defecto
#define ADS1X1X_RANGO_4096      0x0200      //1 = ±4.096mV
#define ADS1X1X_RANGO_2048      0x0400      //2 = ±2.048mV
#define ADS1X1X_RANGO_1024      0x0600      //4 = ±1.024mV
#define ADS1X1X_RANGO_512     0x0800      //8 =   ±512mV
#define ADS1X1X_RANGO_256     0x0A00      //16 =  ±256mV

/*
differs for different devices, check datasheet or readme.md
| datarate | ADS101x | ADS 111x |
|:----:|----:|----:|
| 0 | 128  | 8   |
| 1 | 250  | 16  |
| 2 | 490  | 32  |
| 3 | 920  | 64  |
| 4 | 1600 | 128 |
| 5 | 2400 | 250 |
| 6 | 3300 | 475 |
| 7 | 3300 | 860 |
*/

// BIT 4 comparator modi                    // 1 << 4
#define ADS1X1X_COMP_MODE_TRADITIONAL   0x0000
#define ADS1X1X_COMP_MODE_WINDOW        0x0010

// BIT 3 ALERT active value                 // 1 << 3
#define ADS1X1X_COMP_POL_ACTIV_LOW      0x0000
#define ADS1X1X_COMP_POL_ACTIV_HIGH     0x0008

// BIT 2 ALERT latching                     // 1 << 2
#define ADS1X1X_COMP_NON_LATCH          0x0000
#define ADS1X1X_COMP_LATCH              0x0004

// BIT 0-1 ALERT mode                       // (0..3)
#define ADS1X1X_COMP_QUE_1_CONV         0x0000    // trigger alert after 1 convert
#define ADS1X1X_COMP_QUE_2_CONV         0x0001    // trigger alert after 2 converts
#define ADS1X1X_COMP_QUE_4_CONV         0x0002    // trigger alert after 4 converts
#define ADS1X1X_COMP_QUE_NONE           0x0003    // dosable comparator

// _CONFIG masks
//
// | bit  | description |
// |:----:|:----|
// |  0   | # channels |
// |  1   | -  |
// |  2   | resolution |
// |  3   | - |
// |  4   | GAIN supported |
// |  5   | COMPARATOR supported |
// |  6   | - |
// |  7   | - |
#define ADS_CONF_CHAN_1  0x00
#define ADS_CONF_CHAN_4  0x01
#define ADS_CONF_RES_12  0x00
#define ADS_CONF_RES_16  0x04
#define ADS_CONF_NOGAIN  0x00
#define ADS_CONF_GAIN    0x10
#define ADS_CONF_NOCOMP  0x00
#define ADS_CONF_COMP    0x20

//Medidas por segundo en moco continuo dependiendo del modelo de IC y la velocidad elegida 
static const int16_t  iSPS101x[] = { 128, 250, 490, 920, 1600, 2400, 3300, 3300 };
static const int16_t  iSPS111x[] = { 8, 16, 32, 64, 128, 250, 475, 860 };

class JJ_ADS1X15 {
  
  public:
    bool begin(TwoWire& oTW) {
      if ((_address < 0x48) || (_address > 0x4B)) return false;
      _poTW = &oTW;     
      return true;
    } 

    bool isBusy() {
      uint16_t val = readRegister(_address, ADS1X1X_REG_CONFIG);
      if ((val & ADS1X1X_OS_NOT_BUSY) != 0)   return false;
      return true;
    }
    bool isReady() {  return isBusy() == false; };

    bool isConnected() {
      _poTW->beginTransmission(_address);
      return (_poTW->endTransmission() == 0);
    }

    //Permite aceptar valores de gain de 0,1,2,3,4,5, como ADS1X1X_RANGO_6144, ADS1X1X_RANGO_4096, etc indistintamrente.
    //Impide aceptar ganancias invalidas.
    void setGain(uint16_t gain) {
      if (!(_config & ADS_CONF_GAIN))       gain = 0;
      
      if (gain<=16) {
        switch (gain) {
          default:  // catch invalid values and go for the safest gain.
          case 0: _gain = ADS1X1X_RANGO_6144; break;  //0 = ±6.144mV  default
          case 1: _gain = ADS1X1X_RANGO_4096; break;  //1 = ±4.096mV
          case 2: _gain = ADS1X1X_RANGO_2048; break;  //2 = ±2.048mV
          case 3: _gain = ADS1X1X_RANGO_1024; break;  //3 = ±1.024mV
          case 4: _gain = ADS1X1X_RANGO_512;  break;  //4 =   ±512mV
          case 5: _gain = ADS1X1X_RANGO_256;  break;  //5 =   ±256mV
        }
      }
      else {
        switch (gain) {
          case ADS1X1X_RANGO_6144:      //0x0000 = ±6.144mV  default
          case ADS1X1X_RANGO_4096:      //0x0200 = ±4.096mV
          case ADS1X1X_RANGO_2048:      //0x0400 = ±2.048mV
          case ADS1X1X_RANGO_1024:      //0x0600 = ±1.024mV
          case ADS1X1X_RANGO_512:       //0x0800 =   ±512mV
          case ADS1X1X_RANGO_256:       //0x0A00 =   ±256mV
            _gain = gain;
            break;
          default:
            _gain = ADS1X1X_RANGO_6144;   //0x0000 = ±6.144mV  default
            break;
        }
      }
    }

    //
    uint8_t getGain() {
      if (!(_config & ADS_CONF_GAIN)) return 0;
      switch (_gain) {
        case ADS1X1X_RANGO_6144:    return 0;
        case ADS1X1X_RANGO_4096:    return 1;
        case ADS1X1X_RANGO_2048:    return 2;
        case ADS1X1X_RANGO_1024:    return 4;
        case ADS1X1X_RANGO_512:     return 8;
        case ADS1X1X_RANGO_256:     return 16;
      }
      _err = ADS1X1X_INVALID_GAIN;            //0xFF == invalid gain error.
      return _err;
    }
    
    //Puede devolver ADS1X1X_INVALID_VOLTAGE si la ganancia no es válida. 
    float toVoltage(int16_t val) {
      if (val == 0)   return 0;

      float volts = getMaxVoltage();
      if (volts < 0)    return volts;

      volts *= val;
      if (_config & ADS_CONF_RES_16) {
        volts /= 32767;       //val = 16 bits - sign bit = 15 bits mantissa
      }
      else {
        volts /= 2047;        //val = 12 bits - sign bit = 11 bit mantissa
      }
      return volts;
    }   

    //Puede devolver ADS1X1X_INVALID_VOLTAGE si la ganancia no es válida. 
    float getMaxVoltage() {
      switch (_gain) {
        case ADS1X1X_RANGO_6144:  return 6.144;
        case ADS1X1X_RANGO_4096:  return 4.096;
        case ADS1X1X_RANGO_2048:  return 2.048;
        case ADS1X1X_RANGO_1024:  return 1.024;
        case ADS1X1X_RANGO_512:   return 0.512;
        case ADS1X1X_RANGO_256:   return 0.256;
      }
      _err = ADS1X1X_INVALID_VOLTAGE;
      return _err;            //-10000 = voltaje no válido.
    }
    
    int16_t getMax_mV() {
      switch (_gain) {
        case ADS1X1X_RANGO_6144:  return 6144;
        case ADS1X1X_RANGO_4096:  return 4096;
        case ADS1X1X_RANGO_2048:  return 2048;
        case ADS1X1X_RANGO_1024:  return 1024;
        case ADS1X1X_RANGO_512:   return 512;
        case ADS1X1X_RANGO_256:   return 256;
      }
      _err = ADS1X1X_INVALID_VOLTAGE;
      return _err * 100;            //-10000 = voltaje no válido.
    }
    
    //Resolución de la medida en µV -> µV * LSB
    int32_t getResolution() {
      int32_t iValRes = getMax_mV();
      if (iValRes==-10000)  return -10000;
      iValRes*= 2000;       //se pasa a µV y toda la escala
      //_bitShift-> ADS101x=4 y ADS111x=0
      return (_bitShift==0) ? (iValRes/65536) : (iValRes/4096);
    }
    //Devuelve el valor en µV, si error devuelve -10.000.000µV
    int32_t toMicroVolts(int16_t iEnt) {
      int32_t iRes = getResolution();
      return (iRes==-10000) ? -10000000 : (int32_t)iEnt * iRes;
    }
    //Devuelve el valor en  mV, si error devuelve -10.000mV
    int32_t toMilliVolts(int16_t iEnt) {  return (toMicroVolts(iEnt)/1000);   }
    
    char * getRango(char * tSal) {
      switch (_gain) {
        case ADS1X1X_RANGO_6144:    strcpy(tSal, "±6.144mV"); break;
        case ADS1X1X_RANGO_4096:    strcpy(tSal, "±4.096mV"); break;
        case ADS1X1X_RANGO_2048:    strcpy(tSal, "±2.048mV"); break;
        case ADS1X1X_RANGO_1024:    strcpy(tSal, "±1.024mV"); break;
        case ADS1X1X_RANGO_512:     strcpy(tSal, "±512mV");   break;
        case ADS1X1X_RANGO_256:     strcpy(tSal, "±256mV");   break;
        default:            strcpy(tSal, "Error");    break;
      }
      return tSal;
    }
    
    //Admite indistintamente 0, 1 o ADS1X1X_MODE_CONTINUE, ADS1X1X_MODE_SINGLE
    void setMode(uint16_t mode) {
      switch (mode) {
        case 0:   _mode = ADS1X1X_MODE_CONTINUE;  break;
        default:
        case 1:   _mode = ADS1X1X_MODE_SINGLE;  break;
      }
    }

    //Función sobrecargada, devuelve el mode en modo numérico o texto
    uint8_t getMode(void) {
      switch (_mode) {
        case ADS1X1X_MODE_CONTINUE:   return 0;
        case ADS1X1X_MODE_SINGLE:   return 1;
      }
      _err = ADS1X1X_INVALID_MODE;
      return _err;            //0xFE = modo no valido.
    }   
    char * getMode(char * tSal) {
      switch (_mode) {
        case ADS1X1X_MODE_CONTINUE:   strcpy(tSal, "Continuo"); break;
        case ADS1X1X_MODE_SINGLE:   strcpy(tSal, "Unico");  break;
        default:            strcpy(tSal, "Error");    break;
      }
      return tSal;
    }

    //Velocidad de muestreo 0=muy lento; 7=muy rápido; 4=predeterminado:
    void setDataRate(uint8_t dataRate) {
      _datarate = dataRate;
      if (_datarate > 7)  _datarate = 4;  // default
      _datarate <<= 5;          // convert 0..7 to mask needed.
    }

    //La velocidad real depende del dispositivo: 
    uint8_t getDataRate(void) {
      return (_datarate >> 5);      // convert mask back to 0..7
    }
    //devuelve el número de muestras por segundo, dependiendo del dispositivo y del valor actual.
    int16_t getNumSPS(void) {
      return (_bitShift==0) ? iSPS111x[getDataRate()] : iSPS101x[getDataRate()];
    }
    
    int16_t readADC(uint8_t pin) {
      if (pin >= _maxPorts)   return 0;
      uint16_t mode = ((4 + pin) << 12);  // pin to mask
      return _readADC(mode);
    }

    void  requestADC_Differential_0_1() { _requestADC(ADS1X15_MUX_DIFF_0_1);    }
    int16_t readADC_Differential_0_1() {  return _readADC(ADS1X15_MUX_DIFF_0_1);  }

    // ASYNC INTERFACE
    // requestADC(pin) -> isBusy() or isReady() -> getValue(); 
    // see examples
    void requestADC(uint8_t pin) {
      if (pin >= _maxPorts) return;
      uint16_t mode = ((4 + pin) << 12);  // pin to mask
      _requestADC(mode);
    }

    // used by continuous mode and async mode.
    int16_t getLastValue() { return getValue(); };  // will be obsolete in future
    int16_t getValue() {
      int16_t raw = readRegister(_address, ADS1X1X_REG_CONVERT);
      if (_bitShift)    raw >>= _bitShift;    // Shift 12-bit results
      return raw;
    }

    // COMPARATOR
    // 0    = TRADITIONAL   > high          => on      < low   => off
    // else = WINDOW        > high or < low => on      between => off
    void     setComparatorMode(uint8_t mode) {  _compMode = mode == 0 ? 0 : 1;  };
    uint8_t  getComparatorMode() {  return _compMode; };

    // 0    = LOW (default)
    // else = HIGH
    void     setComparatorPolarity(uint8_t pol) { _compPol = pol ? 0 : 1; };
    uint8_t  getComparatorPolarity() {  return _compPol;  };

    // 0    = NON LATCH
    // else = LATCH
    void     setComparatorLatch(uint8_t latch) {  _compLatch = latch ? 0 : 1; };
    uint8_t  getComparatorLatch() { return _compLatch;  };

    // 0   = trigger alert after 1 conversion
    // 1   = trigger alert after 2 conversions
    // 2   = trigegr alert after 4 conversions
    // 3   = Disable comparator =  default, also for all other values.
    void     setComparatorQueConvert(uint8_t mode) {  _compQueConvert = (mode < 3) ? mode : 3;  };
    uint8_t  getComparatorQueConvert() {  return _compQueConvert; };
    void setComparatorThresholdLow(int16_t lo) {  writeRegister(_address, ADS1X1X_REG_LOW_THRESHOLD, lo); };
    int16_t getComparatorThresholdLow() { return readRegister(_address, ADS1X1X_REG_LOW_THRESHOLD); };
    void setComparatorThresholdHigh(int16_t hi) { writeRegister(_address, ADS1X1X_REG_HIGH_THRESHOLD, hi);  };
    int16_t getComparatorThresholdHigh() {  return readRegister(_address, ADS1X1X_REG_HIGH_THRESHOLD);  };

    int8_t getError() {
      int8_t rv = _err;
      _err = ADS1X1X_OK;
      return rv;
    }
    /** Configures ALERT/RDY pin as a conversion ready pin.
     *  It does this by setting the MSB of the high threshold register to '1' and the MSB 
     *  of the low threshold register to '0'. COMP_POL and COMP_QUE bits will be set to '0'.
     *  Note: ALERT/RDY pin requires a pull up resistor.
     */
    void setConversionReadyPinMode() {
      setComparatorThresholdHigh(1);
      setComparatorThresholdLow(0);
      setComparatorPolarity(0);
      setComparatorQueConvert(0);
      //I2Cdev::writeBitW(_addDev, ADS1115_RA_HI_THRESH, 15, 1);
      //I2Cdev::writeBitW(_addDev, ADS1115_RA_LO_THRESH, 15, 0);
      //setComparatorPolarity(0);
      //setComparatorQueueMode(0);
    }
  private:
    bool writeRegister(uint8_t address, uint8_t reg, uint16_t value) {
      _poTW->beginTransmission(address);
      _poTW->write((uint8_t)reg);
      _poTW->write((uint8_t)(value >> 8));
      _poTW->write((uint8_t)(value & 0xFF));
      return (_poTW->endTransmission() == 0);
    }

    uint16_t readRegister(uint8_t address, uint8_t reg) {
      _poTW->beginTransmission(address);
      _poTW->write(reg);
      _poTW->endTransmission();

      int rv = _poTW->requestFrom(address, (uint8_t) 2);
      if (rv == 2) {
        uint16_t value = _poTW->read() << 8;
        value += _poTW->read();
        return value;
      }
      return 0x0000;
    }


  protected:
    JJ_ADS1X15() {
      setGain(0);      // _gain = ADS1X1X_RANGO_6144;
      setMode(1);      // _mode = ADS1X1X_MODE_SINGLE;
      setDataRate(4);  // middle speed, depends on device.
    }

    // CONFIGURATION
    // BIT  DESCRIPTION
    // 0    # channels        0 == 1    1 == 4;
    // 1    0
    // 2    # resolution      0 == 12   1 == 16
    // 3    0
    // 4    has gain          0 = NO    1 = YES
    // 5    has comparator    0 = NO    1 = YES
    // 6    0
    // 7    0
    uint8_t  _config;
    uint8_t  _maxPorts;
    uint8_t  _address;
    uint8_t  _conversionDelay;
    uint8_t  _bitShift;
    uint16_t _gain;
    uint16_t _mode;
    uint16_t _datarate;

    // COMPARATOR vars
    // TODO merge these into one COMPARATOR MASK?
    //      would speed up code in _requestADC() and save 3 bytes RAM.
    uint8_t  _compMode       = 0;
    uint8_t  _compPol        = 1;
    uint8_t  _compLatch      = 0;
    uint8_t  _compQueConvert = 3;


    int16_t _readADC(uint16_t readmode) {
      _requestADC(readmode);
      if (_mode == ADS1X1X_MODE_SINGLE) {
        while ( isBusy() ) yield();   // wait for conversion; yield for ESP.
      }
      else {
        delay(_conversionDelay);      // TODO needed in continuous mode?
      }
      return getValue();
    }

    void _requestADC(uint16_t readmode) {
      // write to register is needed in continuous mode as other flags can be changed
      uint16_t config = ADS1X1X_OS_START_SINGLE;  // bit 15     force wake up if needed
      config |= readmode;                         // bit 12-14
      config |= _gain;                            // bit 9-11
      config |= _mode;                            // bit 8
      config |= _datarate;                        // bit 5-7
      if (_compMode) {
        config |= ADS1X1X_COMP_MODE_WINDOW;         // bit 4      comparator modi
      }
      else {
        config |= ADS1X1X_COMP_MODE_TRADITIONAL;
      }
      if (_compPol) {
        config |= ADS1X1X_COMP_POL_ACTIV_HIGH;      // bit 3      ALERT active value
      }
      else {
        config |= ADS1X1X_COMP_POL_ACTIV_LOW;
      }
      if (_compLatch) {
        config |= ADS1X1X_COMP_LATCH;
      }
      else {
        config |= ADS1X1X_COMP_NON_LATCH;           // bit 2      ALERT latching
      }
      config |= _compQueConvert;                                  // bit 0..1   ALERT mode
      writeRegister(_address, ADS1X1X_REG_CONFIG, config);
    }

    int8_t  _err = ADS1X1X_OK;
    
    TwoWire * _poTW;
  };

  ///////////////////////////////////////////////////////////////////////////
  //
  // Derived classes from JJ_ADS1X15
  //
  class JJ_ADS1013 : public JJ_ADS1X15 {
    public:
      JJ_ADS1013(uint8_t address = ADS1015_ADDRESS) {
        JJ_ADS1X15::_address = address;
        _config = ADS_CONF_NOCOMP | ADS_CONF_NOGAIN | ADS_CONF_RES_12 | ADS_CONF_CHAN_1;
        _conversionDelay = ADS1015_CONVERSION_DELAY;
        _bitShift = 4;
        _maxPorts = 1;
      }
  };

  class JJ_ADS1014 : public JJ_ADS1X15 {
    public:
      JJ_ADS1014(uint8_t address = ADS1015_ADDRESS) {
        _address = address;
        _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_12 | ADS_CONF_CHAN_1;
        _conversionDelay = ADS1015_CONVERSION_DELAY;
        _bitShift = 4;
        _maxPorts = 1;
      }
  };

  class JJ_ADS1015 : public JJ_ADS1X15 {
    public:
      JJ_ADS1015(uint8_t address = ADS1015_ADDRESS) {
        _address = address;
        _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_12 | ADS_CONF_CHAN_4;
        _conversionDelay = ADS1015_CONVERSION_DELAY;
        _bitShift = 4;
        _maxPorts = 4;
      }

      int16_t readADC_Differential_0_3() {  return _readADC(ADS1X15_MUX_DIFF_0_3);  }
      int16_t readADC_Differential_1_3() {  return _readADC(ADS1X15_MUX_DIFF_1_3);  }
      int16_t readADC_Differential_2_3() {  return _readADC(ADS1X15_MUX_DIFF_2_3);  }
      int16_t readADC_Differential_0_2() {  return readADC(2) - readADC(0); } //not possible in async
      int16_t readADC_Differential_1_2() {  return readADC(2) - readADC(1); } //not possible in async

      void requestADC_Differential_0_3() {  _requestADC(ADS1X15_MUX_DIFF_0_3);  }
      void requestADC_Differential_1_3() {  _requestADC(ADS1X15_MUX_DIFF_1_3);  }
      void requestADC_Differential_2_3() {  _requestADC(ADS1X15_MUX_DIFF_2_3);  }     
  };

  class JJ_ADS1113 : public JJ_ADS1X15 {
    public:
      JJ_ADS1113(uint8_t address = ADS1115_ADDRESS) {
        _address = address;
        _config = ADS_CONF_NOCOMP | ADS_CONF_NOGAIN | ADS_CONF_RES_16 | ADS_CONF_CHAN_1;
        _conversionDelay = ADS1115_CONVERSION_DELAY;
        _bitShift = 0;
        _maxPorts = 1;
      }
  };

  class JJ_ADS1114 : public JJ_ADS1X15 {
    public:
      JJ_ADS1114(uint8_t address = ADS1115_ADDRESS) {
        _address = address;
        _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_16 | ADS_CONF_CHAN_1;
        _conversionDelay = ADS1115_CONVERSION_DELAY;
        _bitShift = 0;
        _maxPorts = 1;
      }
  };

  class JJ_ADS1115 : public JJ_ADS1X15 {
    public:
      JJ_ADS1115(uint8_t address = ADS1115_ADDRESS) {
        _address = address;
        _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_16 | ADS_CONF_CHAN_4;
        _conversionDelay = ADS1115_CONVERSION_DELAY;
        _bitShift = 0;
        _maxPorts = 4;
      }

      int16_t readADC_Differential_0_3() {  return _readADC(ADS1X15_MUX_DIFF_0_3);  }
      int16_t readADC_Differential_1_3() {  return _readADC(ADS1X15_MUX_DIFF_1_3);  }
      int16_t readADC_Differential_2_3() {  return _readADC(ADS1X15_MUX_DIFF_2_3);  }
      int16_t readADC_Differential_0_2() {  return readADC(2) - readADC(0); }
      int16_t readADC_Differential_1_2() {  return readADC(2) - readADC(1); }

      void requestADC_Differential_0_3() {  _requestADC(ADS1X15_MUX_DIFF_0_3);  }
      void requestADC_Differential_1_3() {  _requestADC(ADS1X15_MUX_DIFF_1_3);  }
      void requestADC_Differential_2_3() {  _requestADC(ADS1X15_MUX_DIFF_2_3);  }     
};

// --- END OF FILE ---
