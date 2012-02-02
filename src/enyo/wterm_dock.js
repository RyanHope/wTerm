enyo.kind({

	name: "wTermDock",
	kind: enyo.VFlexBox,
	align: 'center',

	initComponents: function() {
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
			executable: 'wterm',
			width: window.innerWidth,
			height: window.innerHeight,
			bgcolor: '000000',
			params: [enyo.application.p.get('fontSize').toString(10), enyo.application.p.get('exhibition')]
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
