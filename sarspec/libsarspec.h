void LibTest();
int* Connect();
bool Disconnect();
bool ChangeIntegrationTime(int IntegrationTime);
bool SetDeviceGain(int Gain);
double* YData(bool TriggerIN, int TriggerInDelay );
double* XData(double interc, double coef1, double coef2, double coef3);
bool TriggerOutOnOff(int On_Off);
bool LED(bool ON_OFF);
const char* GetDeviceInfo();
bool WriteEEPROMCoeff(double C0, double C1, double C2, double C3);
double* ReadEEPROMCoeff();
bool EEPROMRestore();
bool SetTimeOut(int TimeOut);







