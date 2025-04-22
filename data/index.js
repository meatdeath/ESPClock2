let rangeSelect;
let combinedChart;

function onPageLoad()
{
    rangeSelect = document.getElementById('rangeSelect');
    getOffset();
    telemetryFormat("",null);
    getTelemetry();
    matrixRequest(null, null);
    setInterval(function() { getTelemetry(); }, 1000); 
    setInterval(function() { fetchAndDraw(rangeSelect.value); }, 60000);
    fetchAndDraw(rangeSelect.value);
    rangeSelect.addEventListener('change', () => {
        console.log("Show graph: " + rangeSelect.value);
        fetchAndDraw(rangeSelect.value);
    });
}

function onRestart()
{
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "restart", true);
    xhttp.send();
}

function onLangClick(language)
{
    console.log("Set language: " + language);
    var param = "?language="+language;
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            window.location.reload(true);
        }
    };

    xhttp.open("POST", "/"+param, true);
    xhttp.send();
}

function getTelemetry()
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function()
    {
        if (this.readyState == 4 && this.status == 200)
        { 
            var json = this.responseText;
            var jsonObj = JSON.parse(json);

            var time = jsonObj.time;
            var temperature = jsonObj.temperature;
            var pressure = jsonObj.pressure;

            if (time != undefined) {
                console.log("Time: " + time);
                document.getElementById("time-string").value = time;
            }
            if (temperature != undefined) {
                console.log("Temperature: " + temperature);
                var temperature_string = "&deg;";
                if (temperature.includes("C")) {
                    temperature_string = temperature.replace("C","\u00B0C");
                }
                if (temperature.includes("F")) {
                    temperature_string = temperature.replace("F","\u00B0F");
                }

                document.getElementById("temperature-string").value = temperature_string;
            }
            if (pressure != undefined) {
                console.log("Pressure: " + pressure);
                document.getElementById("pressure-string").value = pressure;
            }
        }
    };
    xhttp.open("GET", "readtelemetry", true);
    xhttp.send();
}

function setOffset() 
{
    var hours = document.getElementById("hours-offset").value;
    var minutes = document.getElementById("minutes-offset").value;
    var seconds = hours*3600 + ((hours>=0)?minutes:(-minutes)) *60;
    console.log("Offset: " + hours + "h " + minutes + "min => " + seconds + "s");

    var param = "?seconds="+seconds;

    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            var seconds = this.responseText;
            var hours = (seconds / 3600).toFixed(0);
            var minutes = ((seconds%3600) / 60).toFixed(0);
            if (minutes < 0) minutes = -minutes;    
            console.log("Offset from device: " + hours + "h " + minutes + "min => " + seconds + "s");
            document.getElementById("hours-offset").value = hours;
            document.getElementById("minutes-offset").value = minutes;
        }
    };

    xhttp.open("POST", "timeoffset"+param, true);
    xhttp.send();
}

function telemetryFormat(paramName, obj)
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            var json = this.responseText;
            var jsonObj = JSON.parse(json);
            if (jsonObj.time_format != undefined)
            {
                console.log("time_format response: " + jsonObj.time_format);
                document.getElementById("time-format").value = jsonObj.time_format;
            }
            if (jsonObj.leading_zero != undefined)
            {
                console.log("leading_zero response: " + jsonObj.leading_zero);
                document.getElementById("leading-zero").checked = jsonObj.leading_zero;
            }
            if (jsonObj.show_ntp_time != undefined)
            {
                console.log("show_ntp_time response: " + jsonObj.show_ntp_time);
                document.getElementById("show-ntp-time").checked = jsonObj.show_ntp_time;
            }
            if (jsonObj.temperature_units != undefined) {
                console.log("Temperature units: ", jsonObj.temperature_units);
                if (jsonObj.temperature_units == "F") {
                    document.getElementById("temperature-units-c").checked = false;
                    document.getElementById("temperature-units-f").checked = true;
                } else {
                    document.getElementById("temperature-units-c").checked = true;
                    document.getElementById("temperature-units-f").checked = false;
                }
                fetchAndDraw(rangeSelect.value);
            }
            if (jsonObj.pressure_units != undefined) {
                console.log("Pressure units: ", jsonObj.pressure_units);
                if (jsonObj.pressure_units == "mm") {
                    document.getElementById("pressure-units-hpa").checked = false;
                    document.getElementById("pressure-units-mm").checked = true;
                } else {
                    document.getElementById("pressure-units-hpa").checked = true;
                    document.getElementById("pressure-units-mm").checked = false;
                }
                fetchAndDraw(rangeSelect.value);
            }
        }
    };

    var param = "?" + paramName + "=";
    if (paramName == "time_format")
    {
        var timeFormat = obj.value;
        console.log("Setting timeFormat: " + timeFormat);
        param += timeFormat;
        xhttp.open("POST", "timeformat" + param, true);
        xhttp.send();
    }
    else
    if (paramName == "leading_zero")
    {
        var leadingZero = obj.checked;
        console.log("Setting leadingZero: " + leadingZero);
        param += String(leadingZero);
        xhttp.open("POST", "timeformat" + param, true);
        xhttp.send();
    }
    else
    if (paramName == "show_ntp_time")
    {
        var showNtpTime = obj.checked;
        console.log("Setting showNtpTime: " + showNtpTime);
        param += String(showNtpTime);
        xhttp.open("POST", "timeformat" + param, true);
        xhttp.send();
    }
    else
    if (paramName == "temperature_units")
    {
        var temperature_units = obj.value;
        console.log("Temperature units: " + temperature_units);
        param += String(temperature_units);
        xhttp.open("POST", "timeformat" + param, true);
        xhttp.send();
    }
    else
    if (paramName == "pressure_units")
    {
        var pressure_units = obj.value;
        console.log("Pressure units: " + pressure_units);
        param += String(pressure_units);
        xhttp.open("POST", "timeformat" + param, true);
        xhttp.send();
    }
    else
    {
        xhttp.open("POST", "timeformat?dummy=dummy", true);
        xhttp.send();
    }
}

function getOffset() 
{
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            var seconds = this.responseText;
            var hours = seconds / 3600;
            var minutes = (seconds%3600) / 60;
            if (minutes < 0) minutes = -minutes;    
            console.log("Offset from device: " + hours + "h " + minutes + "min => " + seconds + "s");
            document.getElementById("hours-offset").value = hours;
            document.getElementById("minutes-offset").value = minutes;
        }
    };

    xhttp.open("GET", "timeoffset", true);
    xhttp.send();
}

function matrixRequest(paramName, obj)
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            var json = this.responseText;
            var matrixObj = JSON.parse(json);
            console.log("Display Matrix Orientation: " + matrixObj.orientation);
            document.getElementById("orientation").value = matrixObj.orientation;  
            console.log("Display Matrix Order: " + matrixObj.order);
            document.getElementById("reverse-matrix-order").checked = (matrixObj.order=="reverse");   
        }
    };

    var paramValue;
    switch (paramName)
    {
        case "orientation": paramValue = obj.value; break;
        case "order": paramValue = obj.checked?"reverse":"straight"; break;
        default:
            xhttp.open("GET", "matrix", true);
            xhttp.send();
            return;
    }

    console.log("Set " + paramName + ": " + paramValue);

    xhttp.open("POST", "matrix?"+paramName+"="+paramValue, true);
    xhttp.send();
}

function onLightRangesChange()
{
    var min_intencity = document.getElementById("min-intensivity").value;
    var max_intencity = document.getElementById("max-intensivity").value;
    var light_on_min_intencity = document.getElementById("light-on-min-intensivity").value;
    var light_on_max_intencity = document.getElementById("light-on-max-intensivity").value;

    document.getElementById("min-intensivity").max = max_intencity;
    document.getElementById("max-intensivity").min = min_intencity;
    document.getElementById("light-on-min-intensivity").max = light_on_max_intencity;
    document.getElementById("light-on-max-intensivity").min = light_on_min_intencity;

    document.getElementById("min-intensivity-value").value = min_intencity;
    document.getElementById("max-intensivity-value").value = max_intencity;
    document.getElementById("light-on-min-intensivity-value").value = light_on_min_intencity;
    document.getElementById("light-on-max-intensivity-value").value = light_on_max_intencity;

    console.log("min_intencity: " + min_intencity);
    console.log("max_intencity: " + max_intencity);
    console.log("light_on_min_intencity: " + light_on_min_intencity);
    console.log("light_on_max_intencity: " + light_on_max_intencity);

    var param = "?min_intencity="+min_intencity+
                "&max_intencity="+max_intencity+
                "&light_on_min_intencity="+light_on_min_intencity+
                "&light_on_max_intencity="+light_on_max_intencity;

    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            var json = this.responseText;
            var jsonObj = JSON.parse(json);
            console.log("jsonObj.min_intencity: " + jsonObj.min_intencity);
            console.log("jsonObj.max_intencity: " + jsonObj.max_intencity);
            console.log("jsonObj.light_on_min_intencity: " + jsonObj.light_on_min_intencity);
            console.log("jsonObj.light_on_max_intencity: " + jsonObj.light_on_max_intencity);
        }
    };

    xhttp.open("POST", "brightness"+param, true);
    xhttp.send();
}


function fetchAndDraw(range) {
    console.log(`Loading /data_${range}.json ...`);

    var temperatureUnits = 
        document.getElementsByName("temperature-units")[0].checked?
            document.getElementsByName("temperature-units")[0].value:
            document.getElementsByName("temperature-units")[1].value;

    var pressureUnits = 
        document.getElementsByName("pressure-units")[0].checked?
            document.getElementsByName("pressure-units")[0].value:
            document.getElementsByName("pressure-units")[1].value;

    fetch(`/data_${range}.json`)
        .then(res => res.json())
        .then(data => {
            const labels = data.map(function (r) {
                var datetime = new Date(r.t * 1000);
                return  datetime.toLocaleString('default', { month: 'short' }) + " " + 
                        datetime.getDate() + " " + 
                        datetime.getHours() + "h";
            });
            //const temps = data.map(r => r.temp);
            const temps = data.map(function(r) {return r.temp.toFixed(1);});
            const hums  = data.map(r => r.hum);
            const pres  = data.map(r => r.pres);

            if (combinedChart) combinedChart.destroy();

            const ctx = document.getElementById('chart').getContext('2d');
            combinedChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: labels,
                    datasets: [
                        {
                            label: 'Temperature (°'+temperatureUnits+')',
                            data: temps,
                            yAxisID: 'y1',
                            borderColor: 'red',
                            backgroundColor: 'red',
                            tension: 0.2,
                            fill: false
                        },
                        {
                            label: 'Humidity (%)',
                            data: hums,
                            yAxisID: 'y2',
                            borderColor: 'blue',
                            backgroundColor: 'blue',
                            tension: 0.2,
                            fill: false
                        },
                        {
                            label: 'Pressure ('+pressureUnits+')',
                            data: pres,
                            yAxisID: 'y3',
                            borderColor: 'green',
                            backgroundColor: 'green',
                            tension: 0.2,
                            fill: false
                        }
                    ]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false, // 🔥 нужно, чтобы высота работала как в CSS
                    interaction: {
                        mode: 'index',
                        intersect: false
                    },
                    stacked: false,
                    plugins: {
                        legend: { position: 'top' }
                    },
                    scales: {
                        x: {
                            title: {
                                display: true,
                                text: 'Time'
                            }
                        },
                        y1: {
                            type: 'linear',
                            display: true,
                            position: 'left',
                            min: 10,
                            max: 35,
                            title: {
                                display: true,
                                text: 'Temperature, °' + temperatureUnits
                            }
                        },
                        y2: {
                            type: 'linear',
                            display: true,
                            position: 'left',
                            min: 0,
                            max: 100,
                            title: {
                                display: true,
                                text: 'Humidity, %'
                            }
                        },
                        y3: {
                            type: 'linear',
                            display: true,
                            position: 'right',
                            min: 720,
                            max: 770,
                            title: {
                                display: true,
                                text: 'Pressure, '+pressureUnits
                            },
                            grid: { drawOnChartArea: false }
                        }
                    }
                }
            });
      });
  }