/*
Programa de control de la placa PCC.
25/04/2021
Juan José Plá

Versiones y librerias utilizadas:
Versión IDE Arduino 1.8.13
Gestor de tarjetas: STM32 Cores  (STMicroelectronics) V=1.9.0
    https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json

Librerias:
DAC libreria propia                 ->  JJ_MCP4728

ADC libreria propia                 ->  JJ_ADS1X15

Protocolo sensores de temperatura           ->  OneWire V=2.3.5
    https://www.pjrc.com/teensy/td_libs_OneWire.html
  
Sensores temperatura Miles Burton           ->  DallasTemperature V=3.8.0
    https://github.com/milesburton/Arduino-Temperature-Control-Library

      -------------------------------------------------------------------
                       STM32CubeProgrammer v2.6.0                  
      -------------------------------------------------------------------
ST-LINK SN  : 51FF77066582525744391767
ST-LINK FW  : V2J37S7
File download complete
Time elapsed during download operation: 00:00:01.024
      -------------------------------------------------------------------
                       STM32CubeProgrammer v2.6.0                  
      -------------------------------------------------------------------
ST-LINK SN  : 322D170029135147324D4E00
ST-LINK FW  : V2J29S7
File download complete
Time elapsed during download operation: 00:00:01.061

Total memoria Flash para firmware: 131.072 Bytes.
Total memoria RAM: 20.480 Bytes.
Uso de Flash  Uso RAM (Bytes)   Quedan  Versión
 42.360 (32%) 5.524   (26%)   14.956  V01.0 (25/04/20)  PRUEBAS activada
 42.772 (32%) 5.524   (26%)   14.956  V01.0 (25/04/20)  PRUEBAS activada
 44.760 (34%) 5.680   (27%)   14.800  V02.0 (28/04/20)  PRUEBAS activada
 45.992 (35%) 5.712   (27%)   14.768  V02.1 (01/05/20)  PRUEBAS activada
 45.992 (35%) 5.712   (27%)   14.768  V02.1 (01/05/20)  PRUEBAS activada
 46.916 (35%) 5.760   (28%)   14.720  V02.2 (01/05/20)  PRUEBAS activada
 46.844 (35%) 5.760   (28%)   14.720  V02.2 (01/05/20)  PRUEBAS activada
 47.388 (36%) 5.800   (28%)   14.680  V02.3 (05/05/20)  PRUEBAS activada
 47.644 (36%) 5.796   (28%)   14.684  V02.4 (07/05/20)  PRUEBAS activada
 47.724 (36%) 5.796   (28%)   14.684  V02.5 (08/05/20)  PRUEBAS activada
 47.764 (36%) 5.796   (28%)   14.684  V02.5 (09/05/20)  PRUEBAS activada
 45.240 (34%) 5.796   (28%)   14.684  V02.5 (09/05/20)  sin PRUEBAS
 45.540 (34%) 5.804   (28%)   14.676  V02.6 (05/08/21)  sin PRUEBAS
 
       -------------------------------------------------------------------
                       STM32CubeProgrammer v2.6.0                  
      -------------------------------------------------------------------

ST-LINK SN  : 51FF77066582525744391767
ST-LINK FW  : V2J37S7
Board       : --
Voltage     : 3.18V
SWD freq    : 4000 KHz
Connect mode: Under Reset
Reset mode  : Hardware reset
Device ID   : 0x410
Revision ID : Rev X
Device name : STM32F101/F102/F103 Medium-density
Flash size  : 128 KBytes
Device type : MCU
Device CPU  : Cortex-M3



Memory Programming ...
Opening and parsing file: PCC_V4.ino.bin
  File          : PCC_V4.ino.bin
  Size          : 45844 Bytes
  Address       : 0x08000000 


Erasing memory corresponding to segment 0:
Erasing internal memory sectors [0 44]
Download in Progress:


File download complete
Time elapsed during download operation: 00:00:02.751


*/


#include <JJ_MCP4728.h>
#include <JJ_ADS1x15.h>
#ifndef PCC_V4_h
#define PCC_V4_h
//HardwareSerial Serial1(PA10,PA9);
  // *****************************************************
  // ************ Definiciones de pines: *****************
  // *****************************************************
  //#define PRUEBAS     //muestra el trace por el USB aunque no esté activado
  //#define ADS16B    //cuando se utiliza el ADS1115 (16b) en vez del ADS1015 (12b)

  #define MO_C1OFF1   PA0
  #define MO_C1OFF2   PB8
  #define MO_C1OFF3   PB9
  #define MO_C1OFF4   PA1
  #define MO_C2OFF1   PA2
  #define MO_C2OFF2   PB0
  #define MO_C2OFF3   PB1
  #define MO_C2OFF4   PA3

  #define MA_C1Ti1    PA6
  #define MA_C1Ti2    PA7
  #define MA_C2Ti1    PA5
  #define MA_C2Ti2    PA4

  #define MI2C0_SCL   PB6
  #define MI2C0_SDA   PB7
  #define MI2C1_SCL   PB10
  #define MI2C1_SDA   PB11

  #define MO_C1LDAC   PB5
  #define MO_C2LDAC   PC13

  #define C3_Sinc     PA8

  #define MI_C1RDY    PB15
  #define MI_C2RDY    PB14

  #define M_C1DQ      PB13
  #define M_C2DQ      PB12
  
  // *****************************************************
  // ************ Otras definiciones: ********************
  // *****************************************************
  #define VERSION     "V02.6"
  #define PRTRACE     if (_bTrace)  Serial.println  //impresión por USB depende de _bTrace
  #define TRACE       Serial          //USB
  #define MAESTRO     Serial1         //comunicación por la UART1 conectado al Maestro.
  #define TIMEOUT     2000          //Time Out de 2s.
  #define L_TX      128           //longitud del texto.
  #define TEM_AVI     600           //temperatura a partir de la cual genera un aviso en décimas de grado.
  #define TEM_ALA     800           //temperatura a partir de la cual genera una alarma en décimas de grado.
  #define TEM_N_S     4           //número total de sensores de temperatura.
  #define TEM_N_SxP   2           //número de sensores de temperatura por placa.
  #define VOL_ERR     4090          //voltaje máximo que genera error en ADC.
  #define MX_VA_D     4095          //valor máximo que no puede superar el DAC.
  
  //Para mayor claridad:
  #define DAC_R_VDD   JJ_MCP4728::VREF::VDD //referencia Vdd.
  #define DAC_R_2V8   JJ_MCP4728::VREF::INTERNAL_2_8V //referencia interna del DAC.
  #define DAC_O_ST    JJ_MCP4728::PWR_DOWN::NORMAL  //resistencia entre la salida y masa cuando está en OFF.
  #define DAC_O_1K    JJ_MCP4728::PWR_DOWN::GND_1KOHM //resistencia entre la salida y masa cuando está en OFF.
  #define DAC_G_X1    JJ_MCP4728::GAIN::X1  //ganancia de la salida respecto a la diferencia.
  #define DAC_G_X2    JJ_MCP4728::GAIN::X2  //ganancia de la salida respecto a la diferencia.

  // *****************************************************
  // ************ Definiciones variables globales: *******
  // *****************************************************
  volatile bool       _bC0RDY;      //indica que ha terminado la conversión AD del ADC I2C 0
  volatile bool       _bC1RDY;      //indica que ha terminado la conversión AD del ADC I2C 1
  uint32_t          _uTieUlt;
  uint32_t          _uTiPxTe;
  uint32_t          _uTiPxIn;
  uint32_t          _uCiclos;
  int16_t           _iIntMx;      //intensidad máxima.
  bool            _bDAC_OK[2];
  bool            _bADC_OK[2];
  bool            _bSisOK;
  int16_t           _iVMin[2][4];   //valor mínimo ADC, [chip_ADC][canal_ADC]   
  int16_t           _iVMax[2][4];   //valor máximo ADC, [chip_ADC][canal_ADC]
  int32_t           _iVPro[2][4];   //valor promedio ADC, [chip_ADC][canal_ADC]
  uint16_t          _uNMed[2][4];   //número de medidas ADC, [chip_ADC][canal_ADC]
  uint8_t           _uCaAc[2];      //canal actual del ADC, [chip_ADC]
  uint32_t          _uTiUs[2];      //tiempo de ocupación del ADC, [chip_ADC]
  uint32_t          _uTiIni[2];     //tiempo inicial ADC, [chip_ADC]
  volatile uint32_t     _vuAcRDY[2];    //número de activaciones del PIN RDY del ADC, [chip_ADC]
  uint32_t          _uConBuc;
  uint8_t           _uTemNS;      //número total de sensores de temperatura.
  uint8_t           _uTemNSxP;      //número de sensores de temperatura por placa.
  
  //Este es el orden en el que estan puestos los canales, Diodos OFF, ADC y DAC:
  uint8_t           _uIdCan[] =
    { MO_C1OFF1, MO_C1OFF2, MO_C1OFF3, MO_C1OFF4, MO_C2OFF1, MO_C2OFF2, MO_C2OFF3, MO_C2OFF4 };
  uint8_t           _uCanADC[] = { 0, 1, 2, 3, 3, 2, 1, 0 };
  
  uint16_t          _uVaCaDa[8];    //donde almacena los valores del DAC
  uint32_t          _uVaCaTi[8];    //tiempo que el canal permanecera encendido en segundos.

  bool            _bCanON[8];     //indica si el canal está encendido.
  uint32_t          _uValTie[8];    //tiempo en que finaliza el encendido del canal

  char            _cTipTe;
  uint8_t           _uPara_x;
  uint8_t           _uPara_y;
  char            _tSal[L_TX+L_TX];
  char            _tEnt[L_TX];
  char            _tInt[L_TX];
  char            _tErr[L_TX];    //variable donde se almacena el último error.
  bool            _bTrabajo;
  bool            _bError;      //verdadero si ha ocurrido un error
  uint8_t           _uError;      //valor numérico del error.
  bool            _bTrace;

  // *****************************************************
  // ************ Cargar clases: *************************
  // *****************************************************
  // ************ Funciones principales: *****************
  // *****************************************************
  //MCP4728 DAC I2C de 4 canales 12b con referencia interna:// *****************************************************
  JJ_MCP4728          _oDAC_0;      //Canal I2C 0 MI2C0_SCL=PB6; MI2C0_SDA=PB7
  JJ_MCP4728          _oDAC_1;      //Canal I2C 1 MI2C0_SCL=PB10; MI2C0_SDA=PB11
  #ifdef ADS16B
    JJ_ADS1115        _oADC_0(0x48);    //= 0x48 = GND
    JJ_ADS1115        _oADC_1(0x49);    //= 0x49 = VDD
  #else
    JJ_ADS1015        _oADC_0(0x48);    //= 0x48 = GND
    JJ_ADS1015        _oADC_1(0x49);    //= 0x49 = VDD
  #endif
  TwoWire           _owI2C_1(PB11,PB10);

  // *****************************************************
  // ************ Sensores de temperatura: ***************
  // *****************************************************
  OneWire                 _oowBus0(M_C1DQ); //comunicaciones sensor de temperatura.
  OneWire                 _oowBus1(M_C2DQ); //comunicaciones sensor de temperatura.
  DallasTemperature         _dtPCP0(&_oowBus0); //control sensor de temperatura.
  DallasTemperature         _dtPCP1(&_oowBus1); //control sensor de temperatura.
  DeviceAddress       _daPCP0[2];     //direcciones sondas PCP0
  DeviceAddress       _daPCP1[2];     //direcciones sondas PCP1
  uint32_t          _uNuCoTe;     //número de comprobaciones de temperatura.
  uint8_t           _uNumTer[2];    //número de termometros
  int16_t           _iTemAvi;     //temperatura a partir de la cual genera un aviso.
  int16_t           _iTemAla;     //temperatura a partir de la cual genera una alarma.

  // *****************************************************
  // ************ Funciones cortas (interrupciones): *****
  // *****************************************************
  void INT_C0RDY() {  _bC0RDY = true; _vuAcRDY[0]++;  }
  void INT_C1RDY() {  _bC1RDY = true; _vuAcRDY[1]++;  }
  
  // *****************************************************
  // ************ Funciones PCC_V4: **********************
  // *****************************************************
  void setup();
  void loop();
  void Ayuda();
  void Canales_ON_OFF(uint8_t);
  void ConfiguraSistema();
  bool EjecutaComando(char*);
  bool EntradaMAESTRO();
  bool EntradaTRACE();
  void ImprimeDireccion(DeviceAddress, char*);
  bool InicializaDispositivos();
  void ParametrosAlarma(char*, char*);
  void ParametrosDAC(char*, char*);
  void ParametrosTiempo(char*, char*);
  void ParametrosOtros(char*, char*);

  // *****************************************************
  // ************ Funciones de trabajo *******************
  // *****************************************************
  bool Activar(bool);
  bool AlarmaTemperatura(uint8_t, uint8_t, int16_t, uint32_t);
  bool CompruebaTemperatura(uint32_t, char*);
  bool CompruebaTiempos(uint32_t);
  void ConfiguraDAC(uint8_t, JJ_MCP4728*);
  bool ErrorADC(uint8_t, int16_t, int16_t, uint32_t);
  void EscanearI2C();
  bool LeeCanales(uint8_t);
  void LimpiaADC();
  void MuestraADC(uint32_t, char*);
  int MuestraEscanerI2C(byte, byte, uint8_t);
  void MuestraEstadoActual();
  uint8_t PeticionTemperatura();
  bool PreparaTemperatura();
  bool Trabajando();
  void VerSituacionDAC(uint8_t, JJ_MCP4728*);
  #ifdef ADS16B
    bool ConfiguraADC(uint8_t, JJ_ADS1115*);
    void VerSituacionADC(uint8_t, JJ_ADS1115*);
  #else
    bool ConfiguraADC(uint8_t, JJ_ADS1015*);
    void VerSituacionADC(uint8_t, JJ_ADS1015*);
  #endif

  // *****************************************************
  // ************ Funciones PCC_Co: **********************
  // *****************************************************
  char * AcotaTexto(char, char, char*);
  size_t DaLonFSH(const __FlashStringHelper*);
  void Imprime(const __FlashStringHelper*, bool = true);
  void Imprime(char*, bool = true);
  bool LeeNumero(char[], int16_t*, int16_t);
  bool LeeNumero(char[], int32_t*, int16_t);
  bool LeeNumero(char[], uint16_t*, int16_t);
  bool LeeNumero(char[], uint32_t*, int16_t);
  int16_t PosTexto(int16_t, char*); 
  int16_t Valor(bool*, int16_t, int16_t, int16_t, int16_t, char*);

  // *****************************************************
  // ************ Funciones PCC_Test: ********************
  // *****************************************************
  #ifdef PRUEBAS
    void ActivaTest();
    bool CompruebaTemperaturaTest();
    void PreparaPrueba_B(char, char);
    void PreparaPrueba_C(char, char);
    void PreparaPrueba_E();
    void Prueba_A();
    void Prueba_B();
    void Prueba_D();
  #endif
#endif
