
tsParticles.load('header',
  
  {
    "particles": {
      "number": {
        "value": 150,
        "density": {
          "enable": true,
          "area": 800
        }
      },
      "color": {
        "value": "#ffffff"
      },
      "shape": {
        "type": "circle",
        "stroke": {
          "width": 0,
          "color": "#000000"
        },
        "polygon": {
          "nb_sides": 5
        }
      },
      "opacity": {
        "value": 0.5,
        "random": false,
        "animation": {
          "enable": false,
          "speed": 1,
          "minimumValue": 0.1,
          "sync": false
        }
      },
      "size": {
        "value": 5,
        "random": true,
        "animation": {
          "enable": false,
          "speed": 40,
          "minimumValue": 0.1,
          "sync": false
        }
      },
      "lineLinked": {
        "enable": true,
        "distance": 150,
        "color": "#ffffff",
        "opacity": 0.4,
        "width": 1
      },
      "move": {
        "enable": true,
        "speed": 2,
        "direction": "none",
        "random": false,
        "straight": false,
        "outMode": "out",
        "attract": {
          "enable": false,
            "rotate": {
		"x": 600,
		"y": 1200
	    }
        }
      }
    },
    "interactivity": {
      "detectsOn": "canvas",
      "events": {
        "onHover": {
          "enable": true,
          "mode": "bubble"
        },
        "onClick": {
          "enable": true,
          "mode": "push"
        },
        "resize": true
      },
      "modes": {
        "grab": {
          "distance": 400,
          "lineLinked": {
            "opacity": 1
          }
        },
        "bubble": {
          "distance": 100,
          "size": 6,
          "duration": 2,
          "opacity": 8,
          "speed": 4
        },
        "repulse": {
          "distance": 200
        },
        "push": {
          "quantity": 1
        },
        "remove": {
          "quantity": 2
        }
      }
    },
    "detectsRetina": true,
    "config_demo": {
      "hide_card": false,
      "background_color": "#b61924",
      "background_image": "",
      "background_position": "50% 50%",
      "background_repeat": "no-repeat",
      "background_size": "cover"
    }
  }

);

