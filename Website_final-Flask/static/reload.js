function reload(url) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.body.innerHTML = this.responseText;
            console.log("AJAX");
        }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
};
