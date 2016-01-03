var xhrRequest = function(url, method, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  xhr.open(method, url);
  xhr.send();
};


// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }                     
);

function locationSuccess(pos) {
  var url = 'https://api.forecast.io/forecast/6fc1ed7b55df5105d75d9d21840da83f/' + pos.coords.latitude + ',' + pos.coords.longitude;
  
  // send request to weather provider
  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);
      var temperature = Math.round(json.currently.temperature);
      var conditions = json.currently.summary;
      console.log("Temperature: " + temperature);
      console.log("Conditions: " + conditions);
      
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e){
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e){
          console.log('Error sending weather info to Pebble!');
        }
      );            
    }
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    
    // Get initial weather
    getWeather();
  }
);
