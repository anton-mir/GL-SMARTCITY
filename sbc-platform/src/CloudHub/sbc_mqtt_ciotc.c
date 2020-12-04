/******************************************************************************
 * Copyright 2017 Google
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
// [START iot_mqtt_include]
#define _XOPEN_SOURCE 500
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#include "jwt.h"
#include "openssl/ec.h"
#include "openssl/evp.h"
#include "MQTTClient.h"
#include <jansson.h>


// [END iot_mqtt_include]
#include "sbc_mqtt_ciotc.h"
#include "hubConfigManagerWrapper.h"

#ifndef DEBUG
#define TRACE 1 /* Set to 1 to enable tracing */
#else
#define TRACE 0 /* Set to 1 to enable tracing */
#endif

struct {
  char* address;
  enum { clientid_maxlen = 256, clientid_size };
  char clientid[clientid_size];
  char* deviceid;
  char* keypath;
  char* projectid;
  char* region;
  char* registryid;
  char* rootpath;
  enum { topic_maxlen = 256, topic_size };
  char topic[topic_size];
  char* payload;
  char* algorithm;
} opts = {
  .address = "ssl://mqtt.googleapis.com:8883",
  .clientid = "projects/{your-project-id}/locations/{your-region-id}/registries/{your-registry-id}/devices/{your-device-id}",
  .deviceid = "{your-device-id}",
  .keypath = "ec_private.pem",
  .projectid = "intense-wavelet-343",
  .region = "{your-region-id}",
  .registryid = "{your-registry-id}",
  .rootpath = "roots.pem",
//  .topic = "/devices/{your-device-id}/events",
//  .topic = "projects/sunlit-precinct-233015/topics/sc-telemetry",
  .topic = "projects/sunlit-precinct-233015/topics/sc-lights",
  .payload = "Hello world!",
  .algorithm = "RS256"
//  .algorithm = "ES256"
};

void Usage() {
  printf("mqtt_ciotc <message> \\\n");
  printf("\t--deviceid <your device id>\\\n");
  printf("\t--region <e.g. us-central1>\\\n");
  printf("\t--registryid <your registry id>\\\n");
  printf("\t--projectid <your project id>\\\n");
  printf("\t--keypath <e.g. ./ec_private.pem>\\\n");
  printf("\t--rootpath <e.g. ./roots.pem>\n\n");
}

// [START iot_mqtt_jwt]
/**
 * Calculates issued at / expiration times for JWT and places the time, as a
 * Unix timestamp, in the strings passed to the function. The time_size
 * parameter specifies the length of the string allocated for both iat and exp.
 */
static void GetIatExp(char* iat, char* exp, int time_size) {
  // TODO: Use time.google.com for iat
  time_t now_seconds = time(NULL);
  snprintf(iat, time_size, "%lu", now_seconds);
  snprintf(exp, time_size, "%lu", now_seconds + 3600);
  if (TRACE) {
    printf("IAT: %s\n", iat);
    printf("EXP: %s\n", exp);
  }
}

static int GetAlgorithmFromString(const char *algorithm) {
    if (strcmp(algorithm, "RS256") == 0) {
        return JWT_ALG_RS256;
    }
    if (strcmp(algorithm, "ES256") == 0) {
        return JWT_ALG_ES256;
    }
    return -1;
}

/**
 * Calculates a JSON Web Token (JWT) given the path to a EC private key and
 * Google Cloud project ID. Returns the JWT as a string that the caller must
 * free.
 */
static char* CreateJwt(const char* ec_private_path, const char* project_id, const char *algorithm) {
  char iat_time[sizeof(time_t) * 3 + 2];
  char exp_time[sizeof(time_t) * 3 + 2];
  uint8_t* key = NULL; // Stores the Base64 encoded certificate
  size_t key_len = 0;
  size_t rsl;
  jwt_t *jwt = NULL;
  int ret = 0;
  char *out = NULL;

  // Read private key from file
  FILE *fp = fopen(ec_private_path, "r");
  if (fp == (void*) NULL) {
    printf("Could not open file: %s\n", ec_private_path);
    return "";
  }
  fseek(fp, 0L, SEEK_END);
  key_len = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  key = malloc(sizeof(uint8_t) * (key_len + 1)); // certificate length + \0

  rsl = fread(key, 1, key_len, fp);
  key[key_len] = '\0';
  fclose(fp);
  if (rsl != key_len) {
      printf("Read key failed\n");
      free(key);
      return "";
  }

  // Get JWT parts
  GetIatExp(iat_time, exp_time, sizeof(iat_time));

  jwt_new(&jwt);

  // Write JWT
  ret = jwt_add_grant(jwt, "iat", iat_time);
  if (ret) {
    printf("Error setting issue timestamp: %d\n", ret);
  }
  ret = jwt_add_grant(jwt, "exp", exp_time);
  if (ret) {
    printf("Error setting expiration: %d\n", ret);
  }
  ret = jwt_add_grant(jwt, "aud", project_id);
  if (ret) {
    printf("Error adding audience: %d\n", ret);
  }
  ret = jwt_set_alg(jwt, GetAlgorithmFromString(algorithm), key, key_len);
  if (ret) {
    printf("Error during set alg: %d\n", ret);
  }
  out = jwt_encode_str(jwt);
  if(!out) {
      perror("Error during token creation:");
  }
  // Print JWT
  if (TRACE) {
    printf("JWT: [%s]\n", out);
  }

  jwt_free(jwt);
  free(key);
//    printf("CreateJwt before exit\n");
  return out;
}
// [END iot_mqtt_jwt]

/**
 * Helper to parse arguments passed to app. Returns false if there are missing
 * or invalid arguments; otherwise, returns true indicating the caller should
 * free the calculated client ID placed on the opts structure.
 *
 * TODO: (class) Consider getopt
 */
// [START iot_mqtt_opts]
bool GetOpts(int argc, char** argv) {
  int pos = 1;
  bool calcvalues = false;
  bool calctopic = true;

  if (argc < 2) {
    return false;
  }

  opts.payload = argv[1];

  while (pos < argc) {
    if (strcmp(argv[pos], "--deviceid") == 0) {
      if (++pos < argc) {
        opts.deviceid = argv[pos];
        calcvalues = true;
      }
      else
        return false;
    } else if (strcmp(argv[pos], "--region") == 0) {
      if (++pos < argc) {
        opts.region = argv[pos];
        calcvalues = true;
      }
      else
        return false;
    } else if (strcmp(argv[pos], "--registryid") == 0) {
      if (++pos < argc) {
        opts.registryid = argv[pos];
        calcvalues = true;
      }
      else
        return false;
    } else if (strcmp(argv[pos], "--projectid") == 0) {
      if (++pos < argc) {
        opts.projectid = argv[pos];
        calcvalues = true;
      }
      else
        return false;
    } else if (strcmp(argv[pos], "--keypath") == 0) {
      if (++pos < argc)
        opts.keypath = argv[pos];
      else
        return false;
    } else if (strcmp(argv[pos], "--rootpath") == 0) {
      if (++pos < argc)
        opts.rootpath = argv[pos];
      else
        return false;
    } else if (strcmp(argv[pos], "--topic") == 0) {
      if (++pos < argc) {
        strcpy((char * restrict)&opts.topic,argv[pos]);
        calctopic=false;
      } else
        return false;
    } else if (strcmp(argv[pos], "--algorithm") == 0) {
      if (++pos < argc)
        opts.algorithm = argv[pos];
      else
        return false;
    }
    pos++;
  }
  if (calctopic) {
    int n = snprintf(opts.topic, sizeof(opts.topic),
        "/devices/%s/events",
        opts.deviceid);
    if (n < 0) {
      printf("Encoding error!\n");
      return false;
    }
    if (n > (int)sizeof(opts.topic)) {
      printf("Error, buffer for storing device ID was too small.\n");
      return false;
    }
  }
  if (calcvalues) {
    int n = snprintf(opts.clientid, sizeof(opts.clientid),
        "projects/%s/locations/%s/registries/%s/devices/%s",
        opts.projectid, opts.region, opts.registryid, opts.deviceid);
    if (n < 0 || (n > clientid_maxlen)) {
      if (n < 0) {
        printf("Encoding error!\n");
      } else {
        printf("Error, buffer for storing client ID was too small.\n");
      }
      return false;
    }
    if (TRACE)
    {
      printf("\n\nGot new message from client with ID: %s", opts.clientid);
    }

    return true; // Caller must free opts.clientid
  }
  return false;
}
// [END iot_mqtt_opts]

static const int kQos = 1;
static const unsigned long kTimeout = 10000L;
static const char* kUsername = "unused";

static const unsigned long kInitialConnectIntervalMillis = 500L;
static const unsigned long kMaxConnectIntervalMillis = 6000L;
static const unsigned long kMaxConnectRetryTimeElapsedMillis = 900000L;
static const float kIntervalMultiplier = 1.5f;


// ------------------------------------- //
/*
volatile MQTTClient_deliveryToken deliveredtoken;
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
*/
// ------------------------------------- //

/**
 * Publish a given message, passed in as payload, to Cloud IoT Core using the
 * values passed to the sample, stored in the global opts structure. Returns
 * the result code from the MQTT client.
 */
// [START iot_mqtt_publish]
int Publish(char* payload, int payload_size, unsigned obj_type, unsigned rport, char* rhost, uint64_t sbc_id) {
  int rc = -1;
  MQTTClient client = {0};
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token = {0};
  char sub_topic[2048];

//    printf("trap1\n");
  MQTTClient_create(&client, opts.address, opts.clientid,
      MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts.keepAliveInterval = 60;
  conn_opts.cleansession = 1;
  conn_opts.username = kUsername;
  conn_opts.password = CreateJwt(opts.keypath, opts.projectid, opts.algorithm);
//    printf("trap2\n");

  MQTTClient_SSLOptions sslopts = MQTTClient_SSLOptions_initializer;

  sslopts.trustStore = opts.rootpath;
  sslopts.privateKey = opts.keypath;
  conn_opts.ssl = &sslopts;

  unsigned long retry_interval_ms = kInitialConnectIntervalMillis;
  unsigned long total_retry_time_ms = 0;
//    printf("trap3\n");

  /*
  rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
  if (rc != MQTTCLIENT_SUCCESS) {
      printf("set callback failed\n");
      exit(EXIT_FAILURE);
  }
  */

  while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
  {
//    printf("trap3.0\n");
    switch (rc)
    {
        case 1:
            printf("WARNING! Address: %s ID: %s - connection refused: Unacceptable protocol version. Check config.\n", opts.address, opts.clientid);
            break;
        case 2:
            printf("WARNING! Address: %s ID: %s - connection refused: Identifier rejected\n", opts.address, opts.clientid);
            break;
        case 3:
            printf("WARNING! ddress: %s ID: %s - connection refused: Server unavailable\n", opts.address, opts.clientid);
            usleep(retry_interval_ms / 1000);
            total_retry_time_ms += retry_interval_ms;
            if (total_retry_time_ms >= kMaxConnectRetryTimeElapsedMillis)
            {
                printf("Failed to connect, maximum retry time exceeded.");
                exit(EXIT_FAILURE);
            }
            retry_interval_ms *= kIntervalMultiplier;
            if (retry_interval_ms > kMaxConnectIntervalMillis)
            {
                retry_interval_ms = kMaxConnectIntervalMillis;
            }
            printf("trap3.1\n");
            break;
        case 4:
            printf("WARNING! Address: %s ID: %s - connection refused: Bad user name or password\n", opts.address, opts.clientid);
            break;
        case 5:
            printf("WARNING! Address: %s ID: %s - connection refused: Not authorized\n", opts.address, opts.clientid);
            break;
        default:
            printf("WARNING! Failed to connect\n");
            break;
    }
    exit(EXIT_FAILURE);
  }
  //char sub_topic[] = "projects/sunlit-precinct-233015//devices/{device-id}/commands/#";
  //char sub_topic[] = "projects/sunlit-precinct-233015/devices/2826951441757190/commands/#";
  //char sub_topic[] = "projects/sunlit-precinct-233015/devices/mqtt-007/commands/#";
  //char sub_topic[] = "commands/#";
  //char sub_topic[] = "projects/sunlit-precinct-233015/topics/sc-lights";
  // char sub_topic[] = "/";
  //char sub_topic[] = "#";
  // char *sub_topic = NULL;
  // char sub_topic[] = "projects/sunlit-precinct-233015/topics/sc-lights";
  // char sub_topic[] = "sc-lights";
  // char sub_topic[] = "projects/sunlit-precinct-233015/topics/sc-lights/switch";
  // char sub_topic[] = "projects/sunlit-precinct-233015/topics/sc-telemetry";
  // char sub_topic[] = "/devices/mqtt-007/commands/#";
  snprintf(sub_topic, sizeof(sub_topic), "/devices/%s/commands/#", opts.deviceid);
  sub_topic[sizeof(sub_topic) - 1 ] = 0x0;
  rc = MQTTClient_subscribe(client, sub_topic, 1);
  if (rc != MQTTCLIENT_SUCCESS) {
      printf("subscription to %s failed, retcode=%d\n", sub_topic, rc);
      MQTTClient_disconnect(client, 10000);
      MQTTClient_destroy(&client);
      exit(EXIT_FAILURE);
  }
//    printf("trap4\n");

  pubmsg.payload = payload;
  pubmsg.payloadlen = payload_size;
  pubmsg.qos = kQos;
  pubmsg.retained = 0;

  if (obj_type != MQTT_OBJ_TYPE_LIGTH) {
  MQTTClient_publishMessage(client, opts.topic, &pubmsg, &token);
  if (TRACE)
    printf("Waiting for up to %lu seconds for publication of %s\n"
          "on topic %s for client with ClientID: %s\n",
          (kTimeout/1000), opts.payload, opts.topic, opts.clientid);
  rc = MQTTClient_waitForCompletion(client, token, kTimeout);
  if (TRACE)
    printf("Message with delivery token %d delivered\n", token);
  } else {
      // receive command from cloud to ligths //
      char *my_topic_name;
      int topicLen;
      MQTTClient_message *my_msg;
      printf("Object is lights, start receive external command, sbc_id:%" PRId64 "\n", sbc_id);
      while(1) {
      my_msg = NULL;
      rc = MQTTClient_receive(client, &my_topic_name, &topicLen, &my_msg, kTimeout);
      if (my_msg != NULL) {
          char data[2048];
          char out_pkt[3072];
          char *ptr;
          int len;
          if (sizeof(data) - 1 > (unsigned)my_msg->payloadlen)
              len = my_msg->payloadlen;
          else
              len = sizeof(data) - 1;
          memcpy(data, my_msg->payload, len);
          data[len] = 0x0;
          printf("//---------------------------//\npayloadlen=%d, payload=%s\n"
                 "//---------------------------//\n//---------------------------//\n",
                 my_msg->payloadlen, data);
          // ---------------------------------------- //
          if (len < 2 || data[0] != '{' || data[len-1] != '}') {
              printf("no json data packet detected, pkt=%s\n", data);
              rc = 1;
          }
          else {
              struct hostent *hstnm;
              int len;
              json_t *root;
              json_error_t error;
              const char *tl_id_text;
              int tl_status;

              printf("%s(): data=%s\n", __func__, data);
              root = json_loads(data, 0, &error);
              if(!root) {
                  printf("%s(): failed parse json data: %s\n", __func__, data);
              } else {
                  json_t *tl_id, *lstatus;
                  tl_id = json_object_get(root, "id");
                  lstatus = json_object_get(root, "status");
                  if (!json_is_string(tl_id) || !json_is_integer(lstatus) ) {
                      printf("%s(): invalid packet data, data=%s\n", __func__, data);
                  } else {
                      uint64_t sbc_id_loc;
                      int resv;
                      tl_id_text = json_string_value(tl_id);
                      tl_status = json_integer_value(lstatus);
                      sbc_id_loc = get_sbc_id_by_mqtt_id((char*)tl_id_text, &resv);
                      if (resv != 0) {
                          printf("%s(): failed, tl_id=%s not found\n", __func__, tl_id_text);
                      } else {

              //ptr = data + 1;
              //len = snprintf(out_pkt, sizeof(out_pkt), "{\"sbcid\":%" PRId64 ",%s", sbc_id, ptr);
              len = snprintf(out_pkt, sizeof(out_pkt), "{\"sbcid\":%" PRId64 ",\"status\":%d}", sbc_id_loc, tl_status);
              printf("%s(): out_pkt=%s\n", __func__, out_pkt);

              hstnm = gethostbyname(rhost);
              if (!hstnm) {
                  printf("gethost for %s failed!!!, no send data to lights id:%" PRId64 "\n", rhost, sbc_id);
                  rc = 1;
              } else {
                  // if host present
                  struct sockaddr_in  server;
                  struct sockaddr_in6 server6;
                  int sockfd;
                  int rsl;
                  int family;
                  if (hstnm->h_addrtype == AF_INET) {
                      family = 0;
                      server.sin_family = AF_INET;
                      server.sin_port = htons(rport);
                      memcpy(&server.sin_addr, hstnm->h_addr_list[0], hstnm->h_length);
                      sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                      if (sockfd < 0)
                          printf("sock failed!!!, err: %s\n", strerror(errno));
                  } else if (hstnm->h_addrtype == AF_INET6) {
                      family = 1;
                      server6.sin6_family = AF_INET;
                      server6.sin6_port = htons(rport);
                      memcpy(&server6.sin6_addr, hstnm->h_addr_list[0], hstnm->h_length);
                      sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
                      if (sockfd < 0)
                          printf("sock6 failed!!!, err: %s\n", strerror(errno));
                  } else {
                      printf("unknown host address type: %d\n", hstnm->h_addrtype);
                      sockfd = -1;
                  }
                  if (sockfd >= 0) { // if valid sockfd
                      if (family == 0 )
                          rsl = sendto(sockfd, out_pkt, len, 0, (struct sockaddr *)&server, sizeof(server));
                      else
                          rsl = sendto(sockfd, out_pkt, len, 0, (struct sockaddr *)&server6, sizeof(server6));
                      if (rsl < 0) {
                          printf("send failed, sockfd=%d, host:port=%s:%d, err:%s data:%s\n", sockfd, rhost, rport, strerror(errno), out_pkt);
                      } else
                          rc = 0;
                      close(sockfd);
                  } else // else sockfd >= 0
                      rc = 1;

              } // end gethostbyname(rhost);
          } // end resv != 0
          } // end !json_is_string(tl_id) || !json_is_integer(lstatus)

          } // end !root json
          } // end json pkt into data
          // ---------------------------------------- //
      } else {
          if (TRACE)
              printf("No message from cloud received\n");
          rc = 0;
      } // end have mqtt message
      } // end while(1)
  }

  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);

//    printf("trap5\n");

  return rc;
}
// [END iot_mqtt_publish]

/**
 * Connects MQTT client and transmits payload.
 */
// [START iot_mqtt_run]
int sbc_mqtt_main(int argc, char* argv[], unsigned obj_type, unsigned rport, char* rhost, uint64_t sbc_id) {
    int retval;
  OpenSSL_add_all_algorithms();
  OpenSSL_add_all_digests();
  OpenSSL_add_all_ciphers();

  retval = EXIT_FAILURE;
  if (GetOpts(argc, argv)) {
    retval = Publish(opts.payload, strlen(opts.payload), obj_type, rport, rhost, sbc_id);
  } else {
    Usage();
  }

  EVP_cleanup();
//  sleep (15); // check for timeout/hangup
  return retval;
}
// [END iot_mqtt_run]
