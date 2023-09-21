const headerFile = "/pages/header.html";
const footerFile = "/pages/footer.html";
    
function insertHTML(url, locationId) {
    fetch(url)
        .then(response => response.text())
        .then(html => {
            const location = document.getElementById(locationId);
            const tempContainer = document.createElement('div');
            tempContainer.innerHTML = html;
            location.appendChild(tempContainer);
        })
        .catch(error => console.error('Error:', error));
}

insertHTMLAtLocation(htmlToInsertUrl, 'insertionPoint');
