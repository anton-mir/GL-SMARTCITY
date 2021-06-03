#Original page

* [https://github.com/den-vasyliev/smartcity] (https://github.com/den-vasyliev/smartcity)

# Display Client

This client displays upcoming departure times and tracked vehicles
from a Firebase Realtime Database instance.

## Usage

The client has two dependencies:

* [Google Maps JavaScript API](https://developers.google.com/maps/documentation/javascript)
to show the map on which the vehicles are placed.
* [Firebase Realtime Database](https://firebase.google.com/docs/database/) to
get details about the location of vehicles, along with upcoming schedule times.

To authorise access to the Google Maps JavaScript API, add an API key to
`index.html`, using the process outlined in
[Google Maps Javascript API: Get a Key](https://developers.google.com/maps/documentation/javascript/get-api-key).

To configure access to Firebase Realtime Database, see
[Add Firebase to your JavaScript Project](https://firebase.google.com/docs/web/setup). You will need to add
a Firebase API Key to the top of `js/index.js`.

To install library dependencies, use Node Package Manager.

```bash
$ npm install
```

To build project and prepare it to deploy use run:

```bash
$ npm run-script build
```

To serve the front end using gulp:

```bash
$ npm run-script serve
```

To deploy project to firebase:

```bash
$ npm run-script deploy
```