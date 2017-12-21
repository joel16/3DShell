#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#define TEXTURE_BATTERY_0      33
#define TEXTURE_BATTERY_15     34
#define TEXTURE_BATTERY_28     35
#define TEXTURE_BATTERY_43     36
#define TEXTURE_BATTERY_57     37
#define TEXTURE_BATTERY_71     38
#define TEXTURE_BATTERY_85     39
#define TEXTURE_BATTERY_100    40
#define TEXTURE_BATTERY_CHARGE 41

#define TEXTURE_WIFI_NULL 42
#define TEXTURE_WIFI_0    43
#define TEXTURE_WIFI_1    44
#define TEXTURE_WIFI_2    45
#define TEXTURE_WIFI_3    46

// Battery Status
void drawBatteryStatus(void);


// Clock functions
void digitalTime(void);
char * getDayOfWeek(int type);
char * getMonthOfYear(int type);


// WiFi Status
void drawWifiStatus(void);

// Draws the status bar
void drawStatusBar(void);

#endif