// Package p contains a Firebase Realtime Database Cloud Function.
package p

import (
	"context"
	"fmt"
	"log"
	"math"
	"os"
	"strconv"

	"cloud.google.com/go/functions/metadata"
	firebase "firebase.google.com/go"
	geo "github.com/kellydunn/golang-geo"
	"google.golang.org/api/option"
)

// RTDBEvent is the payload of a RTDB event.
// Please refer to the docs for additional information
// regarding Firestore events.
type RTDBEvent struct {
	Data  interface{}            `json:"data"`
	Delta map[string]interface{} `json:"delta"`
}

// TL traffic-lights
type TL struct {
	Name      string
	ID        int
	Address   string
	State     int
	Latitude  float64
	Longitude float64
}

// Bsm BSM imput message
type Bsm struct {
	ID      string
	Timestp int
	Msec    int
	Name    string
	Skin    string
	Status  string
	Type    string
	State   struct {
		Acceleration float64
		Course       int
		Gear         string
		Latitude     float64
		Longitude    float64
		Rpm          int
		Speed        float64
	}
}

func fireBaseTList(lat float64, lng float64) error {
	//Firebase Conntect
	json := []byte(os.Getenv("FIREBASE_CONFIG"))
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
			if maxDist, err = strconv.ParseFloat(os.Getenv("MAX_DIST"), 64); err == nil {
				fmt.Println("Max distance:", maxDist)
			}
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

// RTDB handles changes to a Firebase RTDB.
func RTDB(ctx context.Context, e RTDBEvent) error {
	log.Println("Delta", e.Delta)

	for k, v := range e.Delta {

		switch vv := v.(type) {

		case map[string]interface{}:
			log.Println(k, "is interface", vv)

			for i, u := range vv {
				log.Println(i, u)
			}

			fireBaseTList(vv["lat"].(float64), vv["lng"].(float64))

		case string:
			log.Println(k, "is string", vv)
		case float64:
			log.Println(k, "is float64", vv)
		case map[string]float64:
			log.Println(k, "is map", vv)
		case []interface{}:
			log.Println(k, "is an array:")
			for i, u := range vv {
				log.Println(i, u)
			}

		default:
			log.Println(k, "is of a type I don't know how to handle", vv)
		}
	}

	meta, err := metadata.FromContext(ctx)
	if err != nil {
		return fmt.Errorf("metadata.FromContext: %v", err)
	}
	log.Printf("Meta: %v", meta)

	return nil
}

