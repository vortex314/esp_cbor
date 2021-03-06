/*
 * Wifi.cpp
 *
 *  Created on: Oct 24, 2015
 *      Author: lieven
 */

#include "Wifi.h"

/*
 * wifi.c
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */
extern "C" {
#include "stdint.h"
//#include "wifi.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
//#include "mqtt_msg.h"
//#include "debug.h"
#include "user_config.h"
#include "config.h"
#include "ets_sys.h"
#include "espmissingincludes.h"
}
#include "Sys.h"

Wifi::Wifi() : Handler("Wifi"){
_connections=0;
_connected=false;
}

Wifi::~Wifi() {

}

void  Wifi::config(const char* ssid,const char* pswd) {
ets_strncpy(_ssid,ssid,sizeof(_ssid));
ets_strncpy(_pswd,pswd,sizeof(_pswd));
_connected=false;
}

static uint8_t wifiStatus = STATION_IDLE;

 bool Wifi::isConnected() const {
	return _connected;
}

bool  Wifi::dispatch(Msg& msg) {
//	INFO("line : %d ",_ptLine);
// INFO("msg : %d:%d",msg.src(),msg.signal());
PT_BEGIN();
INIT : {
	PT_WAIT_UNTIL(msg.is(0,SIG_INIT));
	struct station_config stationConf;

	INFO("WIFI_INIT");
	if ( wifi_set_opmode(STATION_MODE) ){
		; // STATIONAP_MODE was STATION_MODE
		INFO("line : %d",__LINE__);
		if ( wifi_set_phy_mode(PHY_MODE_11B)) {
			os_memset(&stationConf, 0, sizeof(struct station_config));
			ets_strncpy((char*)stationConf.ssid,_ssid,sizeof(stationConf.ssid));
			ets_strncpy((char*)stationConf.password,_pswd,sizeof(stationConf.password));
			stationConf.bssid_set=0;
			INFO("line : %d",__LINE__);
			if ( wifi_station_set_config(&stationConf) ){
				if ( wifi_station_connect() ){
					INFO("line : %d",__LINE__);
					goto DISCONNECTED;//	wifi_station_set_auto_connect(TRUE);
				}
			}
		}
	}
	//	wifi_station_set_auto_connect(FALSE);
	INFO(" WIFI INIT failed , retrying... ");
	goto INIT;
};
DISCONNECTED: {
	while(true) {
		timeout(1000);
		PT_YIELD_UNTIL(timeout());
		struct ip_info ipConfig;
		wifi_get_ip_info(STATION_IF, &ipConfig);
		wifiStatus = wifi_station_get_connect_status();
		if ( wifi_station_get_connect_status()== STATION_NO_AP_FOUND || wifi_station_get_connect_status()==STATION_WRONG_PASSWORD || wifi_station_get_connect_status()==STATION_CONNECT_FAIL)
		{
			INFO(" NOT CONNECTED ");
			wifi_station_connect();
		} else if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0) {
			_connections++;
			union {
				uint32_t addr;
				uint8_t ip[4];
			} v;
			v.addr = ipConfig.ip.addr;
			INFO("  IP Address : %d.%d.%d.%d ",v.ip[0],v.ip[1],v.ip[2],v.ip[3]);
			INFO(" CONNECTED ");
			Msg::publish(this,SIG_CONNECTED);
			_connected=true;
			timeout(2000);
			goto CONNECTED;
		} else {
			INFO(" STATION_IDLE ");
		}
		timeout(500);
	}
};
CONNECTED : {
	while(true) {
		PT_YIELD_UNTIL(timeout());
		struct ip_info ipConfig;
		wifi_get_ip_info(STATION_IF, &ipConfig);
		wifiStatus = wifi_station_get_connect_status();
		if (wifiStatus != STATION_GOT_IP ) {
			Msg::publish(this,SIG_DISCONNECTED);
			timeout(500);
			_connected=false;
			goto DISCONNECTED;
		}
		timeout(2000);
	}

};
PT_END();

}

