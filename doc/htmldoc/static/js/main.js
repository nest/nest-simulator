
// Fire up the plugins
$(document).ready(function(){

    $('.flexslider').flexslider({
        animation: "fade",
	pauseOnHover: true,
	directionNav: false,
        start: function(slider){
            $('body').removeClass('loading');

        }
    });

    $("#gototop").click(function() {
	window.scroll({ top: 0, left: 0, behavior: 'smooth' });
    });

    window.onscroll = function() {
	if (document.body.scrollTop > 350 || document.documentElement.scrollTop > 350)
	    $("#gototop").fadeIn(250);
	else
	    $("#gototop").fadeOut(250);
    };

    $('#intro .language-python').each(function() {
        var lines = this.textContent.split('\n');
	if (lines[0] === '')
            lines.shift();
	var matches;
	var indentation = (matches = /^[\s\t]+/.exec(lines[0])) !== null ? matches[0] : null;
	if (!!indentation) {
            lines = lines.map(function(line) {return line.replace(indentation, '')});
            this.textContent = lines.join('\n').trim();
	}
    });

    $(".accordion").accordion({
	easing: 'easeInOutCubic',
	header: "pre",
	active: false,
	collapsible: true,
	heightStyle: "content"
    });

    $("#pulse").click(function() {
	$(".accordion pre").first().trigger("click");
    });

    $("#pulse").position({
	my: "center center",
	at: "right-50 center+22",
	of: $(".accordion pre").first(),
	collision: "none"
    });

    $(".flexslider li span h3").click(function() {
	var parent = $(this).parent();
	parent.toggleClass("fsd-active fsd-inactive");
	parent.find("h3").toggleClass("fsd-h3-active fsd-h3-inactive");
	parent.find("p").slideToggle();
	
//	$(".flexslider li span").toggleClass("fsd-active fsd-inactive");
//	$(".flexslider li span h3").toggleClass("fsd-h3-active fsd-h3-inactive");
//	$(".flexslider li span p").slideToggle();
    });
    
    hljs.highlightAll();

});

/**
 * Handles toggling the navigation menu for small screens.
 */
( function() {
	var button = document.getElementById( 'topnav' ).getElementsByTagName( 'div' )[0],
	    menu   = document.getElementById( 'topnav' ).getElementsByTagName( 'ul' )[0];

	if ( undefined === button )
		return false;

	// Hide button if menu is missing or empty.
	if ( undefined === menu || ! menu.childNodes.length ) {
		button.style.display = 'none';
		return false;
	}

	button.onclick = function() {
		if ( -1 == menu.className.indexOf( 'srt-menu' ) )
			menu.className = 'srt-menu';

		if ( -1 != button.className.indexOf( 'toggled-on' ) ) {
			button.className = button.className.replace( ' toggled-on', '' );
			menu.className = menu.className.replace( ' toggled-on', '' );
		} else {
			button.className += ' toggled-on';
			menu.className += ' toggled-on';
		}
	};
} )();

