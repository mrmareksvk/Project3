function reload_id(url,id) {
    // url = location.pathname; // taken from page SCRIPT
    // console.log(url);
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById(id).innerHTML = this.responseText;
            console.log("AJAX#"+id);
        }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
};
