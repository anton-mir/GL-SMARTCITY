const firebaseConfig = {
  apiKey: "",
  authDomain: "",
  databaseURL: "",
  projectId: "",
  storageBucket: "",
  messagingSenderId: "",
  appId: "",
  measurementId: ""
};

const createHTMLMarker = ({ OverlayView = google.maps.OverlayView,  ...args }) => {
  class HTMLMarker extends OverlayView {
    constructor() {
      super();
      this.latlng = args.latlng;
      this.html = args.html;
      this.setMap(args.map);
    }
   
    createDiv() {
      this.div = document.createElement('div');
      this.div.className = args.classList;
      this.div.style.position = 'absolute';
      if (this.html) {
        this.div.innerHTML = this.html;
      }
      google.maps.event.addDomListener(this.div, 'click', event => {
        google.maps.event.trigger(this, 'click');
      });
      if (typeof args.events !== 'undefined') {
        this.div.addEventListener('click', event => args.events.click(event))
      }
    }
  
    appendDivToOverlay() {
      const panes = this.getPanes();
      panes.overlayImage.appendChild(this.div);
    }
  
    positionDiv() {
      const point = this.getProjection().fromLatLngToDivPixel(this.latlng);
      if (point) {
        this.div.style.left = `${point.x}px`;
        this.div.style.top = `${point.y - args.dY}px`;
      }
    }
  
    draw() {
      if (!this.div) {
        this.createDiv();
        this.appendDivToOverlay();
      }
      this.positionDiv();
    }
  
    remove() {
      if (this.div) {
        this.div.parentNode.removeChild(this.div);
        this.div = null;
      }
    }
  
    getPosition() {
      return this.latlng;
    }
  
    getDraggable() {
      return false;
    }
  }

  return new HTMLMarker();
};

class TruckManager {
  constructor(map) {
    this.map = map;
    this.trucksGroup = {};
  }

  add(location, url, id) {
    var style = 'opacity: 0; visibility: collapse;';
    if (document.getElementById('pits-enable').checked)
      style = 'opacity: 1; visibility: visible;';
    // const latLng = new google.maps.LatLng(50.4412606,  30.4999897);
    const latLng = new google.maps.LatLng(location);
    const truck = createHTMLMarker({
      latlng: latLng,
      map: this.map,
      html: '<div class="trucMarcker" style="background-image: url(../images/fireman_truck.jpg); '+style+'"></div>',
      dY: 63,
      // classList: 'truckMarkerWraper',
      classList: '',
    });
    
    truck.addListener('click', () => {
      console.log(id);
      changeIdGauge(id);
      changeCarIdGauge(id);
    });  

    this.trucksGroup[id] = truck;
  }

  update(location, url, id) {
    this.trucksGroup[id].setMap(null);
    this.add(location, url, id);
  }

  clear() {
    const trucksCurent = Object.values(this.trucksGroup);
    trucksCurent.forEach(trucks => {
      this.trucksGroup[trucks].setMap(null);
    });
    this.trucksGroup = {};
  }

}

class RoutManager {
  constructor(map) {
    this.map = map;
    this.routGroup = {};
  }

  add(routObj, id) {
    const routArr = [];
    const singleRout = Object.values(routObj);
    singleRout.forEach(sigle => {
      if(typeof sigle !== 'string'){
        routArr.push(sigle);
      }
    });
    
    const flightPath = new google.maps.Polyline({
      path: routArr,
      geodesic: true,
      strokeColor: singleRout[singleRout.length - 1],
      strokeOpacity: 1.0,
      strokeWeight: 3
    });
    flightPath.setMap(this.map);
    
    this.routGroup[id] = flightPath;
  }

  clear() {
    const routCurent = Object.values(this.routGroup);
    routCurent.forEach(trucks => {
      this.routGroup[trucks].setMap(null);
    });
    this.routGroup = {};
  }

}
class PitManager {
  constructor(map) {
    this.map = map;
  }

  add (location, id) {
    var style = 'opacity: 0; visibility: collapse;';
    if (document.getElementById('pits-enable').checked)
      style = 'opacity: 1; visibility: visible;';
    const latLng = new google.maps.LatLng(location);
    const pit = createHTMLMarker({
      latlng: latLng,
      map: this.map,
      html: '<div class="pitMarcker" style="background-image: url(../images/pit_icon.png); background-position:center; background-size: cover; width: 40px; height: 40px; '+style+'"></div>',
      dY: 63,
      classList: 'marker-wrapper',
    });
  }
}
class ParkingManager {
  constructor(map) {
    this.map = map;
  }

  add (location, id) {
    const parkinglatLng = new google.maps.LatLng(location);
    const parking = createHTMLMarker({
      latlng: parkinglatLng,
      map: this.map,
      html: '<div class="parkingMarcker" style="background-image: url(../images/parking.png);"></div>',
      dY: 65,

    });
  }
}
class TrafficLightManager {
  constructor(map) {
    this.map = map;
    this.focused = 1;
  }

  add(location, state, title, id) {
    var style = 'opacity: 0; visibility: collapse;';
    if (document.getElementById('traffic-lights-enable').checked)
      style = 'opacity: 1; visibility: visible;';
    const latLng = new google.maps.LatLng(location);
    const tl = createHTMLMarker({
      latlng: latLng,
      map: this.map,
      html: `
        <div id=${id} style="${style}" class="traffic-light traffic-light-sm" state="${state}" title="${title}">
          <div class="light red"></div>
          <div class="light yellow"></div>
          <div class="light green"></div>
        </div>`,
      dY: 90,
    });
  }

  focus(id) {
    this.focused = id;
  }
}

class PointManager {
  constructor(map) {
    this.map = map;
    this.points = {};
  }

  add(location, title, id) {
    const pnt = new google.maps.Marker({
      position: location,
      map: this.map,
      title: title,
      optimized: false
    });

    this.points[id] = pnt;
  }

  clear() {
    const pnts = Object.values(this.points);
    pnts.forEach(pnt => {
      this.points[pnt] && this.points[pnt].setMap(null);
    });
    this.points = {};
  }
}

class AirProbeManager {
  constructor(map) {
    this.map = map;
  }

  add (location, id, value) {
    const latLng = new google.maps.LatLng(location);
    const probe = createHTMLMarker({
      latlng: latLng,
      map: this.map,
      html: `<div class="air-probe normal">
              <div class="air-probe-overlay" data-id="`+id+`"></div>
              <div class="air-probe-top">
                <span class="air-probe-value">`+value+`</span>
              </div>
              <div class="air-probe-bottom">
                <div class="air-probe-stick"></div>
                <div class="air-probe-base"></div>
              </div>
            </div>`,
      dY: 63,
      classList: 'marker-wrapper',
      events: {
        click: selectAirProbe
      }
    });
  }
}

function changeTrafficLightFocused(tl) {
  if (tl) {
    const panel = document.getElementsByClassName('traffic-light-panel-container')[0];
  
    panel.innerHTML = `
      <div class="traffic-light-panel">
        <div class="traffic-light-header">
          <div class="traffic-light-name">${tl.address ? tl.address : ''}</div>
        </div>
        <div class="traffic-light-body">
          <div class="traffic-light-box">
            <div class="box-label">Latitude</div>
            <div class="tl-latitude box-text">${tl.latitude ? tl.latitude : ''}</div>
          </div>
          <div class="traffic-light-box">
            <div class="box-label">Longitude</div>
            <div class="tl-longitude box-text">${tl.longitude ? tl.longitude : ''}</div>
          </div>
        </div>                
      </div>`;

    changeTrafficLightColor('main', tl.state, false);
  }
}

function changeTrafficLightColor(id, state, withYellow) {
  const trafficLight = document.getElementById(id);
  const oldState = trafficLight ? trafficLight.getAttribute('state') : 0;

  if (oldState != state) {
    if (withYellow && oldState) {
      trafficLight.setAttribute('state', 0);
      setTimeout(() => trafficLight.setAttribute('state', state), 400);
    } else {
      trafficLight.setAttribute('state', state);
    }
  }
}

function setMapCenter(map, db) {
  db.ref('map').once('value', snap => {
    const val = snap.val();
    val.defaultZoom && map.setZoom(val.defaultZoom);
    val.defaultLatitude && val.defaultLongitude && map.setCenter({ lat: val.defaultLatitude, lng: val.defaultLongitude });
  });
}

function CenterControl(controlDiv, map, db) {
  const controlUI = document.createElement('div');
  controlUI.style.backgroundColor = 'rgb(255, 255, 255)';
  controlUI.style.boxShadow = 'rgba(0, 0, 0, 0.3) 0px 1px 4px -1px';
  controlUI.style.borderRadius = '2px';
  controlUI.style.cursor = 'pointer';
  controlUI.style.width = '40px';
  controlUI.style.height = '40px';
  controlUI.style.margin = '10px';
  controlUI.style.position = 'relative';
  controlDiv.appendChild(controlUI);

  const controlText = document.createElement('div');
  controlText.style.backgroundImage = 'url(/images/location.png)';
  controlText.style.backgroundRepeat = 'no-repeat';
  controlText.style.backgroundPosition = 'center';
  controlText.style.backgroundSize = '25px 25px';
  controlText.style.height = '40px';
  controlUI.appendChild(controlText);

  controlUI.addEventListener('click', () => setMapCenter(map, db));
}

firebase.initializeApp(firebaseConfig);
const db = firebase.database();
const carIdButton = document.getElementById('id-car-folow');
document.getElementById("traffic-lights-enable").checked = false;
document.getElementById("car-enable").checked = false;
document.getElementById("dashboard-enable").checked = false;
document.getElementById("pits-enable").checked = false;
document.getElementById("air-probes-enable").checked = true;
document.getElementById("air-probes-enable").disabled = true;
document.querySelectorAll('.traffic-light-container').forEach(el => {
  el.style.opacity = 0;
  el.style.visibility = 'collapse';
});
document.querySelectorAll('.wrapper-gauge-panel').forEach(el => {
  el.style.opacity = 0;
  el.style.visibility = 'collapse';
});
dropAirProbeInfo();
document.querySelectorAll('.side-bar-label.extended')
  .forEach(el => el.addEventListener('click', event => extendedSidebarLabelClick(event)));
// const carResetButton = document.getElementById('id-car-unfolow');

function initMap() {
  const map = new google.maps.Map(document.getElementById('map'), {
    center: {lat: 50.435998, lng: 30.488985}, 
    zoom: 15.5
  });
  sessionStorage.clear();

  // Add center control to map

  const centerControlDiv = document.createElement('div');
  const centerControl = new CenterControl(centerControlDiv, map, db);

  centerControlDiv.index = 1;
  map.controls[google.maps.ControlPosition.RIGHT_BOTTOM].push(centerControlDiv);

  const header = document.getElementsByClassName('header-label')[0];
  header.addEventListener('click', () => setMapCenter(map, db));
  
  const tlManager = new TrafficLightManager(map);
  const trucksManager = new TruckManager(map);
  const routManager = new RoutManager(map);
  const pitManager = new PitManager(map);
  const parkingManager = new ParkingManager(map);
  const airProbeManager = new AirProbeManager(map);
  // Show trafick drow
  // const trafficLayer = new google.maps.TrafficLayer();
  // trafficLayer.setMap(map);
  function folowCar(){
    if(this.className.indexOf('disabled') === -1){
      if(this.className.indexOf('unfolow') === -1){
        this.className = 'pushed';
        const targetId = this.dataset.target;
        let latitude = '';
        let longitude = '';

        db.ref('car-locations').once('value', snapshot => {
          const val = snapshot.val();
          const trucksLocation = Object.values(val);
      
          trucksLocation.forEach(truck => {
            if( targetId === truck.id){
              latitude = truck.state.latitude;
              longitude = truck.state.longitude;
            }
            sessionStorage.setItem('folowCheckId', targetId);
          });
        });
        map.setCenter(new google.maps.LatLng(latitude, longitude));
        map.setZoom(18);
        // carResetButton.className = ' ';
        setTimeout(() => {
          this.className = 'unfolow';
          this.innerHTML = 'UNFOLLOW ME';
        }, 100);
      } else {
        this.className = 'pushed';
        sessionStorage.setItem('folowCheckId', null);
        setTimeout(() => {
          this.className = ' ';
          this.innerHTML = 'FOLLOW ME';
        }, 100);
      }
    }
  }
  // function unfollowCar(){
  //   carResetButton.className = 'disabled';
  //   sessionStorage.setItem('folowCheckId', null);
  // }
  carIdButton.addEventListener('click', folowCar);
  // carResetButton.addEventListener('click', unfollowCar);

  //------------------------Map DB------------------------------------

  db.ref('map').on('value', snapshot => {
    const val = snapshot.val() || {};
    val.zoom && map.setZoom(val.zoom);
    val.centerLatitude && val.centerLongitude && map.setCenter({ lat: val.centerLatitude, lng: val.centerLongitude });
  });

  //-------------Traffic Lights DB------------------------------------

  db.ref('traffic-lights').once('value', snapshot => {
    const val = snapshot.val();
    const tls = Object.values(val);

    tls.forEach(tl => {    
      tlManager.add(
        {
          lat: tl.latitude,
          lng: tl.longitude
        },
        tl.state,
        tl.address,
        tl.id
      );
    });
  });

  db.ref('tl-focused').on('value', snapshot => {
    const tl = snapshot.val();
    tlManager.focus(tl.id);
    
    db.ref('traffic-lights').orderByChild('id').equalTo(tl.id).limitToFirst(1).once('value', snap => {
      const val = snap.val();
      const keys = Object.keys(val);
      keys.length && changeTrafficLightFocused(val[keys[0]]);
    });
  });
  
  db.ref('traffic-lights').on('child_changed', snapshot => {
    const tl = snapshot.val() || {};

    changeTrafficLightColor(tl.id, tl.state, true);

    if (tl.id === tlManager.focused) changeTrafficLightColor('main', tl.state, true); 
  });
  //-------------Pit DB-----------------------------------------
  db.ref('pit-locations/data').once('value', snapshot => {
    const val = snapshot.val();
    const pitLocation = Object.values(val);
    pitLocation.forEach(pit => {
      pitManager.add(
        {
          lat: pit.lat,
          lng: pit.lng
        },
      );
    });
  });

  //-------------AirProbe DB-----------------------------------------
  db.ref('sensor-data/data').once('value', snapshot => {
    const val = snapshot.val();
    let measurements = Object.values(val);
    
    measurements = measurements.sort(function(a, b) { 
      if (a.messageDateTime > b.messageDateTime) return -1;
      else if (a.messageDateTime < b.messageDateTime) return 1;
      else return 0;
    });

    grouped_measurements = [];
    while (measurements.length > 0) {
      grouped_measurements.push(measurements[0]);
      measurements = measurements.slice(1).filter(x => !geolib.isPointWithinRadius(
        x.deviceLocationGPS,
        measurements[0].deviceLocationGPS,
        100
      ));
    }
    
    grouped_measurements.forEach(data => {
      airProbeManager.add(
        {
          lat: data.deviceLocationGPS.latitude,
          lng: data.deviceLocationGPS.longitude
        },
        data.messageId,
        133
      );
    });
  });

  //---------------PARKING DB------------------------
  db.ref('parking-locations/data').once('value', snapshot => {
    const val = snapshot.val();
    const parkingLocation = Object.values(val);
    parkingLocation.forEach(parking => {
      parkingManager.add(
        {
          lat: parking.lat,
          lng: parking.lng
        },
      );
    });
  });




  //-------------TRUCKS LIST------------------------------------

  db.ref('car-locations').once('value', snapshot => {
    const val = snapshot.val();
    const trucksLocation = Object.values(val);

    trucksManager.clear();

    trucksLocation.forEach(truck => {
      trucksManager.add(
        {
          lat: truck.state.latitude,
          lng: truck.state.longitude
        },
        '/images/truckMarker.png',
        truck.id
      );
    });
  });
    
  db.ref('car-locations').on('child_changed', snapshot => {
    const truck = snapshot.val();
    console.log(truck);
    
    trucksManager.update(
      {
        lat: truck.state.latitude,
        lng: truck.state.longitude,
      },
      '/images/truckMarker.png',
      truck.id
    );

    const folowCheckId = sessionStorage.getItem('folowCheckId');
    // console.log('sessionStorage', folowCheckId);
    // console.log('id', truck.id);
    if(folowCheckId === truck.id) {
      map.setCenter(new google.maps.LatLng(truck.state.latitude, truck.state.longitude));
    }
    
  });


//--------------POLINE ROUTE ---------
  db.ref('routs').once('value', snapshot => {
    const val = snapshot.val();
    const routs = Object.values(val);

    routManager.clear();

    routs.forEach((rout, i) => {
      routManager.add(
        rout,
        i
      );
    });
  });
}

//Gauge panel
google.charts.load('current', {'packages':['gauge']});
google.charts.setOnLoadCallback(drawGaugeSpeed);
google.charts.setOnLoadCallback(drawGaugeRpm);
google.charts.setOnLoadCallback(drawGaugeGear);

let gaugeOptions  = {
  speed: {
    width: 240, height: 240,
    max: 200,
    greenColor: '#ffffff',
    greenFrom: 0,
    greenTo: 120,
    redFrom: 180, redTo: 200,
    yellowFrom:120, yellowTo: 180,
    minorTicks: 10,
    majorTicks: [0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200]
  },
  fps: {
    width: 240, height: 120,
    max: 60,
    greenColor: '#ffffff',
    redFrom: 0, redTo: 30,
    greenFrom:30, greenTo: 60,
    minorTicks: 0,
    majorTicks: [0, 30, 60]
  },
  gear: {
    width: 240, height: 120,
    max: 6,
    greenColor: '#ffffff',
    minorTicks: 0,
    majorTicks: [0, 1, 2, 3, 4, 5, 6]
  },
  rpm: {
    width: 240, height: 240,
    max: 8,
    greenColor: '#ffffff',
    redFrom: 7, redTo: 8,
    greenFrom:0, greenTo: 7,
    minorTicks: 10,
    majorTicks: [0, 1, 2, 3, 4, 5, 6, 7, 8]
  }
};

let gaugeSpeed;
let gaugeRpm;
let gaugeGear;
let gaugeDataSpeed, gaugeDataFps, gaugeDataGear, gaugeDataRpm;

function drawGaugeSpeed() {
  gaugeDataSpeed = new google.visualization.DataTable();
  gaugeDataSpeed.addColumn('number', 'km/h');
  gaugeDataSpeed.addRows(1);
  gaugeDataSpeed.setCell(0, 0, 0);
  gaugeSpeed = new google.visualization.Gauge(document.getElementById('speed_div'));
  gaugeSpeed.draw(gaugeDataSpeed, gaugeOptions.speed);
}

function drawGaugeRpm() {
  gaugeDataRpm = new google.visualization.DataTable();
  gaugeDataRpm.addColumn('number', 'x1000');
  gaugeDataRpm.addRows(1);
  gaugeDataRpm.setCell(0, 0, 0);
  gaugeRpm = new google.visualization.Gauge(document.getElementById('rpm_div'));
  gaugeRpm.draw(gaugeDataRpm, gaugeOptions.rpm);
}

function drawGaugeGear() {
  gaugeDataGear = new google.visualization.DataTable();
  gaugeDataGear.addColumn('number', 'GEAR');
  gaugeDataGear.addRows(1);
  gaugeDataGear.setCell(0, 0, 0);
  gaugeGear = new google.visualization.Gauge(document.getElementById('fps_div'));
  gaugeGear.draw(gaugeDataGear, gaugeOptions.gear);
}

function changeCarIdGauge(carIdValue) {
  const carIdPanel = document.getElementById('id-car-place');
  carIdPanel.innerHTML = `${carIdValue}`;
  carIdButton.dataset.target = carIdValue;
  carIdButton.className = '';
  carIdButton.innerHTML = 'FOLLOW ME';
  sessionStorage.setItem('folowCheckId', null);
}

function changeIdGauge(carID){
  db.ref('car-locations/' + carID).on('value', snapshot => {
    const carLocations = snapshot.val();
    if (carLocations.state.speed) {
      gaugeDataSpeed.setValue(0, 0,  Math.ceil(carLocations.state.speed));
      gaugeSpeed.draw(gaugeDataSpeed, gaugeOptions.speed);
    } else {
      drawGaugeSpeed();
    }
    if (carLocations.state.rpm) {
      gaugeDataRpm.setValue(0, 0,  carLocations.state.rpm/1000);
      gaugeRpm.draw(gaugeDataRpm, gaugeOptions.rpm);
    } else {
      drawGaugeRpm();
    }
    if (carLocations.state.gear) {
      gaugeDataGear.setValue(0, 0,  carLocations.state.gear);
      gaugeGear.draw(gaugeDataGear, gaugeOptions.gear);
    } else {
      drawGaugeGear();
    }
  });
}

// Create route

function calculateAndDisplayRoute(start, end, directionsService, directionsDisplay) {
  console.log(start, end);
  directionsService.route({
    origin: start,
    destination: end,
    travelMode: 'DRIVING'
  }, function(response, status) {
    if (status === 'OK') {
      directionsDisplay.setDirections(response);
    } else {
      window.alert('Directions request failed due to ' + status);
    }
  });
}

function extendedSidebarLabelClick(event) {
  const target = event.target;
  const label = target.classList.contains('extended') ? 
    target : target.closest('.extended');
  label.classList.toggle('active');
}

function selectAirProbe(event) {
  const target = event.target;
  const id = parseInt(target.getAttribute('data-id'));
  db.ref('sensor-data/data').once('value', snapshot => {
    const measurement = Object.values(snapshot.val()).filter(x => x.messageId == id);
    const val = measurement[0];
    db.ref('airc_devices/' + val.id).once('value', data => {
      const device = data.val();
      const type = device.type;
      const working_status = device.working_status;
      const description = device.description;
      const latitude = device.latitude;
      const longitude = device.longitude;
      const altitude = device.altitude;
      const months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];

      const date = new Date(val.messageDateTime * 1000);
      if (typeof val.id !== 'undefined')
        document.getElementById('id-label-val').innerHTML = val.id.toString();
      if (typeof type !== 'undefined')
        document.getElementById('type-label-val').innerHTML = type.toString().replace('_', ' ');
      if (typeof working_status !== 'undefined')
        document.getElementById('status-label-val').innerHTML = working_status == 1 ? 'On' : 'Off';
      if (typeof val.messageId !== 'undefined')
        document.getElementById('message_id-label-val').innerHTML = val.messageId.toString();
      if (typeof date !== 'undefined')
        document.getElementById('date-label-val').innerHTML = date.getDate() + ' ' + months[date.getMonth()] + ' ' + date.getFullYear() + ' ' + date.getHours() + ':' + date.getMinutes() + ':' + date.getSeconds();
      if (typeof description !== 'undefined')
        document.getElementById('description-label-val').innerHTML = description.toString();
      if (typeof latitude !== 'undefined')    
        document.getElementById('latitude-label-val').innerHTML = latitude.toString();
      if (typeof longitude !== 'undefined')
        document.getElementById('longitude-label-val').innerHTML = longitude.toString();
      if (typeof altitude !== 'undefined')
        document.getElementById('altitude-label-val').innerHTML = altitude.toString();
      if (typeof val.measurements.temp !== 'undefined')
        document.getElementById('temp1-label-val').innerHTML = (Math.floor(val.measurements.temp * 100) / 100).toString() + '(°C)';
      if (typeof val.measurements.temp_internal !== 'undefined')
        document.getElementById('temp2-label-val').innerHTML = (Math.floor(val.measurements.temp_internal * 100) / 100).toString() + '(°C)';
      if (typeof val.measurements.humidity !== 'undefined')
        document.getElementById('humidity-label-val').innerHTML = val.measurements.humidity.toString() + '(%)';
      if (typeof val.measurements.pressure !== 'undefined')
        document.getElementById('pressure-label-val').innerHTML = val.measurements.pressure.toString() + '(hPa)';
      if (typeof val.measurements.tvoc !== 'undefined')
        document.getElementById('tvoc-label-val').innerHTML = val.measurements.tvoc.toString() + '(ppb)';
      if (typeof val.measurements.co2 !== 'undefined')
        document.getElementById('co2-label-val').innerHTML = val.measurements.co2.toString() + '(ppm)';
      if (typeof val.measurements.co !== 'undefined')
        document.getElementById('co-label-val').innerHTML = val.measurements.co.toString() + '(ppm)';
      if (typeof val.measurements.co_temp !== 'undefined')
        document.getElementById('co-temp-label-val').innerHTML = val.measurements.co_temp.toString() + '(°C)';
      if (typeof val.measurements.co_hum !== 'undefined')
        document.getElementById('co-hum-label-val').innerHTML = val.measurements.co_hum.toString() + '(%)';
      if (typeof val.measurements.co_err !== 'undefined')
        document.getElementById('co-error-label-val').innerHTML = val.measurements.co_err.toString();
      if (typeof val.measurements.no2 !== 'undefined')
        document.getElementById('no2-label-val').innerHTML = val.measurements.no2.toString() + '(ppm)';
      if (typeof val.measurements.no2_temp !== 'undefined')
        document.getElementById('no2-temp-label-val').innerHTML = val.measurements.no2_temp.toString() + '(°C)';
      if (typeof val.measurements.no2_hum !== 'undefined')
        document.getElementById('no2-hum-label-val').innerHTML = val.measurements.no2_hum.toString() + '(%)';
      if (typeof val.measurements.no2_err !== 'undefined')
        document.getElementById('no2-error-label-val').innerHTML = val.measurements.no2_err.toString();
      if (typeof val.measurements.so2 !== 'undefined')
        document.getElementById('so2-label-val').innerHTML = val.measurements.so2.toString() + '(ppm)';
      if (typeof val.measurements.so2_temp !== 'undefined')
        document.getElementById('so2-temp-label-val').innerHTML = val.measurements.so2_temp.toString() + '(°C)';
      if (typeof val.measurements.so2_hum !== 'undefined')
        document.getElementById('so2-hum-label-val').innerHTML = val.measurements.so2_hum.toString() + '(%)';
      if (typeof val.measurements.so2_err !== 'undefined')
        document.getElementById('so2-error-label-val').innerHTML = val.measurements.so2_err.toString();
      if (typeof val.measurements.o3 !== 'undefined')
        document.getElementById('o3-label-val').innerHTML = val.measurements.o3.toString() + '(ppm)';
      if (typeof val.measurements.o3_temp !== 'undefined')
        document.getElementById('o3-temp-label-val').innerHTML = val.measurements.o3_temp.toString() + '(°C)';
      if (typeof val.measurements.o3_hum !== 'undefined')
        document.getElementById('o3-hum-label-val').innerHTML = val.measurements.o3_hum.toString() + '(%)';
      if (typeof val.measurements.o3_err !== 'undefined')
        document.getElementById('o3-error-label-val').innerHTML = val.measurements.o3_err.toString();
      if (typeof val.measurements.hcho !== 'undefined')
        document.getElementById('hcho-label-val').innerHTML = val.measurements.hcho.toString() + '(ppm)';
      if (typeof val.measurements.pm2_5 !== 'undefined')
        document.getElementById('pm2_5-label-val').innerHTML = val.measurements.pm2_5.toString() + '(μg/m3)';
      if (typeof val.measurements.pm10 !== 'undefined')
        document.getElementById('pm10-label-val').innerHTML = val.measurements.pm10.toString() + '(μg/m3)';
    });
  });
}

function dropAirProbeInfo() {
  document.querySelectorAll('.side-bar-label-val').forEach(el => el.innerHTML = '-');
}

function showTrafficLights(flag) {
  if (flag) {
    document.querySelectorAll('.traffic-light-container').forEach(el => {
      el.style.opacity = 1;
      el.style.visibility = 'visible';
    });
    document.querySelectorAll('.traffic-light').forEach(el => {
      el.style.opacity = 1;
      el.style.visibility = 'visible';
    });
  }
  else {
    document.querySelectorAll('.traffic-light-container').forEach(el => {
      el.style.opacity = 0;
      el.style.visibility = 'collapse';
    });
    document.querySelectorAll('.traffic-light').forEach(el => {
      el.style.opacity = 0;
      el.style.visibility = 'collapse';
    });
  }
}

function showCar(flag) {
  if (flag) {
    document.querySelectorAll('.trucMarcker').forEach(el => {
      el.style.opacity = 1;
      el.style.visibility = 'visible';
    });
  }
  else {
    document.querySelectorAll('.trucMarcker').forEach(el => {
      el.style.opacity = 0;
      el.style.visibility = 'collapse';
    });
  }
}

function showDashboard(flag) {
  if (flag) {
    document.querySelectorAll('.wrapper-gauge-panel').forEach(el => {
      el.style.opacity = 1;
      el.style.visibility = 'visible';
    });
  }
  else {
    document.querySelectorAll('.wrapper-gauge-panel').forEach(el => {
      el.style.opacity = 0;
      el.style.visibility = 'collapse';
    });
  }
}

function showPits(flag) {
  if (flag) {
    document.querySelectorAll('.pitMarcker').forEach(el => {
      el.style.opacity = 1;
      el.style.visibility = 'visible';
    });
  }
  else {
    document.querySelectorAll('.pitMarcker').forEach(el => {
      el.style.opacity = 0;
      el.style.visibility = 'collapse';
    });
  }
}
