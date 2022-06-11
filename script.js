const imgUrl = "https://i.kym-cdn.com/photos/images/original/000/591/928/94f.png";
const imgHtml = `<img src="${imgUrl}" alt="Test img">`;

const button = document.getElementById("button-test");
const imgDiv = document.getElementById("img-div");

button.onclick = () => {
    imgDiv.innerHTML = imgHtml;
}