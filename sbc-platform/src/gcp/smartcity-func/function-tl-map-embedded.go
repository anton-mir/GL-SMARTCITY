// Package p contains a Firebase Realtime Database Cloud Function.
package p

import (
    "context"
    "log"
    "math"
    "os"
    "strconv"
	//"encoding/json"


    firebase "firebase.google.com/go"
    geo "github.com/kellydunn/golang-geo"
    "google.golang.org/api/option"
)

// RTDBEvent is the payload of a RTDB event.
// Please refer to the docs for additional information
// regarding Firestore events.
type RTDBEvent struct {
    Data  interface{}   `json:"data"`
    Delta map[string]interface{} `json:"delta"`
}

// Bsm is the payload of a Pub/Sub event
type Bsm struct {
    Delta struct {
	MqttId struct {
	ID      string `json:"id"`
	Timestp int	`json:"timestp"`
	Msec    int	`json:"msec"`
	Name    string	`json:"name"`	
	Skin    string	`json:"skin"`
	Status  string	`json:"status"`
	Type    string	`json:"type"`	
	State   struct	{
		Acceleration float64 `json:"acceleration"`
		Course       int	`json:"course"`
		Gear         string	`json:"gear"`
		Latitude     float64	`json:"latitude"`
		Longitude    float64	`json:"longitude"`
		Rpm          int	`json:"rpm"`
		Speed        float64	`json:"speed"`
			} `json:"state"`
		} `json:"mqtt-001"`
    } `json:"delta"` 
}
// RTDB handles changes to a Firebase RTDB.
func RTDB(ctx context.Context, e RTDBEvent) error {
	log.Println(e.Delta)

    for mqttId,_ := range e.Delta {
		mqtt := e.Delta[mqttId].(map[string]interface{})
		state := mqtt["state"].(map[string]interface{})
		log.Println(state["latitude"], state["longitude"])
        tlSwitcher(state["latitude"].(float64), state["longitude"].(float64))
    }


    return nil
}

// tlSwitcher
func tlSwitcher(lat float64, lng float64) error {
    //Firebase Conntect
    json := []byte(os.Getenv("FIREBASE_CONF"))
    ctx := context.Background()
    conf := &firebase.Config{
        DatabaseURL: os.Getenv("DB_URL"),
    }
    opt := option.WithCredentialsJSON(json)
    app, err := firebase.NewApp(ctx, conf, opt)
    if err != nil {
        log.Fatalf("error initializing app: %v\n", err)
    }
    client, err := app.Database(ctx)
    if err != nil {
        log.Fatalln("Error initializing database client:", err)
    }
    // The app only has access as defined in the Security Rules
    ref := client.NewRef(os.Getenv("TRAFFIC_LIGHTS"))
    tlRef := client.NewRef(os.Getenv("TL_FOCUSED"))

    // end Firebase Connect
    // Read
    var tlList map[string]interface{}
    if err := ref.Get(ctx, &tlList); err != nil {
        log.Fatalln("Error reading from database:", err)
    }
    log.Println(tlList)
    //end Read
    for k, v := range tlList {
        tlCurrent := v.(map[string]interface{})

        //log.Println(k, "is interface", tlCurrent)
        log.Println("Processing:", k)
        if tlCurrent["latitude"] != nil && tlCurrent["longitude"] != nil {
            tlGeo := geo.NewPoint(tlCurrent["latitude"].(float64), tlCurrent["longitude"].(float64))
            p := geo.NewPoint(lat, lng)
            dist := p.GreatCircleDistance(tlGeo)
            result := math.Ceil(dist * 1000)
            log.Println("Distance:", result)

            var maxDist float64
            maxDist, err = strconv.ParseFloat(os.Getenv("MAX_DIST"), 64)
            
            if result > 0 && result < maxDist {
                stateRef := ref.Child(k)
                if err := stateRef.Update(ctx, map[string]interface{}{"state": 1}); err != nil {
                    log.Fatalln("Error updating to database:", err)
                }
                log.Println("Switched:", k)

                if err = tlRef.Update(ctx, map[string]interface{}{"id": tlCurrent["id"].(float64)}); err != nil {
                    log.Println("Error updating to database:", err)
                }
                log.Println("Focused:", k, "Id:", tlCurrent["id"].(float64))

            }
        }
    }

    return nil
}
