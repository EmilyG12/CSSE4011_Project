#if defined(CONFIG_WIFI)
void wifi_connect(void);
#else
#define wifi_connect()
#endif
