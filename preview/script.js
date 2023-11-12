
document.getElementById("getForm").addEventListener("submit", function(event) {
    event.preventDefault();
    fetch("/post-bin/" + document.getElementById("get_filename").value)
        .then(response => response.text())
        .then(data => {
            document.getElementById("postResponse").style.display = "none";
            document.getElementById("deleteResponse").style.display = "none";
            document.getElementById("getResponse").style.display = "block";
            document.getElementById("getResponse").innerHTML = data;
            document.getElementById("getResponse").textContent = data;
        });
});

document.getElementById("postForm").addEventListener("submit", function(event) {
    event.preventDefault();
    
    const data = new FormData(document.getElementById("postForm"));
    const customData = {};
    data.forEach((value, key) => {
        customData[key] = value;
    });
    const jsonData = JSON.stringify(customData);
    
    fetch("/post-bin/" + document.getElementById("post_filename").value, {
        method: "POST",
        body: jsonData,
        headers: {
            "Content-Type": "text/plain"
        }
    })
    .then(response => response.text())
    .then(data => {
        document.getElementById("deleteResponse").style.display = "none";
        document.getElementById("getResponse").style.display = "none";
        document.getElementById("postResponse").style.display = "block";
        document.getElementById("postResponse").innerHTML = data;
        document.getElementById("directoryListing").src = "/post-bin?" + + new Date().getTime();
    });
});

document.getElementById("deleteForm").addEventListener("submit", function(event) {
    event.preventDefault();
    fetch("/post-bin/" + document.getElementById("delete_filename").value, {
        method: "DELETE"
    })
        .then(response => response.text())
        .then(data => {
            document.getElementById("postResponse").style.display = "none";
            document.getElementById("getResponse").style.display = "none";
            document.getElementById("deleteResponse").style.display = "block";
            document.getElementById("deleteResponse").innerHTML = data;
            document.getElementById("directoryListing").src = "/post-bin?" + + new Date().getTime();
        });
});

const iframe = document.getElementById("directoryListing");
iframe.addEventListener("load", function() {
    if (iframe.contentWindow) {
        iframe.contentWindow.document.addEventListener("click", function(event) {
            if (event.target.tagName === "A") {
                event.preventDefault();
                const originalURL = event.target.getAttribute("href");
                iframe.src = originalURL;
            }
        });
    }
});
