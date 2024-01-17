document.addEventListener("DOMContentLoaded", function1());

function function1(){

var preTags = document.querySelectorAll('.pre');

 preTags.forEach(preTags => {
   let text = preTags.textContent;
   if (text.includes('hl_api')) {
     text = text.replace(/\.hl_api_\w*/g, '');
     preTags.textContent = text;
   }
 });
};
