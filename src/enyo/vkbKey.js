enyo.kind({
	name: 'vkbKey',
	kind: enyo.Button,
	className: '',
	allowHtml: true,
	layoutKind: 'HFlexLayout',
	pack: 'center',
	align: 'center',
	
	published: {
		down: false,
		depressed: false,
		toggling: false,
		disabled: false,
		sym: -1,
		symbols: null
	},
	
	events: {
		ontouchstart: '',
		ontouchend: ''
	},

	create: function() {
		this.inherited(arguments);
		this.addClass('enyo-button');
		this.addClass('key');
	},
	
	initComponents: function() {
    	this.inherited(arguments)
    	if (this.symbols) {
	    	switch(this.symbols.length) {
	    		case 1:
	    			this.createComponents([
	    				{flex: 1, content: this.symbols[0][0]}
    				]);
	    			break;
	    		case 2:
	    			this.createComponents([
	    				{flex: 1, kind: 'VFlexBox', components: [
	    					{content: this.symbols[1][0]},
	    					{content: this.symbols[0][0]},
	    				]}
	    			]);
	    			break;
	    		case 3:
	    			this.createComponents([
	    				{flex: 1, kind: 'VFlexBox', components: [
	    					{content: this.symbols[1][0]},
	    					{content: this.symbols[0][0]},
	    				]},
	    				{flex: 1, kind: 'VFlexBox', pack: 'start', components: [
	    					{content: this.symbols[2][0], flex: 1}
	    				]},
    				]);
	    			break;
	    		case 4:
	    			this.createComponents([
	    				{flex: 1, kind: 'VFlexBox', components: [
	    					{content: this.symbols[1][0]},
	    					{content: this.symbols[0][0]},
	    				]},
	    				{flex: 1, kind: 'VFlexBox', components: [
	    					{content: this.symbols[3][0]},
	    					{content: this.symbols[2][0]}
	    				]}
    				]);
	    			break;
	    	}
	    }
   	},

  	rendered: function() {
		this.inherited(arguments);
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