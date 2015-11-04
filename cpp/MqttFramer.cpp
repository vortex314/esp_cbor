/*
 * MqttFramer.cpp
 *
 *  Created on: Oct 31, 2015
 *      Author: lieven
 */

#include "MqttFramer.h"

MqttFramer::MqttFramer(Tcp* stream) :
		Handler("MqttFrame") {
	_msg = new MqttMsg(256);
	_msg->reset();
	_stream = stream;
}

MqttFramer::~MqttFramer() {

}
extern Str logLine;
void MqttFramer::send(MqttMsg& msg) {
//	INFO(" MQTT send : %s ",msg.toHex(logLine.clear()));
//	INFO(" MQTT send : %s ",((Bytes)msg).toString(logLine.clear()));
	INFO(" MQTT send : %s ", msg.toString(logLine.clear()));
	if (_stream->hasSpace())
		_stream->write(msg);
}

bool MqttFramer::isConnected() {
	return _stream->isConnected();
}

bool MqttFramer::dispatch(Msg& msg) {
	PT_BEGIN()
	while (true) {
		PT_YIELD_UNTIL(msg.is(_stream, SIG_ALL));

		switch (msg.signal()) {
		case SIG_RXD: {
//			INFO(" data rxd");
			int i = 0;
			while (_stream->hasData()) {
				i++;
				if (_msg->feed(_stream->read())) {
					_msg->parse();
					INFO("MQTT IN : %s", _msg->toString(logLine.clear()));
					Msg msg2(200);
					msg2.create(this, SIG_RXD).add(_msg->type()).add(
							_msg->messageId());
					if (_msg->type() == MQTT_MSG_PUBLISH) {
						msg2.addf("iSS", _msg->qos(), _msg->topic(),
								_msg->message());
					}
					msg2.send();
//					Msg::publish(this, SIG_RXD, _msg->type());
					_msg->reset();
				}
			}
//			INFO("Received %d bytes",i);
			break;
		}
		default: {
			Msg::publish(this, msg.signal());
			break;
		}
		}
	}
	PT_END();
	return false;
}
