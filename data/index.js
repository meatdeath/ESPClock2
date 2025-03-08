getTime();
orientationRequest(null);

setInterval(function() 
{
    getTime();
}, 1000); 

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

function orientationRequest(orientationObj)
{
    var value = -1;
    if (orientationObj != null)
    {
        value = orientationObj.value;
    }
    console.log(value);
    
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
        {
            console.log("Display Matrix Orientation: " + this.responseText);
            document.getElementById("orientation").value = this.responseText;   
        }
    };

    if (value == -1)
    {
        xhttp.open("GET", "orientationRequest", true);
    }
    else
    {
        var param = "?orientation="+value;
        xhttp.open("POST", "orientationRequest"+param, true);
    }
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