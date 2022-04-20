/*
 * jquery.sldr
 *
 * A content slider featuring responsive slides, flexible slide widths, callbacks,
 * and custom animation hooks.
 *
 * Version: 1.1
 * Minimum requirements: Developed with jQuery 1.10.2, May work with older versions.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Devon Hirth
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

( function( $ ) {

var $win = $( window );

$.sldr = function( el , options ) {

	var base    = this;
	base.$el    = $( el );
	base.el     = el;
	base.$elwrp = $( el ).children();
	base.$el.data( "sldr" , base );

	/**
	 * Plugin Vars
	 */
	base.callback       = new Array();
	base.sldrSlides     = new Array();
	base.sldrLoadSlides = new Array();
	base.$sliderTimers  = new Array();
	base.$resizeTimers  = new Array();
	base.$delayTimers   = new Array();

	base.wrp            = base.$el.children();
	base.elmnts         = base.wrp.children();
	base.elmntsHTML     = base.wrp.html();

	base.benchcoordinate      = {};
	base.prevcoordinate       = {};
	base.cursorcoordinate     = {};
	base.thiscoordinate       = {};
	base.coordinatedifference = 0;
	base.dragdifference       = 0;
	base.trackmouse           = false;
	base.pagescrollY          = 0;

	base.transformposition = 0;

	/**
	 * Initializing function
	 * @return void
	 */
	base.init = function() {

		base.config = $.extend( {} , $.sldr.defaultOptions , options );

		base.browser();

		var sldr       = base.$el;
		var wrp        = base.$elwrp;
		var elmnts     = wrp.children();
		var elmntsHTML = wrp.html();
		var postLoad   = false;

		/**
		 * Build Slide Array
		 * @type {Number}
		 */
		if ( base.sldrSlides == '' ) {

			base.callback = {
				'sldr'              : base.$el,
				'prevFocalIndex'    : '',
				'prevSlideNum'      : '',
				'currentFocalIndex' : '',
				'currentClass'      : '',
				'currentID'         : '',
				'currentFocalPoint' : '',
				'currentSlideNum'   : '',
				'shiftWidth'        : '',
				'nextFocalIndex'    : '',
				'nextSlideNum'      : ''
			};

			for ( var i = 1; i < elmnts.length + 1; i++ ) {

				var slide     = elmnts.eq( i - 1 );
				var slideLoad = slide.find( '.sldr-load' )
				base.sldrSlides.push({
					'sld'        : slide,
					'slideNum'   : i,
					'id'         : slide.attr( 'id' ),
					'class_name' : slide.attr( 'class' ).split(' ')[0],
					'html'       : slide.html()
				});

				if ( slideLoad.hasClass( 'sldr-load' ) ) {
					postLoad = true;
					base.sldrLoadSlides.push({
						'slideNum'   : i,
						'id'         : slide.attr( 'id' ),
						'class_name' : slide.attr( 'class' ).split(' ')[0],
						'html'       : slide.html()
					});
				}
			}
		}

		/**
		 * Do not finish initiating plugin if there is only one slide.
		 */
		if ( elmnts.length <= 1 ) {
			elmnts.eq( 0 ).addClass( base.config.focalClass );
			base.sliderInit( { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );
			base.sliderLoaded( { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );
		}

		/**
		 * Fill Gaps (if any)
		 */
		if ( elmnts.length > 1 ) base.fillGaps( elmntsHTML );

		/**
		 * sliderInit Callback
		 */
		base.sliderInit( { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );

		/**
		 * Center Slides
		 */
		base.focalChange( 1 );
		base.resizeElements();

		/**
		 * Activate Selectors
		 */
		if ( base.config.selectors != '' ) {
			base.config.selectors.eq( 0 ).addClass( base.config.focalClass );
			base.config.selectors.click( function(e) {
				var th = $( this );
				var change  = base.focalChange( th.index() + 1 , 'selectors' );
				base.animate( change );
				th.siblings().removeClass( base.config.focalClass );
				th.addClass( base.config.focalClass );
				e.preventDefault();
			});

			if ( base.config.sldrAuto ) {
				base.config.selectors.bind( 'mouseenter' , function() {
					base.sliderPause();
				});
				base.config.selectors.bind( 'mouseleave' , function() {
					base.sliderTimer();
				});
			}
		}

		if ( base.config.nextSlide != '' ) {
			base.config.nextSlide.click( function(e) {
				if ( base.config.selectors != '' ) {
					base.config.selectors.removeClass( base.config.focalClass );
					base.config.selectors.eq( base.callback.nextSlideNum - 1 ).addClass( base.config.focalClass );
				}
				var change  = base.focalChange( base.callback.nextSlideNum , 'next' );
				base.animate( change );
				e.preventDefault();
			});

			if ( base.config.sldrAuto ) {
				base.config.nextSlide.bind( 'mouseenter' , function() {
					base.sliderPause();
				});
				base.config.nextSlide.bind( 'mouseleave' , function() {
					base.sliderTimer();
				});
			}
		}

		if ( base.config.previousSlide != '' ) {
			base.config.previousSlide.click( function(e) {
				if ( base.config.selectors != '' ) {
					base.config.selectors.removeClass( base.config.focalClass );
					base.config.selectors.eq( base.callback.prevSlideNum - 1 ).addClass( base.config.focalClass );
				}
				var change  = base.focalChange( base.callback.prevSlideNum , 'prev' );
				base.animate( change );
				e.preventDefault();
			});

			if ( base.config.sldrAuto ) {
				base.config.previousSlide.bind( 'mouseenter' , function() {
					base.sliderPause();
				});
				base.config.previousSlide.bind( 'mouseleave' , function() {
					base.sliderTimer();
				});
			}
		}

		if ( base.config.swipeDragDesktop || base.isMobile() ) {

			base.$elwrp.bind( 'mousemove touchmove' , base.coordinateevents );

			base.$elwrp.bind( 'mousedown touchstart' , function( event ) {
				var xtf;

				if ( base.pagescrollY !== window.pageYOffset ) return;

				event.preventDefault();
				base.trackmouse = true;
				base.$elwrp.removeClass( 'animate' );
				base.transformposition = parseInt( base.getTranslatePosition( base.$elwrp.css( base.config.cssPrefix + 'transform' ) , 4 ) );
				if ( event.originalEvent.touches !== undefined ) {
					base.benchcoordinate = { x : event.originalEvent.touches[0].pageX , y : event.originalEvent.touches[0].pageY , trans : base.transformposition };
				} else {
					base.benchcoordinate = { x : event.clientX , y : event.clientY , trans : base.transformposition };
				}
				base.prevcoordinate = base.benchcoordinate;
				base.coordinateevents( event );
			});

			base.$elwrp.bind( 'mouseup touchend' , function( event ) {
				var change;

				if ( base.pagescrollY !== window.pageYOffset ) return;

				event.preventDefault();
				base.trackmouse = false;
				base.coordinatedifference = base.benchcoordinate.x - base.cursorcoordinate.x;
				if ( base.coordinatedifference > 0 && base.coordinatedifference > base.$el.width() / base.config.swipeDragCoefficient ) {
					change = base.focalChange( base.callback.nextSlideNum , 'next' );
				} else if ( base.coordinatedifference < 0 && -(base.coordinatedifference) > base.$el.width() / base.config.swipeDragCoefficient ) {
					change = base.focalChange( base.callback.prevSlideNum , 'prev' );
				} else if ( base.config.swipeDragSnapBack ) {
					base.$elwrp.addClass( 'animate' );
					base.$elwrp.css( base.config.cssPrefix + 'transform' , 'translate3d( ' + base.benchcoordinate.trans + 'px , 0 , 0 )' );
				}
				base.animate( change );
			});

		}

		if ( elmnts.length > 1 ) base.sliderTimer();

		/**
		 * Activate Resize
		 */
		$win.bind( 'resize' , function( e ) {
			base.$resizeTimers[base.config.sldrNumber] = setTimeout( function() {
				base.sliderPause();
				base.resizeElements();
				if ( elmnts.length > 1 ) base.sliderTimer();
			} , base.config.resizeDelay );
		});

		base.resizeElements();

		var change = {
			'currentFocalIndex' : base.$el.find( '.' + base.config.focalClass ).index(),
			'currentFocalPoint' : base.findFocalPoint(),
			'shiftWidth'        : 0
		};
		base.positionFocus( change );

		base.sliderLoaded( { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );

	};

	/**
	  * change the focus of the slider and animate it
	  * @return void
	  */
	base.animate = function( change ) {
		try {
			if ( base.config.animate != '' && base.config.animate ) {
				base.config.animate( base.$el , change , { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );
			} else {
				if ( !change ) return;
				var curr, tf, easing;
				base.$delayTimers[base.config.sliderNumber] = setTimeout( function() {
					base.$elwrp.addClass( 'animate' );
					if ( base.config.animateJquery || base.config.isBrowser == 'MSIE 6' || base.config.isBrowser == 'MSIE 7' || base.config.isBrowser == 'MSIE 8' || base.config.isBrowser == 'MSIE 9' || base.config.animate != false ) {
						easing = ( $.easing && $.easing.easeInOutQuint ) ? base.config.animateJqueryEasing : 'linear';
						base.$elwrp.animate({
							marginLeft : base.config.offset - change.currentFocalPoint
						} , base.config.animateJqueryTiming , easing );
					} else {
						curr = base.config.offset - change.currentFocalPoint;
						base.$elwrp.css( base.config.cssPrefix + 'transform' , 'translate3d( ' + curr + 'px , 0 , 0 )' );
					}
					base.slideComplete( { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );
				} , 100 ); // Tiny delay needed for slider to adjust
			}
		} catch ( err ) { console.log( err.message ); }
	};

	/**
	 * change the focus of the slider without animating it
	 * @param  {jquery selector object} the slide object
	 * @param  {number} change [description]
	 * @return void
	 */
	base.positionFocus = function( change ) {
		try {
			if ( !change ) return;
			var focus;
			base.$elwrp.removeClass( 'animate' );
			if ( base.config.animateJquery || base.config.isBrowser == 'MSIE 6' || base.config.isBrowser == 'MSIE 7' || base.config.isBrowser == 'MSIE 8' || base.config.isBrowser == 'MSIE 9' || base.config.animate != false ) {
				base.$elwrp.css( 'margin-left' , base.config.offset - change.currentFocalPoint );
			} else {
				if ( base.$elwrp.css( base.config.cssPrefix + 'transform' ) == 'none' ) base.$elwrp.css( base.config.cssPrefix + 'transform' , 'translate3d( 0 , 0 , 0 )' );
				focus = base.config.offset - change.currentFocalPoint;
				base.$elwrp.css( base.config.cssPrefix + 'transform' , 'translate3d(' + focus + 'px , 0  , 0 )' );
			}
			base.$delayTimers[base.config.sliderNumber] = setTimeout( function() {
				base.$elwrp.addClass( 'animate' );
				base.slideComplete( { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );
			} , base.config.resizeDelay + 1 ); // Tiny delay needed for slider to adjust

		} catch ( err ) { console.log( err.message ); }
	};


	/**********************************************************
	 ****************** Positioning Functions *****************
	 **********************************************************/

	/**
	 * Function to move focal point of the slider to previous or next slide.
	 * @param  {string} method ['prev' or 'previous' moves the slider backwards. Defaults to next.]
	 * @return {object}        The callback of the slider including slide number, slide index, previous, next slides etc.
	 */
	base.focalChange = function( focalChangeNum , method ) {
		try {
			method = typeof method !== 'undefined' ? method : 'default';
			var elmnts     = base.$elwrp.children();
			var focalElmnt = base.$elwrp.find( '> .' + base.config.focalClass );
			var focalIndex = focalElmnt.index();
			var nextFocalIndex, nextFocalPoint, prevFocalIndex, focalPoint, shiftSlide, shiftSlideClone, shiftSlideWidth, direction, slideClass;

			base.slideStart( { 'slides' : base.sldrSlides , 'callback' : base.callback , 'config' : base.config } );

			/**
			 * Find the nearest index of the focal point we want.
			 * @type {integer}
			 */
			if ( !base.sldrSlides[ focalChangeNum - 1 ] ) return;
			slideClass = base.sldrSlides[ focalChangeNum - 1 ].class_name;
			slideClass = slideClass.split(' ');
			slideClass = slideClass[ 0 ];

			if ( focalElmnt.hasClass( slideClass ) ) return false;

			closerBehind  = elmnts.eq( focalIndex ).prevAll( '.' + slideClass + ':first' ).index();
			closerInFront = elmnts.eq( focalIndex ).nextAll( '.' + slideClass + ':first' ).index();

			if (
				closerInFront != -1
				&& closerInFront - focalIndex < focalIndex - closerBehind
				|| closerInFront - focalIndex == focalIndex - closerBehind
				&& method != 'prev'
			) {
				nextFocalIndex = closerInFront;
			} else if ( closerBehind != -1 ) {
				nextFocalIndex = closerBehind;
			} else {
				nextFocalIndex = $( '.' + slideClass ).index();
			}

			nextFocalPoint = elmnts.eq( nextFocalIndex );
			elmnts.removeClass( base.config.focalClass );
			nextFocalPoint.addClass( base.config.focalClass );

			/**
			 * Find the range of elments in the slider to cut and paste, making it symmetrical.
			 * @type {Object}
			 */
			direction   = ( nextFocalIndex > parseInt( ( elmnts.length - 1 ) / 2 ) ) ? 'next' : 'prev';
			sliceStart  = ( direction == 'prev' ) ? parseInt( ( ( elmnts.length - nextFocalIndex + 1 ) / 2 ) + nextFocalIndex + 2 ) : 0;
			sliceEnd    = ( direction == 'prev' ) ? elmnts.length : parseInt( nextFocalIndex * 0.5 ) - 1;
			elmntsSlice = elmnts.slice( sliceStart , sliceEnd );
			elmntsClone = elmntsSlice.clone();

			/**
			 * Find the width difference to shift the slider before animating.
			 * @type {Number}
			 */
			shiftSlideWidth = 0;
			for ( var i = 0; i < elmntsSlice.length; i++ ) {
				shiftSlideWidth = shiftSlideWidth + $( elmntsSlice[ i ] ).width();
			}
			shiftSlideWidth = ( direction == 'prev' ) ? -( shiftSlideWidth ) : shiftSlideWidth;

			/**
			 * Shift the slider so the transition will appear seamless
			 */
			base.$elwrp.removeClass( 'animate' );
			if ( base.config.animateJquery || base.config.isBrowser == 'MSIE 6' || base.config.isBrowser == 'MSIE 7' || base.config.isBrowser == 'MSIE 8' || base.config.isBrowser == 'MSIE 9' || base.config.animate != false ) {
				// GET MARGIN PROPERTY
				curr = parseFloat( base.$elwrp.css( 'margin-left' ) );
				base.$elwrp.css( 'margin-left' , curr + shiftSlideWidth + 'px' );
			} else {
				// GET TRANSFORM PROPERTY
				if ( base.$elwrp.css( base.config.cssPrefix + 'transform' ) == 'none' ) base.$elwrp.css( base.config.cssPrefix + 'transform' , 'translate3d( 0 , 0 , 0 )' );
				matrixIndex = ( base.config.isIE ) ? 12 : 4;
				base.transformposition = parseInt( base.getTranslatePosition( base.$elwrp.css( base.config.cssPrefix + 'transform' ) , matrixIndex ) );
				curr = base.transformposition + shiftSlideWidth;
				base.$elwrp.css( base.config.cssPrefix + 'transform' , 'translate3d( ' + curr + 'px , 0 , 0 )' );
			}

			/**
			 * Remove/Append/Prepend Slides back to slider.
			 */
			elmnts.slice( sliceStart , sliceEnd ).remove();
			if ( direction == 'prev' || direction == 'previous' ) {
				base.$elwrp.prepend( elmntsClone );
			} else {
				base.$elwrp.append( elmntsClone );
			}

			/**
			 * Update $.sldr.callback.
			 * @type {Object}
			 */
			focalPoint        = base.findFocalPoint();
			currentFocalIndex = base.$elwrp.find( '> .' + base.config.focalClass ).index();
			nextFocalIndex    = ( currentFocalIndex + 1 == elmnts.length )           ? 0 : currentFocalIndex + 1;
			prevFocalIndex    = ( currentFocalIndex - 1 == -1 )                      ? elmnts.length - 1 : currentFocalIndex - 1;
			nextSlideNum      = ( focalChangeNum + 1 == base.sldrSlides.length + 1 ) ? 1 : focalChangeNum + 1;
			prevSlideNum      = ( focalChangeNum - 1 == 0 )                          ? base.sldrSlides.length : focalChangeNum - 1;

			base.callback.sldr              = base.$el;
			base.callback.prevFocalIndex    = prevFocalIndex;
			base.callback.prevSlideNum      = prevSlideNum;
			base.callback.currentFocalIndex = currentFocalIndex;
			base.callback.currentClass      = nextFocalPoint.attr( 'class' );
			base.callback.currentID         = nextFocalPoint.attr( 'id' );
			base.callback.currentFocalPoint = focalPoint;
			base.callback.currentSlideNum   = focalChangeNum;
			base.callback.shiftWidth        = shiftSlideWidth;
			base.callback.nextFocalIndex    = nextFocalIndex;
			base.callback.nextSlideNum      = nextSlideNum;

			if ( base.config.toggle.length > 0 ) {
				base.config.toggle.removeClass( base.config.focalClass );
				base.config.toggle.eq( focalChangeNum - 1 ).addClass( base.config.focalClass );
			}

			return base.callback;

		} catch ( err ) { console.log( err.message ); }
	};

	/**
	 * Recursive function to make slider fill the gaps of the stage
	 * @param  {object} elmntsHTML The markup to fill gaps with
	 * @param  {object} wrp        the container to place the markup in
	 * @return {boolean}           returns true when finished
	 */
	base.fillGaps = function( elmntsHTML ) {
		try {
			var sldrw     = base.$el.width();
			var wrp       = base.$elwrp;
			var elmnt     = wrp.children();
			var elmntw    = base.findWidth( elmnt );
			var lastClass = base.sldrSlides[base.sldrSlides.length - 1].class_name;
			if ( elmntw < sldrw * 5 ) {
				wrp.find( '.' + lastClass ).after( elmntsHTML );
				base.fillGaps( elmntsHTML );
			} else {
				wrp.css( 'width' , elmntw );
				return true;
			}
		} catch ( err ) { console.log( err.message ); }
	};

	/**
	 * Find the width of a set of elements
	 * @return {float} the width of the entire set
	 */
	base.findWidth = function( elmnt ) {
		try {
			var wdth = 0;
			elmnt.each( function( i ) {
				wdth = wdth + $( elmnt[i] ).width();
			});
			return wdth;
		} catch ( err ) { console.log( err.message ); }
	};

	/**
	 * Find the focal point of the slider, by class name 'focalPoint'
	 * @return {integer} the x position of the focal point
	 */
	base.findFocalPoint = function() {
		try {
			var elmnts          = base.$elwrp.children();
			var focalSlide      = base.$el.find( '.'+base.config.focalClass );
			var focalSlideWidth = focalSlide.width() / 2;
			var focalIndex      = focalSlide.index();
			var focalPoint      = focalSlideWidth;
			for ( var i = 0; i < focalIndex; i++ ) {
				focalPoint = focalPoint + elmnts.eq(i).width();
			}
			return focalPoint;
		} catch ( err ) { console.log( err.message ); }
	};

	/**
	 * Functions to perform when the browser is resized
	 * @return void
	 */
	base.resizeElements = function() {
		try {
			var elmnts   = base.$elwrp.children();
			var elmwdth  = base.findWidth( elmnts );
			var wrpwdth  = base.$elwrp.width();
			var samewdth = ( base.$el.css( 'width' ) == elmnts.css( 'width' ) ) ? true : false;

			if ( base.config.sldrWidth == 'responsive' ) {
				elmnts.css( 'width' , base.$el.width() );
				if ( elmwdth > wrpwdth ) base.$elwrp.css( 'width' , elmwdth );
			} else if ( base.config.sldrWidth != '' ) {
				return;
			}

			if ( base.config.sldrCols !== 1 ) {
				elmnts.css( 'width' , base.$el.width() / base.config.sldrCols );
			}

			if ( samewdth ) return;

			base.config.offset = base.$el.width() / 2; // UPDATE THE OFFSET, need to change this to work on the config offset

			var change = {
				'currentFocalIndex' : base.$el.find( '.' + base.config.focalClass ).index(),
				'currentFocalPoint' : base.findFocalPoint(),
				'shiftWidth'        : 0
			};
			base.positionFocus( change );
		} catch ( err ) { console.log( err.message ); }
	};

	/**
	 * Updates the previous coordinates if there is movement on click drag or touch drag
	 * @return {[type]} [description]
	 */
	base.coordinateevents = function( event ) {
		var curr;
		base.pagescrollY = window.pageYOffset;
		if ( !base.trackmouse ) return;
		event = event || window.event;
		if ( event.originalEvent.touches !== undefined ) {
			base.cursorcoordinate = { x : event.originalEvent.touches[0].pageX , y : event.originalEvent.touches[0].pageY };
		} else {
			base.cursorcoordinate = { x : event.clientX , y : event.clientY };
		}
		base.thiscoordinate = base.cursorcoordinate;
		if ( base.prevcoordinate != base.thiscoordinate ) {
			matrixIndex = ( base.config.isIE ) ? 12 : 4;
			base.transformposition = parseInt( base.getTranslatePosition( base.$elwrp.css( base.config.cssPrefix + 'transform' ) , matrixIndex ) );
			curr = base.transformposition - ( base.prevcoordinate.x - base.thiscoordinate.x );
			base.$elwrp.css( base.config.cssPrefix + 'transform' , 'translate3d( ' + curr + 'px , 0 , 0 )' );
			base.prevcoordinate = base.thiscoordinate;
		}
	};

	/**
	 * When the sldr is initiated, before the DOM is manipulated
	 * @param  {object} args [the slides, callback, and config of the slider]
	 * @return void
	 */
	base.sliderInit = function( args ) {
		if( base.config.sldrInit != '' ) {
			base.config.sldrInit( args );
		}
	};

	/**
	 * When individual slides are loaded
	 * @param  {object} args [the slides, callback, and config of the slider]
	 * @return void
	 */
	base.slideLoaded = function( args ) {
		if( base.config.sldLoaded != '' ) {
			base.config.sldLoaded( args );
		}
	};

	/**
	 * When the slider is loaded, after the DOM is manipulated
	 * @param  {object} args [the slides, callback, and config of the slider]
	 * @return void
	 */
	base.sliderLoaded = function( args ) {
		if( base.config.sldrLoaded != '' ) {
			base.config.sldrLoaded( args );
		}
	};

	/**
	 * Before the slides animate
	 * @param  {object} args [the slides, callback, and config of the slider]
	 * @return void
	 */
	base.slideStart = function( args ) {
		if( base.config.sldrStart != '' ) {
			base.config.sldrStart( args );
		}
	};

	/**
	 * When the slide has completed animating
	 * @param  {object} args [the slides, callback, and config of the slider]
	 * @return void
	 */
	base.slideComplete = function( args ) {
		if( base.config.sldrComplete != '' ) {
			base.config.sldrComplete( args );
		}
	};

	/**
	 * [sliderTimer description]
	 * @return void
	 */
	base.sliderTimer = function() {
		if ( base.config.sldrAuto ) {
			base.$sliderTimers[base.config.sldrNumber] = setTimeout( function() {
				var change  = base.focalChange( base.callback.nextSlideNum , 'next' );
				var animate = base.animate( change );
				base.sliderTimer( base.$el );
				if ( base.config.selectors != '' ) {
					var selector = base.config.selectors.eq( base.callback.nextSlideNum - 2 );
					selector.siblings().removeClass( base.config.focalClass );
					selector.addClass( base.config.focalClass );
				}

			} , base.config.sldrTime )
		}
	};

	/**
	 * Pauses slider by clearing it's timer
	 * @return void
	 */
	base.sliderPause = function() {
		if ( base.config.sldrAuto ) {
			clearTimeout( base.$sliderTimers[base.config.sldrNumber] );
		}
	};

	/**
	 * Check if number is even
	 * @param  {number} num Number to check
	 * @return boolean
	 */
	base.evenNumber = function( num ) {
		return ( num % 2 == 0 ) ? true : false;
	};

	/**
	 * Disect and return the value of a desired property in 'matrix(0, 0, 0, 0, 0, 0)' string format
	 * @param  {string} tf          matrix string
	 * @param  {number} matrixindex index of matrix desired
	 * @return {string}             matrix value string
	 */
	base.getTranslatePosition = function( tf , matrixindex ) {
		tf  = tf.slice( 7 , -1 ); // remove 'matrix(' and ')' from string
		tf  = tf.split( ', ' );   // create array of matrix
		return tf[ matrixindex ]; // return value of desired matrix
	};

	/**
	 * Get Browser and Set URL Prefix
	 * @return null
	 */
	base.browser = function() {
		if( navigator.userAgent.match('WebKit') != null ) {
			base.config.isBrowser = 'Webkit';
			base.config.cssPrefix = '-webkit-';
		} else if( navigator.userAgent.match('Gecko') != null ) {
			base.config.isBrowser = 'Gecko';
		} else if( navigator.userAgent.match('MSIE 6') != null ) {
			base.config.isBrowser = 'MSIE 6';
			base.config.isIE = true;
		} else if( navigator.userAgent.match('MSIE 7') != null ) {
			base.config.isBrowser = 'MSIE 7';
			base.config.isIE = true;
		} else if( navigator.userAgent.match('MSIE 8') != null ) {
			base.config.isBrowser = 'MSIE 8';
			base.config.isIE = true;
		} else if( navigator.userAgent.match('MSIE 9') != null ) {
			base.config.isBrowser = 'MSIE 9';
			base.config.isIE = true;
			base.config.cssPrefix = '-ms-';
		} else if( navigator.userAgent.match('MSIE 10') != null ) {
			base.config.isBrowser = 'MSIE 10';
			base.config.isIE = true;
		}
	};

	/**
	 * [isMobile description]
	 * @return {Boolean} [description]
	 */
	base.isMobile = function() {
      var check = false;
      (function (a) {
        if (/(android|ipad|playbook|silk|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows (ce|phone)|xda|xiino/i.test(a) || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0, 4))) check = true;
      })( navigator.userAgent || navigator.vendor || window.opera );
      return check;
    };

    base.init();
  };

  /**
   * [defaultOptions description]
   * @type {Object}
   */
  $.sldr.defaultOptions = {
    focalClass           : 'focalPoint',
	offset               : $( this ).width() / 2,
	selectors            : '',
	toggle               : '',
	nextSlide            : '',
	hashChange           : false,
	previousSlide        : '',
	resizeDelay          : 1,
	sldrNumber           : 0,
	sldrStart            : '',
	sldrComplete         : '',
	sldrInit             : '',
	sldLoaded            : '',
	sldrLoaded           : '',
	sldrWidth            : '',
	sldrCols             : 1,
	animate              : '',
	animateJquery        : false,
	animateJqueryTiming  : 750,
	animateJqueryEasing  : 'easeInOutQuint',
	swipeDragDesktop     : true,
	swipeDragCoefficient : 3,
	swipeDragSnapBack    : true,
	sldrAuto             : false,
	sldrTime             : 8000,
	isBrowser            : navigator.userAgent,
	isIE                 : false,
	cssPrefix            : ''
  };

  /**
   * [sldr description]
   * @param  {[type]} options [description]
   * @return {[type]}         [description]
   */
  $.fn.sldr = function( options ) {
    return this.each( function() {
      ( new $.sldr( this , options ) );
      $.sldr.defaultOptions.sldrNumber++;
    });
  };

})( jQuery );