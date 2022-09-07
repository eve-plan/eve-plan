#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>

#include <pthread.h>

// wifi+密码   2.4G频段的
#define wifi_ssid "ormissia-tp"
#define wifi_password "1234qweASD"

#define mqtt_server_host "192.168.13.52" // 设置的mqtt服务端地址（我这与HA的IP一样）
#define mqtt_server_port 11883
// mqtt 用户名和密码
#define mqtt_user ""
#define mqtt_password ""

#define topic_cpu_usage "iotlink/workgroup/ormissia-win/windows-monitor/stats/cpu/usage"
#define topic_memory_usage "iotlink/workgroup/ormissia-win/windows-monitor/stats/memory/usage"

#define gpio_cpu_usage D9
#define gpio_memory_usage D10

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void reconnect_mqtt();

void setup_wifi();

void mqtt_callback(char *topic, byte *payload, unsigned int length);

void setup() {
    Serial.begin(115200);
    Serial.printf("setup gpio: %d\n", gpio_cpu_usage);
    pinMode(gpio_cpu_usage, OUTPUT);
    Serial.printf("setup gpio: %d\n", gpio_cpu_usage);
    pinMode(gpio_memory_usage, OUTPUT);
    Serial.printf("setup gpio: %d\n", gpio_cpu_usage);
    setup_wifi();
    mqttClient.setServer(mqtt_server_host, mqtt_server_port);
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect_mqtt();
    }
    mqttClient.loop();
}

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (mqttClient.connect("claptrap_cpu_memory", mqtt_user, mqtt_password)) {
            Serial.println("mqtt connected");

            // 订阅CPU占用率topic || 订阅内存占用率topic
            if (mqttClient.subscribe(topic_cpu_usage, 1) && mqttClient.subscribe(topic_memory_usage, 1)) {
                Serial.println("topic cpu_usage subscribe");
                Serial.println("topic memory_usage subscribe");

                mqttClient.setCallback(&mqtt_callback);
            }
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println("try again in 1 seconds");
            delay(1000);
        }
    }
}

/*
占用率百分比（0-100）转换为代表电压的值（0-255）代表0-3.3V
选用电压表量程为3V，故电压的值只取0-255*(3/3.3)即0-232部分
设函数输入值为x，输出值为y
得公式：x/100=y/[255*(3/3.3)]
解得：y=255x/110=51x/22
*/
int percentageToVoltage(int percentage) {
//  return (int)percentage * 51/22;
// 按照原算法，100%时XIAO板子只能达到2.98V。故将倍数放大一点
    return (int) percentage * 51 / 22;
}

// Callback function mqtt收到消息后的回调
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.printf("received message topic: %s\n", topic);
    // In order to republish this payload, a copy must be made
    // as the original payload buffer will be overwritten whilst
    // constructing the PUBLISH packet.

    // Allocate the correct amount of memory for the payload copy
    char *percentageStr = new char[length];
    // Copy the payload to the new buffer
    memcpy(percentageStr, payload, length);
    Serial.printf("msg: %s\n", percentageStr);

    // 占用率转成字符串再转成数字
    String payloadStr(percentageStr);
    int usage = payloadStr.toInt();

    Serial.printf("msg toInt: %d ", usage);

    // 判断消息来自哪个topic
    String topicStr(topic);
    if (strstr(topicStr.c_str(), topic_cpu_usage) != nullptr) {
        Serial.println("cpu topic received msg");
        Serial.printf("setup gpio: %d\n", gpio_cpu_usage);
        analogWrite(gpio_cpu_usage, percentageToVoltage(usage));
    } else if (strstr(topicStr.c_str(), topic_memory_usage) != nullptr) {
        Serial.println("memory topic received msg");
        Serial.printf("setup gpio: %d\n", gpio_memory_usage);
        analogWrite(gpio_memory_usage, percentageToVoltage(usage));
    } else {
        return;
    }
    // Free the memory
    free(percentageStr);
}

