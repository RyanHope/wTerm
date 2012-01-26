enyo.kind({

	name: "wTermDock",
	kind: enyo.VFlexBox,
	align: 'center',
		
	initComponents: function() {
		enyo.application.prefs = new Prefs();
		this.inherited(arguments)
		this.createComponent({
			kind: 'ApplicationEvents',
			onApplicationRelaunch: 'onRelaunch',
			onWindowActivated: 'windowActivated',
			onWindowDeactivated: 'windowDeactivated'
		})
		this.createComponent({
			kind: 'Terminal',
    		name: 'terminal',
			width: window.innerWidth,
			height: window.innerHeight,
			exec: enyo.application.prefs.get('exhibition'),
		})
	},
	
	windowActivated: function() {
		this.$.terminal.setActive(1)
		this.$.terminal.inject('\x11')
	},
	windowDeactivated: function() {
		this.$.terminal.inject('\x13')
		this.$.terminal.setActive(0)
	}

})
