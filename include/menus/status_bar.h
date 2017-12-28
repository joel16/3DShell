#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#define TEXTURE_BATTERY_0      35
#define TEXTURE_BATTERY_15     36
#define TEXTURE_BATTERY_28     37
#define TEXTURE_BATTERY_43     38
#define TEXTURE_BATTERY_57     39
#define TEXTURE_BATTERY_71     40
#define TEXTURE_BATTERY_85     41
#define TEXTURE_BATTERY_100    42
#define TEXTURE_BATTERY_CHARGE 43

#define TEXTURE_WIFI_NULL 44
#define TEXTURE_WIFI_0    45
#define TEXTURE_WIFI_1    46
#define TEXTURE_WIFI_2    47
#define TEXTURE_WIFI_3    48

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