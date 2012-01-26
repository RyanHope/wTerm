enyo.kind({

	name: "wTermDock",
	kind: enyo.VFlexBox,
	align: 'center',

	components: [
		{
			kind: 'ApplicationEvents',
			onApplicationRelaunch: 'onRelaunch',
			onWindowActivated: 'windowActivated',
			onWindowDeactivated: 'windowDeactivated'
		},
		{
			kind: 'Terminal',
    		name: 'terminal',
			width: window.innerWidth,
			height: window.innerHeight,
			exec: enyo.application.prefs.get('exhibition'),
		}
	],
	
	windowActivated: function() {
		this.$.terminal.setActive(1)
		this.$.terminal.inject('\x11')
	},
	windowDeactivated: function() {
		this.$.terminal.inject('\x13')
		this.$.terminal.setActive(0)
	}

})
