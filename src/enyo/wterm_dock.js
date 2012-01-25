enyo.kind({

	name: "wTermDock",
	kind: enyo.VFlexBox,
	align: 'center',

	components: [
		{
			kind: 'Terminal',
    		name: 'terminal',
			width: window.innerWidth,
			height: window.innerHeight,
			exec: '/media/cryptofs/apps/usr/palm/applications/us.ryanhope.wterm/bin/cmatrix',
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
