// Generate popup. Using eventlisteners for click events, rather than 'onclick'
// because: https://stackoverflow.com/questions/6348494/addeventlistener-vs-onclick

// Ensuring page content is loaded before executing
document.addEventListener("DOMContentLoaded", function() {
  // All svg elements of interest are tagged with class = 'clickable'and an id like 'neuron1'
  const svgElements = document.querySelectorAll("#mySvg .clickable");
  // Find all the popups (note: the popups are embeded HTML directly in rst
  const popups = document.querySelectorAll(".popuptext");
  // add click event listener and get the 'ids'
  svgElements.forEach(function(svgElement) {
    svgElement.addEventListener("click", function() {
      const svgElementId = svgElement.getAttribute("id");
      const popupId = getMatchingPopupId(svgElementId);

      if (popupId) {
        const popup = document.getElementById(popupId);
        togglePopup(popup);
      }
    });
  });
  // if a click event happens outside the svg area hide popups
  document.addEventListener("click", function(event) {
    if (!event.target.closest("#mySvg")) {
      popups.forEach(function(popup) {
        hidePopup(popup);
      });
    }
  });
  // ensure only one popup is shown at a time
  function togglePopup(popup) {
    if (popup.classList.contains("visible")) {
      hidePopup(popup);
    } else {
      hideAllPopups();
      showPopup(popup);
    }
  }

  function hidePopup(popup) {
    popup.classList.remove("visible");
  }

  function showPopup(popup) {
    popup.classList.add("visible");
  }

  function hideAllPopups() {
    popups.forEach(function(popup) {
      hidePopup(popup);
    });
  }
  // look for matching terms in the popup id and svg element so correct image is used
  // Using neuron, synapse, device
  function getMatchingPopupId(svgElementId) {
    const idWord = svgElementId.replace(/[0-9]+$/, "");
    const matchingPopup = Array.from(popups).find(function(popup) {
      return popup.id === idWord;
    });

    return matchingPopup ? matchingPopup.id : null;
  }
});

