
function onPageLoad()
{
    getOffset();
    getTimeFormat();getTime();
    matrixRequest(null, null);
    setInterval(function() { getTime(); }, 1000); 
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

function getTime()
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function()
    {
        if (this.readyState == 4 && this.status == 200)
        {
            document.getElementById("time-string").value = this.responseText;
        }
    };
    xhttp.open("GET", "timeread", true);
    xhttp.send();
}

function setOffset() 
{
    var hours = document.getElementById("hours-offset").value;
    var minutes = document.getElementById("minutes-offset").value;
    var seconds = hours*3600 + ((hours>0)?minutes:(-minutes)) *60;
    console.log("Offset: " + hours + "h " + minutes + "min => " + seconds + "s");

    var param = "?seconds="+seconds;

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

    xhttp.open("POST", "timeoffset"+param, true);
    xhttp.send();
}

function setTimeFormat(paramName, obj)
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
                console.log("time_format respoonse: " + jsonObj.time_format);
                document.getElementById("time-format").value = jsonObj.time_format;
            }
            if (jsonObj.leading_zero != undefined)
            {
                console.log("leading_zero respoonse: " + jsonObj.leading_zero);
                document.getElementById("leading-zero").checked = jsonObj.leading_zero;
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
function getTimeFormat() 
{
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            var json = this.responseText;
            var jsonObj = JSON.parse(json);
            var timeFormat = jsonObj.time_format;
            var leadingZero = jsonObj.leading_zero;
            if (timeFormat != undefined)
            {
                console.log("Time format: " + timeFormat);
                document.getElementById("time-format").value = timeFormat;
            }
            if (leadingZero != undefined)
            {
                console.log("Leading zero: " + leadingZero);
                document.getElementById("leading-zero").value = leadingZero;
            }
        }
    };

    xhttp.open("GET", "timeformat", true);
    xhttp.send();
}

function matrixRequest(paramName, obj)
{;
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