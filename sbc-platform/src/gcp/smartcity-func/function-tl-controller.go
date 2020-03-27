// Package p contains a Firebase Realtime Database Cloud Function.
package p

import (
    "context"
    "log"
    "os"
    //"io/ioutil"
	"fmt"
	b64 "encoding/base64"

	"golang.org/x/oauth2/google"
	cloudiot "google.golang.org/api/cloudiot/v1"
)

// RTDBEvent is the payload of a RTDB event.
// Please refer to the docs for additional information
// regarding Firestore events.
type RTDBEvent struct {
    Data  interface{}   `json:"data"`
    Delta map[string]interface{} `json:"delta"`
}

func check(e error) {
    if e != nil {
        log.Println(e)
    }
}
// RTDB handles changes to a Firebase RTDB.
func RTDB(ctx context.Context, e RTDBEvent) error {
	log.Println(e.Delta)
    
    projectID := os.Getenv("GOOGLE_CLOUD_PROJECT")
	regionID := os.Getenv("GOOGLE_CLOUD_REGION")
	gatewayID := os.Getenv("GOOGLE_CLOUD_GATEWAY")
	deviceID := os.Getenv("GOOGLE_CLOUD_HUB")
    credJSON := os.Getenv("GOOGLE_APPLICATION_CREDENTIALS_JSON")
	credPath := os.Getenv("GOOGLE_APPLICATION_CREDENTIALS")

    f, err0 := os.Create(credPath)
        defer f.Close()
        check(err0)

    n3, err1 := f.WriteString(credJSON)
    check(err1)

    fmt.Printf("wrote %d bytes\n", n3)
    f.Sync()
    
    for tlId,_ := range e.Delta {
		tl := e.Delta[tlId].(map[string]interface{})
		log.Println(tlId,tl["state"])
        log.Println(sendCommand(projectID, regionID, gatewayID, deviceID, fmt.Sprintf(`{"id":"%s","status":%.f}`, tlId, tl["state"].(float64))))
    }
	//log.Println("Sent command to device")


    return nil
}

// sendCommand sends a command to a device listening for commands.
func sendCommand(projectID string, region string, registryID string, deviceID string, sendData string) (*cloudiot.SendCommandToDeviceResponse, error) {
	// Authorize the client using Application Default Credentials.
	// See https://g.co/dv/identity/protocols/application-default-credentials
	ctx := context.Background()
	httpClient, err := google.DefaultClient(ctx, cloudiot.CloudPlatformScope)
	if err != nil {
		return nil, err
	}
	client, err := cloudiot.New(httpClient)
	if err != nil {
		return nil, err
	}

	req := cloudiot.SendCommandToDeviceRequest{
		BinaryData: b64.StdEncoding.EncodeToString([]byte(sendData)),
	}

	name := fmt.Sprintf("projects/%s/locations/%s/registries/%s/devices/%s", projectID, region, registryID, deviceID)

	response, err := client.Projects.Locations.Registries.Devices.SendCommandToDevice(name, &req).Do()
	if err != nil {
		return nil, err
	}

	return response, nil
}

// [END iot_send_command]
