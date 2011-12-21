enyo.kind({
	name: 'vkbKey',
	kind: enyo.Button,
	className: 'enyo-button key',
	allowHtml: true,
	
	published: {
		down: false,
		depressed: false,
		toggling: false,
		disabled: false
	},
	
	events: {
		ontouchstart: '',
		ontouchend: ''
	},

  	rendered: function() {
  		this.hasNode();
        this.node.ontouchstart = enyo.bind(this,'handleTouchstart')
        //this.node.ontouchmove = this.doTouchMove;
        this.node.ontouchend = enyo.bind(this,'handleTouchend')
  	},
  	
  	handleTouchstart: function() {
  		if (!this.disabled) {
  			if (this.toggling)
	  			this.setDown(!this.down)
  			else
  				this.setDown(true)
	  		return this.doTouchstart()
	  	}
  	},
  	
  	handleTouchend: function() {
  		if (!this.disabled && !this.toggling) {
	  		this.setDown(false)
	  		return this.doTouchend()
	  		
	  	}
  	},
	
	mouseoverHandler: function() {},                                                                    
    mouseoutHandler: function() {},                                                                
    mousedownHandler: function() {},
	mouseupHandler: function() {},
	flickHandler: function() {},
	clickHandler: function() {},
	dragstartHandler: function() {},
	dragoverHandler: function() {},
	dragfinishHandler: function() {}
	
})