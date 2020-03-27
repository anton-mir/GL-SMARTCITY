// Package p contains a Pub/Sub Cloud Function.
package p

import (
	"context"
	"encoding/json"
	"log"
	//"math"
    "os"

	//geo "github.com/kellydunn/golang-geo"
    firebase "firebase.google.com/go"
    "google.golang.org/api/option"


)

// PubSubMessage is the payload of a Pub/Sub event. Please refer to the docs for
// additional information regarding Pub/Sub events.
type PubSubMessage struct {
	Data []byte `json:"data"`
}
// Bsm is the payload of a Pub/Sub event
type Bsm struct {
	ID      string `json:"id"`
	Timestp int	`json:"timestp"`
	Msec    int	`json:"msec"`
	Name    string	`json:"name"`	
	Skin    string	`json:"skin"`
	Status  string	`json:"status"`
	Type    string	`json:"type"`	
	State   struct	{
		Acceleration float64 `json:"acceleration"`
		Course       float64	`json:"course"`
		Gear         string	`json:"gear"`
		Latitude     float64	`json:"latitude"`
		Longitude    float64	`json:"longitude"`
		Rpm          int		`json:"rpm"`
		Speed        float64	`json:"speed"`
	} `json:"state"`
}

// CarsPubSub consumes a Pub/Sub message.
func CarsPubSub(ctx context.Context, m PubSubMessage) error {

    log.Printf( "Input: %s", string(m.Data) )
    
	var bsm []Bsm
    
	err := json.Unmarshal(m.Data, &bsm)
	if err != nil {
		log.Printf("error:%s", err)
	}
	
    log.Printf("Output: %+v", bsm)
    
    for id, v := range bsm {
	    var inInterface map[string]interface{}
	
        inrec, _ := json.Marshal(v)
		json.Unmarshal(inrec, &inInterface)
	
        fireBaseWrite(bsm[id].ID, inInterface)

	}
	return nil
	}

func fireBaseWrite(id string, car map[string]interface{}) {
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
		ref := client.NewRef(os.Getenv("CAR_LOCATIONS"))
		carsRef := ref.Child(id)
		if err := carsRef.Update(ctx, car); err != nil {
		log.Fatalln("Error updating to database:", err)
		}
// Write the new post's data simultaneously in the posts list and the user's post list.
	}

