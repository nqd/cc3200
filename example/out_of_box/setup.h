#ifndef __SETUP__
#define __SETUP__

#define APPLICATION_VERSION              "1.1.0"
#define APP_NAME                         "Out of Box"

void DisplayBanner(char * AppName);
void BoardInit(void);
void ConfigureGPIO(void);
void doSetup(void);

#endif //  __SETUP__
