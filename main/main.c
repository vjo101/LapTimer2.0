/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

 #include <stdio.h>
 #include <inttypes.h>
 #include <string.h>
 #include <stdlib.h>
 #include "sdkconfig.h"
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "esp_chip_info.h"
 #include "esp_flash.h"
 #include "nvs_flash.h"
 #include "esp_system.h"
 #include "driver/gpio.h"
 #include "esp_wifi.h"
 #include "../../esp/esp-idf/components/nvs_flash/include/nvs.h"
 #include "esp_http_client.h"
 #include "esp_log.h"
 
 #define ESP_WIFI_SSID "dfrlaptimer"
 #define ESP_WIFI_PASS "timetotimelaps"
 #define RETRIES 10
 
 static const char *TAG = "wifi station";
 
 static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                 int32_t event_id, void* event_data) {
   switch (event_id) {
     case WIFI_EVENT_STA_START:
       esp_wifi_connect();
       break;
     case WIFI_EVENT_STA_DISCONNECTED:
       for (int i = 0; i < RETRIES; i++) {
         esp_wifi_connect();
       }
       break;
     case IP_EVENT_STA_GOT_IP:
       ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
       printf("got ip: " IPSTR, IP2STR(&event->ip_info.ip));
       break;
     default:
       printf("other event happpend");
       break;
   }
 }
 
 bool setupWifi(void) {
     // initialize NVS (idk what this does)
     esp_err_t ret = nvs_flash_init();
     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
       ESP_ERROR_CHECK(nvs_flash_erase());
       ret = nvs_flash_init();
     }
     ESP_ERROR_CHECK(ret);
   ESP_ERROR_CHECK(esp_netif_init());  //  Ensures network stack is initialized
     ESP_ERROR_CHECK(esp_event_loop_create_default());
     
     esp_netif_create_default_wifi_sta(); // Creates default network interface for WiFi STA mode
 
 
 
     // ESP_ERROR_CHECK(esp_event_loop_create_default());
 
     esp_event_handler_instance_t instance_any_id;
     esp_event_handler_instance_t instance_got_ip;
     ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                         ESP_EVENT_ANY_ID,
                                                         &wifi_event_handler,
                                                         NULL,
                                                         &instance_any_id));
     ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                         IP_EVENT_STA_GOT_IP,
                                                         &wifi_event_handler,
                                                         NULL,
                                                         &instance_got_ip));
     
   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   ret = esp_wifi_init(&cfg);
   if (ret != ESP_OK) {
     printf("Wi-Fi init failed: %s\n", esp_err_to_name(ret));
     return false;
   }
 
       wifi_config_t wifi_config = {
         .sta = {
             .ssid = ESP_WIFI_SSID,
             .password = ESP_WIFI_PASS,
             .threshold.authmode = WIFI_AUTH_WPA2_PSK,
             // .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
             // .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
         },
     };
 
     // sets to wifi station mode
     ret = esp_wifi_set_mode(WIFI_MODE_STA);
     if (ret != ESP_OK) {
     printf("Wi-Fi set mode failed: %s\n", esp_err_to_name(ret));
     return false;
     }
     // pass in wifi config for ssid and pass
     ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
     if (ret != ESP_OK) {
     printf("Wi-Fi config failed: %s\n", esp_err_to_name(ret));
     return false;
     } 
     // connect
     ret = esp_wifi_start();
     if (ret == ESP_OK) {
       printf("success");
     } else {
       printf("failed");
     }
 
     return true;
 }
 
 // event handler for the HTTP Events. need these so they can handle any events instead of just crashing
 esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
 {
     switch (evt->event_id)
     {
     case HTTP_EVENT_ON_DATA:
         printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
         break;
 
     default:
         break;
     }
     return ESP_OK;
 }
 
 // Function to send HTTP POST request in a separate FreeRTOS task
 void post_rest_task(void *postData) {
   esp_http_client_config_t config_post = {
       .url = "http://httpbin.org/post",
       .method = HTTP_METHOD_POST,
       .cert_pem = NULL,
       .event_handler = client_event_post_handler
   };
   
   esp_http_client_handle_t client = esp_http_client_init(&config_post);
 

   // create the JSON data
   char jsonBuffer[128];
   sprintf(jsonBuffer, "{\n\t\"type\": \"Lap Time\",\n\t\"data\": {\n\t\t\"lap_duration\": %s\n\t}\n}", (char *) postData);
 
   esp_http_client_set_post_field(client, jsonBuffer, strlen(jsonBuffer));
   esp_http_client_set_header(client, "Content-Type", "application/json");

   esp_err_t err = esp_http_client_perform(client);
   if (err == ESP_OK) {
       ESP_LOGI(TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
   } else {
       ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
   }
 
   esp_http_client_cleanup(client);
   
   // Free the memory allocated for the postData and delete the task
   free(postData);
   vTaskDelete(NULL);
 }
 
 // Wrapper function to create a new task for sending POST request
 void post_rest_function(const char* postData) {
   char *data = malloc(strlen(postData) + 1);
   if (data) {
       strcpy(data, postData);
       xTaskCreate(&post_rest_task, "post_rest_task", 8192, data, 5, NULL);
   } else {
       ESP_LOGE(TAG, "Failed to allocate memory for post data");
   }
 }
 
 
 void app_main(void) {
 
   setupWifi();
 
   for (int i = 13; i < 19; i++) {
     if (gpio_set_direction(i, GPIO_MODE_INPUT) == ESP_OK) {
       gpio_set_pull_mode(i, GPIO_PULLDOWN_ENABLE);
       printf("GPIO %d", i);
     }
   }
   for (int i = 21; i < 23; i++) {
     if (gpio_set_direction(i, GPIO_MODE_INPUT) == ESP_OK) {
       gpio_set_pull_mode(i, GPIO_PULLDOWN_ENABLE);
       printf("GPIO %d", i);
     }
   }
   for (int i = 25; i < 27; i++) {
     if (gpio_set_direction(i, GPIO_MODE_INPUT) == ESP_OK) {
       gpio_set_pull_mode(i, GPIO_PULLDOWN_ENABLE);
       printf("GPIO %d", i);
     }
   }
   for (int i = 32; i < 36; i++) {
     if (gpio_set_direction(i, GPIO_MODE_INPUT) == ESP_OK) {
       gpio_set_pull_mode(i, GPIO_PULLDOWN_ENABLE);
       printf("GPIO %d", i);
     }
   }
 
   int previousLapTime = -1;
   int newLapTime = 0;
   char buffer[100];
 
   while (1) {
     vTaskDelay(1000 / portTICK_PERIOD_MS);
     for (int i = 13; i < 19; i++) {
         if (gpio_get_level(i) == 1) {
           newLapTime = time(NULL);
           if (previousLapTime != -1) {
             sprintf(buffer, "%ld", (long)difftime(newLapTime, previousLapTime));
             printf("%s\n", buffer);
             post_rest_function(buffer);
           }
           previousLapTime = newLapTime;
             printf("GPIO %d was high. ", i);
         }
     }
     for (int i = 21; i < 23; i++) {
         if (gpio_get_level(i) == 1) {
             printf("GPIO %d was high. ", i);
         }
     }
   for (int i = 25; i < 27; i++) {
     if (gpio_get_level(i) == 1) {
             printf("GPIO %d was high. ", i);
         }
   }
   for (int i = 32; i < 36; i++) {
     if (gpio_get_level(i) == 1) {
             printf("GPIO %d was high. ", i);
         }
   }
     printf("\n");
   }
 }
 